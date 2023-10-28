#ifndef _RECEIVER_THREAD_H_
#define _RECEIVER_THREAD_H_

#include "list/list.h"

void Receiver_init(List* ReceivedList); 

void Receiver_shutdown(); 

void Receiver_tellPrintToWait();

void Receiver_cancel();

#endif 