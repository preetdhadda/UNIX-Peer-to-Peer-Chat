#ifndef _PRINT_THREAD_H_
#define _PRINT_THREAD_H_

#include "list/list.h"

void Printer_init(List* ReceivedList); 

void Printer_shutdown(); 

void Printer_cancel();

#endif 