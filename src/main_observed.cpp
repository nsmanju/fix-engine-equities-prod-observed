#include <cstdio>
#include <thread>
#include <chrono>
#include "fix_session_observed.cpp"
#include "epoll_reactor_observed.cpp"
int main(){
  printf("\033[32m=== fix-engine-equities-prod-observed DEMO ===\033[0m\n");
  printf("Goal: SEE state machine live | v4 665us/1M reused | Latitude 7480\n\n");
  fix_prod_observed::SessionObserved sess[3];
  sess[0].set_ticker("AAPL.NASDAQ",0); sess[1].set_ticker("MSFT.NASDAQ",1); sess[2].set_ticker("0700.HK",2);
  for(int i=0;i<3;++i){ printf("\n--- Session %d %s startup ---\n",i,sess[i].ticker);
    sess[i].transition(fix_prod_observed::State::DISCONNECTED,"Init");
    sess[i].on_logon_send(); std::this_thread::sleep_for(std::chrono::milliseconds(50));
    sess[i].on_logon_recv(); sess[i].on_msg_seq(1,35); sess[i].on_msg_seq(2,35); sess[i].on_msg_seq(3,35); }
  printf("\n--- Simulate GAP on AAPL ---\n"); sess[0].on_msg_seq(5,68);
  printf("\n--- Normal flow MSFT ---\n"); sess[1].on_msg_seq(4,68); sess[1].on_msg_seq(5,68);
  printf("\n--- Logout 0700.HK EOD ---\n"); sess[2].on_logout("EOD 35=5 Text=Session closed");
  printf("\n\033[32m=== Demo complete ===\033[0m\nProd: epoll_wait -> recv -> FixBuilder -> on_msg_seq -> Store mmap\nObserved: +10-20ns printf only on state change\n");
  return 0;
}
