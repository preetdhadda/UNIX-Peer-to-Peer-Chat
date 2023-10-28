#ifndef _SENDER_THREAD_H_
#define _SENDER_THREAD_H_

#include "list/list.h"

void Sender_init(List* SentList); 

void Sender_shutdown(); 

void Sender_cancel();
#endif 