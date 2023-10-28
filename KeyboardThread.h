#ifndef _KEYBOARD_THREAD_H_
#define _KEYBOARD_THREAD_H_

#include "list/list.h"

void Keyboard_init(List* SentList); 

void Keyboard_shutdown(); 

void Keyboard_tellSendToWait();

void Keyboard_cancel();
#endif 