#include "reactor_server.h"

p_reactor_t p_reactor;

void sigHandler(int sig)
{
    if (p_reactor)
    {
        stopReactor(p_reactor);
        deleteReactor(p_reactor);
    }

    exit(0);
}

// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Return a listening socket
int get_listener_socket(void)
{
    int listener; // Listening socket descriptor
    int yes = 1;  // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
    {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
        {
            continue;
        }

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this

    // If we got here, it means we didn't get bound
    if (p == NULL)
    {
        return -1;
    }

    // Listen
    if (listen(listener, MAX_CLIENTS) == -1)
    {
        return -1;
    }

    return listener;
}

void connectionHandler(p_reactor_t reactor, void *arg)
{
    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;
    char remoteIP[INET6_ADDRSTRLEN];
    int newfd; // Newly accepted socket descriptor

    addrlen = sizeof remoteaddr;
    newfd = accept(reactor->listenerFd, (struct sockaddr *)&remoteaddr, &addrlen); // Accept the incoming connection
    if (newfd == -1)
    {
        perror("accept");
    }
    else
    {
        printf("server: new connection from %s on "
               "socket %d\n",
               inet_ntop(remoteaddr.ss_family,
                         get_in_addr((struct sockaddr *)&remoteaddr),
                         remoteIP, INET6_ADDRSTRLEN),
               newfd);
        handler_t cHandler;
        cHandler.arg = NULL;
        cHandler.handler = &clientHandler;
        addFd(reactor, newfd, cHandler);
    }
}
void clientHandler(p_reactor_t reactor, int client_fd, void *arg)
{
    char buf[BUFF_SIZE] = {0};
    int nbytes;

    nbytes = recv(client_fd, buf, BUFF_SIZE, 0);
    if (nbytes <= 0)
    {
        if (nbytes == 0)
        {
            printf("server: socket %d hung up\n", client_fd);
        }
        else
        {
            perror("recv");
        }
        close(client_fd);
        deleteFd(reactor, client_fd);
    }
    else
    {
        // send to all clients
        printf("server: socket %d got message: %s", client_fd, buf);
        for (int i = 0; i < reactor->count; i++)
        {
            if (reactor->fds[i].fd != client_fd && reactor->fds[i].fd != reactor->listenerFd)
            {
                if (send(reactor->fds[i].fd, buf, nbytes, 0) == -1)
                {
                    perror("send");
                }
            }
        }
        printf("message sent to all clients\n");
    }
}

int main(void)
{
    // set signal handler
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    int listener; // Listening socket descriptor

    int newfd;                          // Newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;

    char buf[256]; // Buffer for client data

    char remoteIP[INET6_ADDRSTRLEN];

    // Set up and get a listening socket
    printf("listener: waiting for connections...\n");
    listener = get_listener_socket();

    if (listener == -1)
    {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    // Create reactor
    p_reactor = createReactor(5, listener); // 5 is the size of reactor will expand if needed

    // Add the listener to set
    handler_t cHandler;
    cHandler.arg = NULL;
    cHandler.handler = &connectionHandler;
    addFd(p_reactor, listener, cHandler);

    startReactor(p_reactor);
    printf("Reactor is running to stop the programm use CTRL+C\n");

    // now after reactor is runnig in other thread server can do other operations if needed
    // in our case nothing will happen so we will let the server some rest
    // only signal can stop the server/reactor
    while (p_reactor->isRunning)
    {
        sleep(1); // zzzzzzzz
    }

    return 0;
}