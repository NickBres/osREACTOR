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
    return reactor;
}

void stopReactor(p_reactor_t reactor)
{
    if (reactor != NULL)
    {
        reactor->isRunning = 0;
        pthread_join(reactor->thread, NULL);
    }
}

void startReactor(p_reactor_t reactor)
{
    if (!reactor->isRunning)
    {
        reactor->isRunning = 1;
        pthread_create(&reactor->thread, NULL, runReactor, reactor);
    }
}

void *runReactor(void *arg)
{
    p_reactor_t reactor = (p_reactor_t)arg;
    while (reactor->isRunning)
    {
        waitFor(reactor);
    }
}

void addFd(p_reactor_t reactor, int fd, handler_t handler)
{
    if (reactor->count < reactor->size)
    {
        reactor->handlers[reactor->count] = (p_handler_t)malloc(sizeof(handler_t));
        reactor->handlers[reactor->count]->handler = handler.handler;
        reactor->handlers[reactor->count]->arg = handler.arg;
        reactor->fds[reactor->count].fd = fd;
        reactor->fds[reactor->count].events = POLLIN;
        reactor->count++;
    }
    else
    {
        // expand arrays
        reactor->handlers = (p_handler_t *)realloc(reactor->handlers, reactor->size * 2 * sizeof(p_handler_t));
        reactor->fds = (struct pollfd *)realloc(reactor->fds, reactor->size * 2 * sizeof(struct pollfd));
        reactor->size *= 2;
        addFd(reactor, fd, handler);
    }
}

void waitFor(p_reactor_t reactor)
{
    int numEvents = poll(reactor->fds, reactor->count, -1);
    if (numEvents > 0)
    {
        int currCount = reactor->count;
        for (int i = 0; i < currCount; i++)
        {
            if (reactor->fds[i].revents & POLLIN)
            {
                reactor->handlers[i]->handler(reactor, reactor->fds[i].fd, reactor->handlers[i]->arg);
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
        free(reactor);
    }
}
