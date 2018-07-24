#include "include/tqdm/tqdm.h"
#include <thread>



int main() {

  auto run = [](int N, int us_per_loop){
  std::vector<int> a(N, 1);
    for (auto x: tqdm::tqdm(a)
                              .mininterval(std::chrono::milliseconds(100))
                              .leave(true)
                              .desc("hello")
                       ){
      std::this_thread::sleep_for(std::chrono::microseconds(us_per_loop));
    }
  };

  run(100000,1);
  run(10000,10);
  run(1000,100);
  run(100,1000);
  run(10,10000);
  run(5,100000);
  run(5,2000000);


}