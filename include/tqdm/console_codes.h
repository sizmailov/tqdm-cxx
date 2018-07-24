#pragma once
#include <iostream>

namespace tqdm{
namespace console_codes {

template<typename CharT=char>
struct CodeWithIntParameter {
  explicit constexpr CodeWithIntParameter(const CharT* suffix, int n = 1) noexcept : suffix(suffix), n(n) {
  }

  constexpr auto operator()(int n) {
    return CodeWithIntParameter(suffix, n);
  };
  const CharT* suffix;
  const int n;

  friend std::ostream& operator<<(std::ostream& out, const CodeWithIntParameter& code) {
    out << "\033[" << code.n << code.suffix;
    return out;
  }
};

constexpr auto move_cursor_up = CodeWithIntParameter("A");
constexpr auto move_cursor_down = CodeWithIntParameter("B");
constexpr auto move_cursor_right = CodeWithIntParameter("C");
constexpr auto move_cursor_left = CodeWithIntParameter("D");
constexpr auto move_cursor_to_col1_up = CodeWithIntParameter("E");
constexpr auto move_cursor_to_col1_down = CodeWithIntParameter("F");
constexpr auto move_cursor_to_col = CodeWithIntParameter("G");
//  constexpr auto move_cursor_to_row_col = CodeWithIntIntParameter("H",1,1);
constexpr auto erase_display_down = "\033[J";
constexpr auto erase_display_up = "\033[1J";
constexpr auto erase_display = "\033[2J";
constexpr auto erase_end_of_line = "\033[K";
constexpr auto erase_start_of_line = "\033[1K";
constexpr auto erase_line = "\033[2K";
constexpr auto insert_lines = CodeWithIntParameter("L");
constexpr auto delete_lines = CodeWithIntParameter("M");

constexpr auto reset_display = "\033[0m";
constexpr auto display_bright = "\033[1m";
constexpr auto display_dim = "\033[2m";
constexpr auto display_underscore = "\033[4m";
constexpr auto display_blink = "\033[5m";
constexpr auto display_reverse = "\033[7m";
constexpr auto display_hidden = "\033[8m";


constexpr auto fg_black = "\033[30m";
constexpr auto fg_red = "\033[31m";
constexpr auto fg_green = "\033[32m";
constexpr auto fg_yellow = "\033[33m";
constexpr auto fg_blue = "\033[34m";
constexpr auto fg_magenta = "\033[35m";
constexpr auto fg_cyan = "\033[36m";
constexpr auto fg_white = "\033[37m";
constexpr auto fg_default_underscore = "\033[38m";
constexpr auto fg_default_no_underscore = "\033[39m";

constexpr auto bg_black = "\033[40m";
constexpr auto bg_red = "\033[41m";
constexpr auto bg_green = "\033[42m";
constexpr auto bg_yellow = "\033[43m";
constexpr auto bg_blue = "\033[44m";
constexpr auto bg_magenta = "\033[45m";
constexpr auto bg_cyan = "\033[46m";
constexpr auto bg_white = "\033[47m";
constexpr auto bg_default = "\033[49m";

}
}