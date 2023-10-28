#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

#include "SocketManager.h"
#include "ReceiveThread.h"
#include "SendThread.h"
#include "KeyboardThread.h"
#include "PrintThread.h"
#include "list/list.h"

#define MSG_MAX_LEN 1024

static void pItemFreeFn(void *Item)
{
    if (Item != NULL)
    {
        free(Item);
    }
}

int main (int argc, char **args)
{
    puts("------ Opening Socket ------");

    List* SentList = List_create();         // Shared in send thread and keyboard thread
    List* ReceivedList = List_create();     // Shared in receive thread and print thread 

    // Startup of the socket 
    SocketManager_Init(atoi(args[1]), args[2], args[3]);

    // Startup of threads
    Sender_init(SentList); 
    Keyboard_init(SentList);
    Receiver_init(ReceivedList);
    Printer_init(ReceivedList); 

    // Shutdown of threads
    Keyboard_shutdown();
    Sender_shutdown(); 
    Receiver_shutdown();
    Printer_shutdown(); 

    // Free up memory in lists 
    List_free(SentList, pItemFreeFn);
    List_free(ReceivedList, pItemFreeFn);

    return 0;
}