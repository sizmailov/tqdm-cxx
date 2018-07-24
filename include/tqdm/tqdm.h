#pragma once

#include <iostream>
#include <optional>
#include <vector>
#include <string>
#include <string_view>
#include <iomanip>
#include <chrono>

#include "./utils.h"
#include "./console_codes.h"

namespace tqdm {

namespace detail {
template<typename Iterator1, typename Iterator2>
class ProgressBar {
public:

  class IteratorWrapper;

  class Sentinel {
  public:
    explicit Sentinel(Iterator2& end) : m_end(end) {
    }

  private:
    friend class IteratorWrapper;

    Iterator2& m_end;
  };

  class IteratorWrapper {
  public:
    explicit IteratorWrapper(ProgressBar& pbar, Iterator1& it) : m_pbar(pbar), m_it(it) {
    }

    decltype(auto) operator*() {
      return *m_it;
    }

    decltype(auto) operator->() {
      return *m_it;
    }

    decltype(auto) operator++() {
      ++m_it;
      m_pbar.update();
      return m_it;
    }

    bool operator!=(const Sentinel& sentinel) {
      return m_it != sentinel.m_end;
    }

  private:
    Iterator1& m_it;
    ProgressBar& m_pbar;
  };

  ProgressBar(Iterator1 begin, Iterator2 end, std::optional<long long> size = 0) : m_begin(begin), m_end(end), m_size(
      size) {
  }

  ~ProgressBar() {
    if (!m_started || m_disabled) {
      return;
    }

    if (m_leave) {
      m_min_iterations_to_print = 0;
      print_progress_line();
      std::cout << "Total prints: " << m_total_prints << std::endl;
    } else {
      std::cout << console_codes::move_cursor_up << console_codes::erase_line;
    }
  }

  void update(int n = 1) {
    if (m_disabled) {
      return;
    }
    m_n += n;
    print_progress_line();
  }

  auto begin()& {
    if (m_started) {
      throw std::runtime_error("tqdm.begin:  double enter");
    }
    m_begin_time = std::chrono::high_resolution_clock::now();
    m_started = true;
    print_progress_line<true>();
    return IteratorWrapper(*this, m_begin);
  }

  auto end()& {
    return Sentinel(m_end);
  }

  ProgressBar desc(const std::string& message)&& {
    m_message = message;
    return std::move(*this);
  }

  ProgressBar& desc(const std::string& message)& {
    m_message = message;
    return *this;
  }

  ProgressBar total(long long n)&& {
    m_size = n;
    return std::move(*this);
  }

  ProgressBar& total(long long n)& {
    m_size = n;
    return *this;
  }


  ProgressBar ncols(long long n)&& {
    m_ncols = n;
    return std::move(*this);
  }

  ProgressBar& ncols(long long n)& {
    m_ncols = n;
    return *this;
  }


  ProgressBar leave(bool flag = true)&& {
    m_leave = flag;
    return std::move(*this);
  }


  ProgressBar& leave(bool flag = true)& {
    m_leave = flag;
    return *this;
  }

  ProgressBar disable(bool flag = false)&& {
    m_disabled = flag;
    return std::move(*this);
  }

  ProgressBar& disable(bool flag = false)& {
    m_disabled = flag;
    return *this;
  }

  ProgressBar mininterval(std::chrono::milliseconds interval)&& {
    m_mininterval = interval;
    return std::move(*this);
  }

  ProgressBar& mininterval(std::chrono::milliseconds interval)& {
    m_mininterval = interval;
    return *this;
  }


private:
  Iterator1 m_begin;
  Iterator2 m_end;
  std::optional<long long> m_size;
  std::chrono::high_resolution_clock::time_point m_begin_time;
  std::chrono::high_resolution_clock::time_point m_last_update_time;
  std::chrono::milliseconds m_mininterval = std::chrono::milliseconds{100};
  std::optional<long long> m_min_iterations_to_print = {};
  std::string m_message;
  long long m_ncols = 80;
  long long m_n = 0;
  long long m_n_last_update = 0;
  long long m_total_prints = 0;
  bool m_leave = true;
  bool m_disabled = false;
  bool m_started = false;


  template<bool is_first = false>
  void print_progress_line() {

    using namespace console_codes;

    if (!is_first) {
      if (!m_min_iterations_to_print) {
        m_min_iterations_to_print =
            std::chrono::duration_cast<std::chrono::nanoseconds>(m_mininterval).count() /
            std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()
                                                                 - m_last_update_time).count();
        if (m_min_iterations_to_print == 0) {
          m_min_iterations_to_print = 1;
        }
      }
    } else {
      m_n_last_update = m_n;
    }

