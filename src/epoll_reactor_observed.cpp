#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
namespace fix_prod_observed {
#define CYAN "\033[36m"
#define RESET "\033[0m"
struct EpollReactorObserved {
  int epfd=-1;
  EpollReactorObserved(){ epfd=epoll_create1(EPOLL_CLOEXEC); printf(CYAN "[epoll] create epfd=%d ET|EPOLLONESHOT\n" RESET, epfd); }
  bool add_fd(int fd){
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL,0)|O_NONBLOCK);
    int one=1; setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    epoll_event ev{}; ev.events=EPOLLIN|EPOLLET|EPOLLONESHOT; ev.data.fd=fd;
    printf("[epoll] add_fd fd=%d EPOLLIN|ET|ONESHOT TCP_NODELAY\n", fd);
    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev)==0;
  }
  int wait_loop(char* buf,int cap){
    epoll_event evs[64]; int n=epoll_wait(epfd,evs,64,10);
    if(n>0) printf("[epoll] wait %d events\n", n);
    for(int i=0;i<n;++i){ int fd=evs[i].data.fd; ssize_t got=::recv(fd,buf,cap,0);
      printf("[epoll] recv fd=%d got=%zd zero-copy -> FixBuilder\n",fd,got);
      epoll_event ev{}; ev.events=EPOLLIN|EPOLLET|EPOLLONESHOT; ev.data.fd=fd;
      epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev); printf("[epoll] re-arm ONESHOT fd=%d\n",fd); }
    return n;
  }
};
}
