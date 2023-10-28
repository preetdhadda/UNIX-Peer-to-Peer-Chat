#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "SocketManager.h"

#define MAX_MSG_LEN 1024 

static int Socket = 0; 
static struct addrinfo *RemoteAddressInfo;

void SocketManager_Init(int LocalPort, char* RemoteMachine, char* RemotePort)
{
    // Socket configurations 
    struct addrinfo hints; 
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_DGRAM; 
    hints.ai_flags = AI_PASSIVE;

    // Store the address information of the other machine in the chat 
    if (getaddrinfo(RemoteMachine, RemotePort, &hints, &RemoteAddressInfo) != 0) 
    {
        perror("getaddrinfo() error\n");
    }

    Socket = socket(PF_INET, SOCK_DGRAM, 0);

    // Configure local address to bind to socket 
    struct sockaddr_in LocalAddress;
    memset(&LocalAddress, 0, sizeof(LocalAddress));
    LocalAddress.sin_family = AF_INET;
    LocalAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    LocalAddress.sin_port = htons(LocalPort);

    if (bind(Socket, (struct sockaddr*)&LocalAddress, sizeof(LocalAddress)) < 0) 
    {
        printf("Error: Socket could not be found\n");
    }
}

int GetSocket() 
{
    return Socket; 
}

struct addrinfo* GetRemoteAddressInfo()
{
    return RemoteAddressInfo; 
}

void SocketManager_Shutdown()
{
    close(Socket);
    Socket = -1;
}