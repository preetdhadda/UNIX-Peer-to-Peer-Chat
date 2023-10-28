#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "ReceiveThread.h"
#include "SocketManager.h"
#include "list/list.h"

#define MSG_MAX_LEN 1024

static pthread_t ThreadPID; 
static pthread_mutex_t ListMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t MessageMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t ReceivePrintMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t ReceivePrintCond = PTHREAD_COND_INITIALIZER;
List* ReceivedList;

void* ReceiveThread(void* args)
{
    while (1) 
    {
        // Access the socket and the address information of the machine we're receiving 
        // the message from 
        int Socket = GetSocket(); 
        if (Socket < 0) 
        {
            perror("Socket error in Receive Thread\n"); 
            continue;
        }

        struct addrinfo *RemoteAddress = GetRemoteAddressInfo();
        struct sockaddr_in* SinRemote = (struct sockaddr_in*)RemoteAddress->ai_addr;
        unsigned int SinLen = sizeof(SinRemote); 
        char MessageReceived[MSG_MAX_LEN]; 
        
        // Receive the message through the socket 
        int BytesReceived = recvfrom(Socket, 
            MessageReceived,
            MSG_MAX_LEN, 
            0, 
            (struct sockaddr*)&SinRemote, &SinLen 
        );

        if (BytesReceived < 0)
        {
            perror("recvfrom error\n"); 
            continue;
        }

        int TerminateIdx = (BytesReceived < MSG_MAX_LEN) ? BytesReceived : MSG_MAX_LEN-1; 
        MessageReceived[TerminateIdx] = '\0'; 

        // Add the message to the list that is shared between Receive and Print threads
        pthread_mutex_lock(&ListMutex);
        {
            if (ReceivedList!= NULL) 
            {
                int append = List_append(ReceivedList, MessageReceived);
                if (append == -1)
                {
                    perror("Append failed in Receive thread\n");
                }
            }
            else 
            {
                perror("Received list is NULL\n");
            }
        }
        pthread_mutex_unlock(&ListMutex);

        // Signal the Print thread to begin executing 
        pthread_mutex_lock(&ReceivePrintMutex);
        {
            pthread_cond_signal(&ReceivePrintCond);
        }
        pthread_mutex_unlock(&ReceivePrintMutex);
    }

    return NULL; 
}

void Receiver_init(List* inReceivedList)
{
    ReceivedList = inReceivedList;

    pthread_create
    (
        &ThreadPID, 
        NULL, 
        ReceiveThread, 
        NULL
    ); 
}

// Accessed by Print thread, telling Print to wait until signalled by Receive 
void Receiver_tellPrintToWait() 
{
    pthread_mutex_lock(&ReceivePrintMutex);
    {
        pthread_cond_wait(&ReceivePrintCond, &ReceivePrintMutex);
    }
    pthread_mutex_unlock(&ReceivePrintMutex);
}

static void pItemFreeFn(void *Item)
{
    if (Item != NULL)
    {
        free(Item);
    }
}

void Receiver_shutdown()
{
    pthread_join(ThreadPID, NULL);
    pthread_mutex_destroy(&ReceivePrintMutex);
    pthread_cond_destroy(&ReceivePrintCond);
    pthread_mutex_destroy(&ListMutex);
    pthread_mutex_destroy(&MessageMutex);
    List_free(ReceivedList, pItemFreeFn);
    } 

void Receiver_cancel()
{
    pthread_cancel(ThreadPID); 
} 

