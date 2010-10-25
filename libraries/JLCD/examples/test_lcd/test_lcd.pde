#define DEBUG
#include <JOS.h>
#include <JLCD.h>

struct MyTask: JOS::Task {
  unsigned long counter;
  JOS::LCD* lcd;
  virtual boolean run();
  MyTask(): JOS::Task(), counter(0), lcd(0) {}
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
  if (counter == 0) {
    if (lcd != 0) lcd->print("JOS Demo!");
  }
  value_to_buf(counter_str, counter);
  if (lcd != 0) lcd->print(1, counter_str, 4);
  ++counter;
  rest(1000000);
  return false;
}


void setup() 
{
  D_JOS("Constructing LCD");
  JLCD::LCD* lcd = new JLCD::LCD;
  
  D_JOS("Constructing MyTask");
  MyTask* task = new MyTask;
  task->lcd = lcd;
  D_JOS("Adding MyTask");
  JOS::tasks.add(task);
}

void loop()
{
  JOS::tasks.run();
}

