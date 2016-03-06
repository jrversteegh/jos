## JOS ##

These embedded libraries include a set of task based components that do non-blocking IO (no calls to "delay" or "while" loops) A central tasklist runs all tasks in a loop. Each task indicates whether it is runnable and determines what it should do when running by means of a task state.

Currently the following components are available:
  * Hardware serial (no block on write -> tx buffer)
  * 4 bit LCD display (HD44780, no blocking wait between commands -> one command at a time)
  * 128x64 graphic LCD (ks0108 under development)
  * Ethershield (WIZnet w5100, enc28j60 under development)

Since each task quickly returns to the mainloop without blocking, the arduino remains responsive and won't miss e.g. IO pin signals.

If you're interested or have questions, you can email me at j.r.versteegh @ gmail.
