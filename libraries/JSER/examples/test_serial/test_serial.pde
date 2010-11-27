#define DEBUG
#include <JOS.h>
#include <JSer.h>
#include <wiring_private.h>

// Task sends out a buffer of data over the serial port
// and then echo's incoming data
struct My_task: JOS::Task {
  // Just a counter for recording how often the task has run
  unsigned long counter;
  // Serial port instance
  JOS::Serial* serial;
  // Task implementation goes in run!
  virtual boolean run();
  My_task(): JOS::Task(), counter(0), serial(0) {}
};

#define BUFSIZE 256
static byte buf[BUFSIZE];

boolean My_task::run() {

  // static because it keeps track of the amount of data sent
  // so its value should persist through multiple calls
  static int i = 0;  

  // Local buffer for storing incoming bytes
  int j;
  byte local_buf[16];

  if (serial != 0) {
    // Not all data may be send at once (due to limited buffer size)
    // so keep track of the data sent with "i"
    int writeable = serial->writeable();
    int len = BUFSIZE - i;
    if (writeable < len)
      len = writeable;
    if (serial->write(&buf[i], len)) {
      i += len;
    }
    
    // Echo any incoming bytes
    if (j = serial->read(local_buf, 16)) {
      serial->write(local_buf, j);
    }
  };
  // Since this is a low latency library, "counter" should
  // be a reasonably good runtime indication (seconds since start)
  ++counter;
  // Don't run again for 1M micros (1s)
  rest(1000000);
  return false;
}

void setup() 
{
  D_JOS(""); // Force a new line
  D_JOS("Constructing Serial");
  JOS::Serial* serial = new JOS::Serial(9600, 0);

  // Fill buffer with all possible byte values to send as test
  for (int i = 0; i < BUFSIZE; ++i) {
    buf[i] = i;
  }
  
  D_JOS("Constructing MyTask");
  My_task* task = new My_task;
  task->serial = serial;
  
  // Add tasks to tasklist, so they will be run on JOS::tasks.run()
  D_JOS("Adding MyTask");
  JOS::tasks.add(task);
  D_JOS("Adding Serial");
  JOS::tasks.add(serial);
}

void loop()
{
  // Run tasks
  JOS::tasks.run();
}

