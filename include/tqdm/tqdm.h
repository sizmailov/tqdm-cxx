#pragma once

#include <iostream>
#include <optional>
#include <vector>
#include <string>
#include <sstream>
#include <regex>
#include <string_view>
#include <iomanip>
#include <chrono>
#include <cmath>

#include "./utils.h"
#include "./console_codes.h"
#include "./console_size.h"

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

  ProgressBar(){
  };

  ProgressBar(Iterator1 begin, Iterator2 end, std::optional<long long> size = {}) : m_begin(begin), m_end(end), m_size(
      size) {
  }

  ~ProgressBar() {
    if (!m_started || m_disabled) {
      return;
    }

    if (m_leave) {
      m_min_iterations_to_print = 0;
      print_progress_line();
    } else {
      (*m_file) << console_codes::move_cursor_up << console_codes::erase_line;
    }
  }

  void update(int n = 1) {
    if (m_disabled) {
      return;
    }
    m_n += n;
    print_progress_line();
  }

  void start() {
    if (m_started) {
      throw std::runtime_error("tqdm.begin:  double enter");
    }
    m_begin_time = std::chrono::high_resolution_clock::now();
    m_started = true;
    print_progress_line<true>();
  }


  auto begin()& {
    this->start();
    return IteratorWrapper(*this, m_begin);
  }

  auto end()& {
    return Sentinel(m_end);
  }

  ProgressBar desc(std::string message)&& {
    m_message = std::move(message);
    return std::move(*this);
  }

  ProgressBar& desc(std::string message)& {
    m_message = std::move(message);
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


  ProgressBar ncols(int n)&& {
    m_ncols = n;
    return std::move(*this);
  }

  ProgressBar& ncols(int n)& {
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

  ProgressBar file(std::ostream& out)&& {
    m_file = &out;
    return std::move(*this);
  }

  ProgressBar& file(std::ostream& out)& {
    m_file = &out;
    return *this;
  }


private:
  Iterator1 m_begin;
  Iterator2 m_end;
  std::optional<long long> m_size;
  std::chrono::high_resolution_clock::time_point m_begin_time;
  std::chrono::high_resolution_clock::time_point m_last_update_time;
  std::chrono::milliseconds m_mininterval = std::chrono::milliseconds{200};
  std::optional<long long> m_min_iterations_to_print = {};
  std::optional<double> estimated_speed = {};
  std::ostream* m_file = &std::cerr;
  std::string m_message;
  std::string unit = "it";

  std::optional<int> m_ncols;
  long long m_n = 0;
  long long m_n_last_update = 0;
  long long m_total_prints = 0;
  bool m_leave = true;
  bool m_disabled = false;
  bool m_started = false;

  int count_places(long long i){
    i = abs(i);
    int places = 0;
    while (i!=0){
      if (i<10) return 1+places;
      if (i<100) return 2+places;
      if (i<1000) return 3+places;
      if (i<10000) return 4+places;
      places+=4;
      i/=10000;
    }
    return places;
  }
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

      std::stringstream prefix;
      std::stringstream suffix;

      auto now = std::chrono::high_resolution_clock::now();
      auto dt_from_last_update = std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_last_update_time);
      if (m_min_iterations_to_print) {
        m_min_iterations_to_print =
            (m_n - m_n_last_update) *
            std::chrono::duration_cast<std::chrono::nanoseconds>(m_mininterval).count() /
            dt_from_last_update.count();

        if (m_min_iterations_to_print == 0) {
          m_min_iterations_to_print = 1;
        }
      }

      auto elapsed_time = now - m_begin_time;

      std::optional<int> percent;

      if (m_size) {
        int places = count_places(m_size.value());
        percent = m_n * 100 / m_size.value();

        prefix << std::setfill(' ') << std::setw(3) << std::fixed << std::setprecision(0) << percent.value() << "% ";

        prefix << std::setw(places) << m_n
                  << "/"
                  << display_bright << std::setw(places) << m_size.value() << reset_display;
      } else {
        prefix << m_n;
      }
      if (m_min_iterations_to_print) {
        if (m_n - m_n_last_update>0){
          double speed = (m_n - m_n_last_update)
              * 1e9 / std::chrono::duration_cast<std::chrono::nanoseconds>(dt_from_last_update).count();
          estimated_speed = speed;
        }
      }

      if (estimated_speed){
        print_speed(suffix, estimated_speed.value());
      }else{
        suffix << "    ?? "<<unit<<"/s";
      }


      suffix << fg_yellow << "[" << reset_display;
      if (m_size) {
        bool more_than_hour = elapsed_time > std::chrono::hours{1};
        std::optional<std::chrono::milliseconds> total_estimation;
        if (m_n > 0){
          total_estimation = std::chrono::duration_cast<std::chrono::milliseconds>(m_size.value() * elapsed_time / m_n);
          more_than_hour |= total_estimation > std::chrono::hours{1};
        }

        print_time(suffix, std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time), more_than_hour);
        suffix << "/";
        if (total_estimation){
          print_time(suffix,total_estimation.value(),more_than_hour);
        }else{
          suffix << "??" << (more_than_hour?("      "):("   "));
        }
      }else{
        print_time(suffix, std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time));
      }
      suffix << fg_yellow << "]" << reset_display;



      if (!is_first) {
        (*m_file) << move_cursor_up << erase_line;
      }

      (*m_file) << prefix.str();

      int cols = 0;
      if (m_ncols){
        cols = m_ncols.value();
      }else{
        cols = console_size::get_console_size().second;
      }

      int free_space = cols - std::regex_replace(prefix.str()+suffix.str(), std::regex("\033\\[\\d+\\w"),"").size();


      if (free_space>0){
        print_bar((*m_file), percent.value_or(0), free_space, m_message);
      }

      (*m_file) << suffix.str();

      (*m_file) << std::endl;


      ++m_total_prints;


      m_n_last_update = m_n;
      m_last_update_time = std::chrono::high_resolution_clock::now();
    }
  }

  void print_bar(std::ostream& out, int percent, int width, const std::string& text = "") {
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

  void print_speed(std::ostream& out, double speed){
    std::string suffix = unit+"/s";
    if (std::fabs(speed)<1.0){
      speed = 1/speed;
      suffix = "s/"+unit;
    }
    auto [num, places, scale] = format_sizeof(speed);

    auto old_fill = out.fill();
    out << std::fixed << std::setw(5) << std::setprecision(places) << std::setfill(' ') << num << scale << " " << suffix;
    out << std::setfill(old_fill);
  }

  void print_time(std::ostream& out, std::chrono::milliseconds dt, bool more_than_hour=false) {

    auto h = std::chrono::duration_cast<std::chrono::hours>(dt);
    auto m = std::chrono::duration_cast<std::chrono::minutes>(dt - h);
    auto s = std::chrono::duration_cast<std::chrono::seconds>(dt - h - m);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dt - h - m - s);

    using namespace console_codes;

    if (more_than_hour || h.count() > 0) {
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

  std::tuple<double, int, const char*> format_sizeof(double num){
    const double divisor=1000;

    for(auto unit: {"", "k", "M", "G", "T", "P", "E", "Z"}){
      if (std::fabs(num) < 9.995) return {num,2,unit};
      if (std::fabs(num) < 99.95) return {num,1,unit};
      if (std::fabs(num) < 999.5) return {num,0,unit};
      num /= divisor;
    }
    return {num,0, "Z"};
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

template<typename T>
class Range{
public:
  class Iterator;
  class Sentinel {
  public:
    explicit Sentinel(T pos) : m_pos(pos) {
    }

  private:
    friend class Iterator;
    long long m_pos;
  };

  class Iterator{
  public:
    explicit Iterator(T pos, T step) : m_pos(pos), m_step(step) {
    }

    decltype(auto) operator*() {
      return m_pos;
    }

    decltype(auto) operator++() {
      m_pos+=m_step;
      return *this;
    }

    bool operator!=(const Sentinel& sentinel) {
      return m_pos != sentinel.m_pos;
    }

  private:
    T m_pos;
    T m_step;
  };

  auto operator()(T start, T end, T step=1) const {
    T size;
    if (step > 0){
      size =(end - start+step-1)/step;
      end = size*step + start;
    }else{
      size = (end - start-step+1)/step;
      end = size*step + start;
    }
    return detail::ProgressBar(Range<T>::Iterator(start, step), Range<T>::Sentinel(end),size);
  }

  auto operator()(T end) const {
    return (*this)(0,end,1);
  }
};

constexpr auto tqdm = Tqdm();
constexpr auto range = Range<long long>();

}