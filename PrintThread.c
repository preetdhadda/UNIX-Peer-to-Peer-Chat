#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "KeyboardThread.h"
#include "SendThread.h"
#include "ReceiveThread.h"
#include "PrintThread.h"
#include "SocketManager.h"
#include "list/list.h"

#define MSG_MAX_LEN 1024
static pthread_t ThreadPID; 

static pthread_mutex_t ListMutex = PTHREAD_MUTEX_INITIALIZER;
List* ReceivedList;

void* PrinterThread(void* args)
{
    while (1) 
    {
        // Wait until signalled by Receive thread 
        Receiver_tellPrintToWait();

        // Trim the message from the list updated in Receive thread and print to screen 
        char* Message; 
        pthread_mutex_lock(&ListMutex);
        {
            if (List_count(ReceivedList) > 0) 
            {
                Message = List_trim(ReceivedList);

                // If first char in Message is !, but the following chars are not '\n', then we don't want to exit 
                // So, set TimeToClose flag to false 
                bool TimeToClose = true; 
                if (*Message == '!')
                {
                    for (int i = 1; Message[i] != '\0'; i++)
                    {
                        if (Message[i] != '\n')
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

                // Shutdown the threads and socket if Message is just one "!"
                if (TimeToClose == true)
                {
                    fputs("** The other user has left the chat **\n", stdout);
                    fputs("------ Closing Socket ------\n", stdout);

                    Keyboard_cancel();
                    Sender_cancel(); 
                    Receiver_cancel();
                    Printer_cancel(); 
                    SocketManager_Shutdown();
                }

                fputs(">> ", stdout); 
                fputs(Message, stdout);
            }
            else {
                free(Message);
                perror("List is empty, unable to receive message");
            }
        }
        pthread_mutex_unlock(&ListMutex);
    }
}

void Printer_init(List* inReceivedList) 
{
    ReceivedList = inReceivedList;

    pthread_create
    (
        &ThreadPID, 
        NULL, 
        PrinterThread, 
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

void Printer_shutdown()
{
    pthread_join(ThreadPID, NULL); 
    pthread_mutex_destroy(&ListMutex);
    List_free(ReceivedList, pItemFreeFn);
} 

void Printer_cancel()
{
    pthread_cancel(ThreadPID); 
} 



