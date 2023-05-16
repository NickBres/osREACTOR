#include "st_reactor.h"
#include <stdlib.h>

p_reactor_t createReactor(int size, int listenerFd)
{
    p_reactor_t reactor = (p_reactor_t)malloc(sizeof(reactor_t));
    reactor->handlers = (p_handler_t *)malloc(size * sizeof(p_handler_t));
    reactor->fds = (struct pollfd *)malloc(size * sizeof(struct pollfd));
    reactor->count = 0;
    reactor->size = size;
    reactor->isRunning = 0;
    reactor->listenerFd = listenerFd;
    pthread_mutex_init(&reactor->thread, NULL);
    return reactor;
}

void stopReactor(p_reactor_t reactor)
{
    reactor->isRunning = 0;
    pthread_mutex_destroy(&reactor->thread);
}

void startReactor(p_reactor_t reactor)
{
    pthread_mutex_lock(&reactor->thread);
    if (reactor->isRunning)
    {
        reactor->isRunning = 1;
        pthread_create(&reactor->thread, NULL, runReactor, reactor);
    }
    pthread_mutex_unlock(&reactor->thread);
}

void *runReactor(void *arg)
{
    p_reactor_t reactor = (p_reactor_t)arg;
    while (reactor->isRunning)
    {
        pthread_mutex_lock(&reactor->thread);
        waitFor(reactor);
        pthread_mutex_unlock(&reactor->thread);
    }
    return NULL;
}

void addFd(p_reactor_t reactor, int fd, handler_t handler)
{
    pthread_mutex_lock(&reactor->thread);
    if (reactor->count < reactor->size)
    {
        reactor->handlers[reactor->count] = (p_handler_t)malloc(sizeof(handler_t));
        reactor->handlers[reactor->count]->handler = handler.handler;
        reactor->handlers[reactor->count]->arg = handler.arg;
        reactor->fds[reactor->count].fd = fd;
        reactor->fds[reactor->count].events = POLLIN;
        reactor->count++;
    }
    pthread_mutex_unlock(&reactor->thread);
}

void waitFor(p_reactor_t reactor)
{
    int numEvents = poll(reactor->fds, reactor->count, -1);
    if (numEvents > 0)
    {
        for (int i = 0; i < reactor->count; i++)
        {
            if (reactor->fds[i].revents & POLLIN)
            {
                reactor->handlers[i]->handler(reactor->handlers[i]->arg);
            }
        }
    }
}

int findFd(p_reactor_t reactor, int fd)
{
    for (int i = 0; i < reactor->count; i++)
    {
        if (reactor->fds[i].fd == fd)
        {
            return i;
        }
    }
    return -1;
}

void deleteFd(p_reactor_t reactor, int fd)
{
    pthread_mutex_lock(&reactor->thread);
    int index = findFd(reactor, fd);
    if (index != -1)
    {
        free(reactor->handlers[index]);
        for (int i = index; i < reactor->count - 1; i++)
        {
            reactor->handlers[i] = reactor->handlers[i + 1];
            reactor->fds[i] = reactor->fds[i + 1];
        }
        reactor->count--;
    }
    pthread_mutex_unlock(&reactor->thread);
}

void deleteReactor(p_reactor_t reactor)
{
    if (reactor != NULL)
    {
        if (reactor->isRunning)
        {
            stopReactor(reactor);
        }
        if (reactor->handlers != NULL)
        {
            for (int i = 0; i < reactor->count; i++)
            {
                free(reactor->handlers[i]);
            }
            free(reactor->handlers);
        }
        if (reactor->fds != NULL)
        {
            free(reactor->fds);
        }
        pthread_mutex_destroy(&reactor->thread);
        free(reactor);
    }
}
