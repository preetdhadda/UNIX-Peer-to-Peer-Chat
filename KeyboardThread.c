#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>

#include "KeyboardThread.h"
#include "SendThread.h"
#include "ReceiveThread.h"
#include "PrintThread.h"
#include "SocketManager.h"
#include "list/list.h"

#define MSG_MAX_LEN 1024
pthread_t ThreadPID; 

static pthread_mutex_t MessageMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t ListMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t KeyboardSendMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t KeyboardSendCond = PTHREAD_COND_INITIALIZER;
static List* Sentlist;

void* KeyboardThread(void* args)
{
    fputs("Enter a message to send (enter ! to exit):\n", stdout); 
    
    while (1) 
    {
        // Receive inputted message and store it in malloced memory  
        char* NewMessage = (char*)malloc(MSG_MAX_LEN*sizeof(char));

        pthread_mutex_lock(&MessageMutex); 
        {
            if (fgets(NewMessage, MSG_MAX_LEN, stdin) == NULL)
            {
                perror("fgets is null\n");
            }
        }
        pthread_mutex_unlock(&MessageMutex); 

        // Add the message to the list that is shared between the Keyboard and Send threads 
        pthread_mutex_lock(&ListMutex);
        {
            int append = List_append(Sentlist, NewMessage);
            if (append == -1)
            {
                free(NewMessage);
                perror("Append failed in Keyboard thread\n");
                
            }
        }
        pthread_mutex_unlock(&ListMutex);

        // Signal the Send thread to begin executing 
        pthread_mutex_lock(&KeyboardSendMutex);
        {
            if (pthread_cond_signal(&KeyboardSendCond) != 0)
            {
                free(NewMessage);
                perror("Keyboard thread unable to signal Sender thread\n");
            }
        }
        pthread_mutex_unlock(&KeyboardSendMutex);

        // If first char in NewMessage is !, but the following chars are not '\n', then we don't want to exit 
        // So, set TimeToClose flag to false 
        bool TimeToClose = true; 
        if (*NewMessage == '!')
        {
            for (int i = 1; NewMessage[i] != '\0'; i++)
            {
                if (NewMessage[i] != '\n')
                {
                    TimeToClose = false; 
                    break;
                }
            }
        }
        else 
        {
            TimeToClose = false; 
        }

        // Shutdown the threads and socket if NewMessage is just one "!"
        if (TimeToClose == true)
        {
            fputs("------ Closing Socket ------\n", stdout);
            Keyboard_cancel();
            Sender_cancel(); 
            Receiver_cancel();
            Printer_cancel(); 
            SocketManager_Shutdown();
        }
    }
    
    return NULL; 
}

void Keyboard_init(List* inSentList) 
{
    Sentlist = inSentList;

    pthread_create
    (
        &ThreadPID, 
        NULL, 
        KeyboardThread, 
        NULL
    ); 
} 

// Accessed by Send thread, telling Send to wait until signalled by Keyboard
void Keyboard_tellSendToWait() 
{
    pthread_mutex_lock(&KeyboardSendMutex);
    {
        pthread_cond_wait(&KeyboardSendCond, &KeyboardSendMutex);
    }
    pthread_mutex_unlock(&KeyboardSendMutex);
}

static void pItemFreeFn(void *Item)
{
    if (Item != NULL)
    {
        free(Item);
    }
}

void Keyboard_shutdown()
{
    pthread_join(ThreadPID, NULL); 
    pthread_mutex_destroy(&MessageMutex);
    pthread_mutex_destroy(&ListMutex);
    pthread_mutex_destroy(&KeyboardSendMutex);
    pthread_cond_destroy(&KeyboardSendCond);
    List_free(Sentlist, pItemFreeFn);
} 

void Keyboard_cancel()
{
    pthread_cancel(ThreadPID); 
} 


