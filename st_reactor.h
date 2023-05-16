#ifndef ST_REACTOR_H
#define ST_REACTOR_H

#include <sys/poll.h>
#include <pthread.h>

typedef struct st_reactor* p_reactor_t;
typedef struct {
    void (*handler)(p_reactor_t, int, void*);
    void* arg;
} handler_t, *p_handler_t;

typedef struct st_reactor
{
    p_handler_t* handlers; // handler array 
    struct pollfd* fds; // fd array
    int count; 
    int size; // array size
    int isRunning; // reactor status
    int listenerFd; // listener fd
    pthread_mutex_t thread; // reactor thread. mutex to make it thread safe
}reactor_t, *p_reactor_t;

p_reactor_t createReactor(int size,int listenerFd); // allocate memory for reactor
void stopReactor(p_reactor_t reactor); // join threads and stop reactor
void startReactor(p_reactor_t reactor); // create new reactor thread
void* runReactor(void* arg); // run reactor thread
void addFd(p_reactor_t reactor, int fd, handler_t handler); // add new fd to reactor
void waitFor(p_reactor_t reactor); // wait for events
void deleteReactor(p_reactor_t reactor); // free memory and stop reactor
void deleteFd(p_reactor_t reactor, int fd); // delete fd from reactor
int findFd(p_reactor_t reactor, int fd); // return fd index in reactor

#endif // !ST_REACTOR_H
