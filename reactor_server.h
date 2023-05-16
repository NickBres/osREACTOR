#ifndef REACTOR_SERVER_H
#define REACTOR_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

#include "st_reactor.h"

#define PORT "9034" // Port we're listening on
#define MAX_CLIENTS 100 // Max number of people on chat
#define BUFF_SIZE 256 // Max buffer size

// Function Declarations
void sigHandler(int sig); // will stop reactor and close the program
void *get_in_addr(struct sockaddr *sa); // get sockaddr, IPv4 or IPv6 from beej's guide
int get_listener_socket(void);  // get listener socket from beej's guide
void connectionHandler(p_reactor_t reactor,void *arg); // handle new connections 
void clientHandler(p_reactor_t reactor,int client_fd,void *arg); // recieve and send messages to clients

#endif //REACTOR_SERVER_H
