#include <JOS.h>
#include <JSer.h>
#include <wiring_private.h>

struct MyTask: JOS::Task {
  unsigned long counter;
  JOS::Serial* serial;
  virtual boolean run();
  MyTask(): JOS::Task(), counter(0), serial(0) {}
};

void value_to_buf(char* buf, unsigned long value, char pad = ' ') 
{
  boolean done = false;
  unsigned long div10 = value;
  buf[10] = 0;
  for (int i = 0; i < 10; ++i) {
    if (!done) {
      byte mod10 = div10 % 10;
      buf[9 - i] = mod10 + 0x30;
      div10 = div10 / 10;
      if (div10 == 0) {
        done = true;
      }
    }
    else {
      buf[9 - i] = pad;
    }
  }
}

boolean MyTask::run() {
  char counter_str[11];
  if (serial != 0) serial->println("JOS Demo!");
  value_to_buf(counter_str, counter);
  if (serial != 0) serial->println(counter_str);
  ++counter;
  rest(1000000);
  return false;
}

JOS::Serial* serial;

void setup() 
{
  D_JOS("Constructing Serial");
  serial = new JOS::Serial(9600, 0);
  
  D_JOS("Constructing MyTask");
  MyTask* task = new MyTask;
  task->serial = serial;
  D_JOS("Adding MyTask");
  JOS::tasks.add(task);
  D_JOS("Adding Serial");
  JOS::tasks.add(serial);
}

void loop()
{
  JOS::tasks.run();
}

