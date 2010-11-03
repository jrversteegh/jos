/*
  JOS.h - Main task library for JOS
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

/**
 * \file
 * Main header file for JOS.
 *
 * \mainpage
 * \section JOS
 * This is JOS, a system of low latency arduino libraries.
 */

#ifndef __JOS_H__
#define __JOS_H__

//#define DEBUG
#include "JOS_config.h"
#include "JDbg.h"

#include <stdlib.h>
#include <wiring.h>
#undef round
#include <math.h>

#if PANIC_REBOOT != 0
#include <avr/wdt.h>
#endif

__extension__ typedef int __guard __attribute__((mode (__DI__)));

#ifdef __cplusplus
extern "C" {
#endif

/** Implemented to keep linker happy with respect to dynamic object allocation */
int __cxa_guard_acquire(__guard *);
/** Implemented to keep linker happy with respect to dynamic object allocation */
void __cxa_guard_release (__guard *);
/** Implemented to keep linker happy with respect to dynamic object allocation */
void __cxa_guard_abort (__guard *); 
/** Implemented to keep linker happy with respect to dynamic object allocation */
void __cxa_pure_virtual(void);

#ifdef __cplusplus
}
#endif

/**
 * Memory allocator
 * Allocate memory for and constuct an object
 * new is not defined or implemented by default in avr, but we need it
 * so define and implement here. new will panic() instead of throwing an
 * exception in case an allocation fails.
 * \param size Size of object to allocate
 * \return Pointer to new object
 */
void * operator new(size_t size) throw();
/**
 * Memory disposer
 * Destroy object pointed to by ptr and dispose of its memory.  
 * \param ptr Pointer to object to be disposed of.
 */
void operator delete(void * ptr) throw(); 
/**
 * Fatal error hander.
 * Call when arduino can no longer continue due to non recoverable error.
 */
void panic();

namespace JOS {


struct TaskList;

struct Task {
  Task(): _run_state(0), _running(false), _high_priority(false),
      _continue_at(0), _next(0), _task_list(0) {}
  // Don't destroy tasks explicitly, but rather have the "run"
  // method return true. This signals task completion upon 
  // which the tasklist will delete the task
  virtual ~Task() {}
  // rest before start of next execution 
  void rest(const unsigned long microsecs);
  
  void boost_priority() {
    _high_priority = true;
  }   
  void set_predecessor(Task* task) {
    task->_next = this;
  }
  friend class TaskList;
protected:
  static const int run_state_default = 0xFE;
  byte _run_state;
  virtual boolean run() = 0;
  virtual void prev_completed(Task* prev_task) {};
  virtual boolean suspended();
private:
  boolean _running;
  boolean _high_priority;
  unsigned long _continue_at;
  Task* _next; 
  TaskList* _task_list;

  boolean run_task();
};

struct TaskList {
  TaskList(): _size(0), _list_size(4) { 
    // Create a default list with space for 4 tasks
    _list = (Task**)malloc(_list_size * sizeof(Task*)); 
    if (_list == NULL) {
      panic;
    }
#if PANIC_REBOOT != 0
    wdt_disable();
#endif
  }
  void add(Task* task);
  int count() const;
  void run();
  friend class Task;
private:
  int _size;
  int _list_size;
  Task** _list;
  void run_task(int item);
};

/* Global task list */
extern TaskList tasks;

}  // namespace JOS


#endif
