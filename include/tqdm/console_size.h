#pragma once

#if defined(_WIN32) || defined(WIN32)
  #define TQDM_ON_WINDOWS
#elif defined(__unix__)
  #define TQDM_ON_NIX
#else
  #define TQDM_ON_SOMETHING_ELSE
#endif

#include <utility>

#ifdef TQDM_ON_NIX
#include <sys/ioctl.h>
#include <unistd.h>
#elif defined(TQDM_ON_WINDOWS)
#ifdef NOMINMAX
#include <windows.h>
#else
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#endif
#endif

namespace console_size{


inline std::pair<int,int> get_console_size(){
#ifdef TQDM_ON_WINDOWS
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#elif defined(TQDM_ON_NIX)
    struct winsize w{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    int rows = w.ws_row;
    int columns =  w.ws_col;
#else
    int rows = 25;
    int columns =  80;
#endif


    return {rows, columns};
  }

}
