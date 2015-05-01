#JOS

These arduino libraries include a set of task based components that do 
non-blocking IO (no calls to "delay" or "while" loops) A central tasklist
runs all tasks in a loop. Each task indicates whether it is runnable and 
determines what it should do when running by means of a task state.
This is an asynchronous or event driven approach, like e.g. python "Twisted",
which is a lightweight alternative for doing multiple things at once
without using threads.

Currently the following components are available:
 * Hardware serial (no block on write -> tx buffer)
 * 4 bit LCD display (no blocking wait between commands -> one  
   command at a time)
 * Wiznet ethershield (working on enc28j60) 

Since each task quickly returns to the mainloop without blocking, the 
arduino remains responsive and won't miss e.g. IO pin signals.
           