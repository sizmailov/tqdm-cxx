#include "include/tqdm/tqdm.h"
#include <thread>



int main() {

  std::vector<int> a(100, 1);


  for (auto x: tqdm::tqdm(a)
                            .mininterval(std::chrono::milliseconds(100))
                            .leave(true)
                            .desc("hello")
                     ){
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    for (auto y: tqdm::tqdm(a)
        .mininterval(std::chrono::milliseconds(100))
        .leave(false)
        .desc("hello")
        ){
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }

}