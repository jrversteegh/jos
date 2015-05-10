/*
  JOS.cpp - Main task library for JOS
  Copyright (c) 2010 Jaap Versteegh.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "JOS.h"
#include <avr/interrupt.h>

#if PANIC_REBOOT != 0
#include <avr/wdt.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

  int __cxa_guard_acquire(__guard *g)  
  {
    return !*(char *)(g);
  }

  void __cxa_guard_release (__guard *g) 
  {
    *(char *)g = 1;
  }

  void __cxa_guard_abort (__guard *) 
  {
  }

  void __cxa_pure_virtual(void) 
  {
  }

  int atexit(void (*func)()) {
    //func();
    return 0;
  }
#ifdef __cplusplus
}
#endif

void panic()
{
  // Not yet sure what to do on panic, but we'll have to halt.
  // We'll flash a led to alert
  D_JOS("Panic!");
#if defined(__AVR_ATmega8__)
  UCSRB = 0;
#else
  UCSR0B = 0;
#endif
#if PANIC_REBOOT != 0
  wdt_enable(WDTO_4S);
#endif
  pinMode(PANIC_LED_PIN, OUTPUT);
  while (true) {
    digitalWrite(PANIC_LED_PIN, !digitalRead(PANIC_LED_PIN));
    delay(50);
  };
}

void * operator new(size_t size) throw()
{
  D_JOS("New Alloc:");
  void * result = malloc(size);
  D_JOS((int)result);

  if (result == NULL) {
    D_JOS("Out of memory");
    panic();
  }
  return result;
}

void operator delete(void * ptr) throw()
{
  D_JOS("Delete Dealloc:");
  D_JOS((int)ptr);
  free(ptr);
  D_JOS("Done");
} 
 
namespace JOS {

void Task::rest(const unsigned long microsecs)
{
  _continue_at = micros() + microsecs;
  // zero has the special meaning: "not suspended", so avoid it here.
  // waiting an extra microsecond won't hurt ;)
  if (_continue_at == 0) {
    _continue_at = 1;
  }
}

boolean Task::suspended()
{
  if (_continue_at != 0) {
    unsigned long diff = micros() - _continue_at;
    boolean result = diff > 0x7FFFFFFF;
    if (!result) {
      _continue_at = 0;
    }
    return result;
  }
  else {
    return false;
  }
}

boolean Task::run_task()
{
  D_JOS("Running task");
  boolean result = false;
  if (!suspended()) {
    D_JOS("Task wasn't suspended");
    result = run();
    ++_run_state;
  }
  else {
    D_JOS("Task was suspended");
  }
  return result;
}

void TaskList::add(Task* task)
{
  D_JOS("TaskList.add()");
  task->_task_list = this;
  if (_size >= _list_size) {
    _list_size += 4;
    _list = (Task**)realloc(_list, _list_size * sizeof(Task*));
    if (_list == NULL) {
      panic();
    }
  }
  _list[_size] = task;
  ++_size;
}

int TaskList::count() const 
{
  return _size;
}

void TaskList::run_task(int item)
{
  if (_list[item]->run_task()) {
    // When run_task returned true, the task is complete
    D_JOS("Task finished");
    Task* finished_task = _list[item];
    if (finished_task->_next != 0) {
      // Continue with next sequential task
      _list[item] = finished_task->_next;
      _list[item]->prev_completed(finished_task);
    }
    else {
      // Clean up
      --_size;
      while (item < _size) {
        _list[item] = _list[item + 1];
        ++item;
      }
      if ((_list_size - _size) > 4) {
        _list_size -= 4;
        _list = (Task**)realloc(_list, _list_size * sizeof(Task*));
        if (_list == NULL) {
          panic();
        }
      }
    }
    D_JOS("Deleting finished task");
    delete finished_task;
  } 
}

void TaskList::run()
{
  // run high priority tasks first
  for (int i = 0; i < _size; ++i) {
    if (_list[i]->_high_priority) {
      run_task(i);
    }
  }
  for (int i = 0; i < _size; ++i) {
    run_task(i);
  }
}

/* Global task list */
TaskList tasks;

}  // namespace JOS
