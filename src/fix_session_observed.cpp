#include <cstdio>
#include <chrono>
namespace fix_prod_observed {
enum class State { DISCONNECTED, LOGON_SENT, LOGON_RECV, ACTIVE, RESEND, LOGOUT };
inline const char* to_str(State s){
  switch(s){
    case State::DISCONNECTED: return "DISCONNECTED";
    case State::LOGON_SENT: return "LOGON_SENT (35=A sent)";
    case State::LOGON_RECV: return "LOGON_RECV (35=A recv)";
    case State::ACTIVE: return "ACTIVE (trading)";
    case State::RESEND: return "RESEND (35=2 Gap)";
    case State::LOGOUT: return "LOGOUT (35=5)";
  }
  return "UNK";
}
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define MAGENTA "\033[35m"
#define RESET "\033[0m"
struct SessionObserved {
  State state = State::DISCONNECTED;
  uint64_t exp_seq=1, out_seq=1;
  int session_id=0;
  const char* ticker="";
  void set_ticker(const char* t,int id){ ticker=t; session_id=id; }
  void transition(State next,const char* reason){
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    long long us = std::chrono::duration_cast<std::chrono::microseconds>(now).count()%1000000;
    printf(CYAN "[%06lld us]" RESET " Sess %d %s " YELLOW "%s -> %s" RESET " %s exp=%lu out=%lu\n",
      us,session_id,ticker,to_str(state),to_str(next),reason,exp_seq,out_seq);
    state=next;
  }
  void on_logon_send(){ transition(State::LOGON_SENT,"We send 35=A 98=0 108=30"); }
  void on_logon_recv(){ transition(State::LOGON_RECV,"Exchange 35=A ok"); transition(State::ACTIVE,"Both logons -> ACTIVE"); }
  bool on_msg_seq(uint64_t recv_seq,int tag35){
    if(recv_seq==exp_seq){ printf(GREEN " -> OK Seq %lu exp %lu tag35=%d %s\n" RESET,recv_seq,exp_seq,tag35,ticker); exp_seq++; return true; }
    if(recv_seq>exp_seq){ printf(RED " *** GAP! *** recv=%lu > exp=%lu gap=%lu %s\n" RESET,recv_seq,exp_seq,recv_seq-exp_seq,ticker);
      printf(YELLOW " -> Send 35=2 Resend 7=%lu 16=%lu\n" RESET,exp_seq,recv_seq-1);
      transition(State::RESEND,"GapFill needed 35=2"); printf(MAGENTA " -> Expect 35=4 GapFill / replay\n" RESET); exp_seq=recv_seq+1;
      transition(State::ACTIVE,"Gap filled -> ACTIVE"); return false; }
    printf(" -> Dup seq=%lu < exp=%lu\n",recv_seq,exp_seq); return true;
  }
  void on_logout(const char* txt){ transition(State::LOGOUT,txt); transition(State::DISCONNECTED,"Socket closed, ready reconnect"); }
};
}
