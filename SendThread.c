#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "KeyboardThread.h"
#include "SendThread.h"
#include "SocketManager.h"
#include "list/list.h"

static struct addrinfo* SinRemote; 

#define MSG_MAX_LEN 1024
static pthread_t ThreadPID; 
static pthread_mutex_t MessageMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t inSentListMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t inSentListCond = PTHREAD_COND_INITIALIZER;
List* SentList;

void* SenderThread(void* args)
{
    while (1)
    {
        // Wait until signalled by Send thread 
        Keyboard_tellSendToWait();

        // Access the socket and get address information about machine we're sending the message to 
        int Socket = GetSocket(); 
        SinRemote = GetRemoteAddressInfo();

        if (Socket < 0)
        {
            perror("Error creating socket\n");
            continue; 
        }

        // Trim the list that was updated in Keyboard thread and send it through the socket 
        // to the other machine 
        pthread_mutex_lock(&MessageMutex); 
        {
            char* Message;
            if (List_count(SentList) > 0) 
            {
                Message = List_trim(SentList); 
                
                sendto(Socket, Message, strlen(Message), 0, SinRemote->ai_addr, SinRemote->ai_addrlen); 
            }
            else 
            {
                free(Message);
                perror("List is empty, unable to send message\n");
            }
        }
        pthread_mutex_unlock(&MessageMutex);
    }
}

void Sender_init(List* inSentList)
{
    SentList = inSentList;

    pthread_create
    (
        &ThreadPID, 
        NULL, 
        SenderThread, 
        NULL
    ); 
}

static void pItemFreeFn(void *Item)
{
    if (Item != NULL)
    {
        free(Item);
    }
}

void Sender_shutdown()
{
    pthread_join(ThreadPID, NULL); 
    pthread_mutex_destroy(&MessageMutex);
    List_free(SentList, pItemFreeFn);
}

void Sender_cancel()
{
    pthread_cancel(ThreadPID); 
}