    if (m_n - m_n_last_update >= m_min_iterations_to_print.value_or(0)) {
      auto now = std::chrono::high_resolution_clock::now();
      if (m_min_iterations_to_print) {
        m_min_iterations_to_print =
            (m_n - m_n_last_update) *
            std::chrono::duration_cast<std::chrono::nanoseconds>(m_mininterval).count() /
            std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_last_update_time).count();

        if (m_min_iterations_to_print == 0) {
          m_min_iterations_to_print = 1;
        }
      }

      auto elapsed_time = now - m_begin_time;

      if (!is_first) {
        std::cout << move_cursor_up << erase_line;
      }
      if (m_size) {
        std::cout << m_n
                  << "/"
                  << display_bright << m_size.value() << reset_display;
        print_progress_line(std::cout, m_n * 100 / m_size.value(), 30, m_message);
      } else {
        std::cout << m_n;
      }
      if (m_min_iterations_to_print) {
        std::cout
            << ", "
            << m_min_iterations_to_print.value()
               * 1000
               / std::chrono::duration_cast<std::chrono::milliseconds>(m_mininterval).count()
            << " it/s";
      }
      std::cout << "[";
      print_time(std::cout, std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time));
      if (m_size && m_n > 0) {
        std::cout << " / ";
        print_time(std::cout,
                   std::chrono::duration_cast<std::chrono::milliseconds>(m_size.value() * elapsed_time / m_n));
      }
      std::cout << "]";

      std::cout << std::endl;


      ++m_total_prints;


      m_n_last_update = m_n;
      m_last_update_time = std::chrono::high_resolution_clock::now();
    }
  }

  void print_progress_line(std::ostream& out, int percent, int width, const std::string& text = "") {
    bool violated = false;
    if (percent < 0 || percent > 100) {
      violated = true;
    }
    percent = std::max(std::min(100, percent), 0);
    using namespace console_codes;
    width -= 2;
    int p1 = percent * width / 100;
    int p2 = width - percent * width / 100;

    auto s1 = text.substr(0, p1);
    auto s2 = p1 > text.size() ? std::string() : text.substr(p1);

    s1.resize(p1, ' ');
    s2.resize(p2, ' ');


    if (!violated) {

      out
          << '|'
          << bg_black << fg_white << s1
          << bg_white << fg_black << s2
          << reset_display
          << '|';
    } else {
      out
          << '|'
          << bg_red << fg_white << s1
          << bg_white << fg_red << s2
          << reset_display
          << '|';
    }
  }

  void print_time(std::ostream& out, std::chrono::milliseconds dt) {

    auto h = std::chrono::duration_cast<std::chrono::hours>(dt);
    auto m = std::chrono::duration_cast<std::chrono::minutes>(dt - h);
    auto s = std::chrono::duration_cast<std::chrono::seconds>(dt - h - m);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dt - h - m - s);

    using namespace console_codes;


    if (h.count() > 0) {
      out
          << fg_magenta
          << display_bright
          << std::setw(2) << std::setfill('0') << h.count()
          << fg_default_no_underscore
          << ":";
    }


    out
        << fg_cyan
        << display_bright
        << std::setw(2) << std::setfill('0') << m.count()
        << fg_default_no_underscore << ":"
        << fg_blue
        << display_bright
        << std::setw(2) << std::setfill('0') << s.count()
        << reset_display;
  }
};

template<typename T>
using sized_type_t = decltype(std::declval<T&>().size());

}


class Tqdm {
public:

  template<typename T, int N>
  auto operator()(T (& c)[N]) const {
    return detail::ProgressBar(std::begin(c), std::end(c), N);
  }

  template<typename Container, typename std::enable_if_t<utils::is_detected_v<detail::sized_type_t, Container>>* = nullptr>
  auto operator()(Container& c) const {
    return detail::ProgressBar(std::begin(c), std::end(c), c.size());
  }


  template<typename Container, typename std::enable_if_t<!utils::is_detected_v<detail::sized_type_t, Container>>* = nullptr>
  auto operator()(Container& c) const {
    return detail::ProgressBar(std::begin(c), std::end(c));
  }

};

constexpr auto tqdm = Tqdm();

}