#define DEBUG
#include <JOS.h>
#include <JSer.h>
#include <wiring_private.h>

struct My_task: JOS::Task {
  unsigned long counter;
  JOS::Text_serial* serial;
  virtual boolean run();
  My_task(): JOS::Task(), counter(0), serial(0) {}
};

boolean My_task::run() {
  char counter_str[11];
  int i;
  byte buf[16];
  if (serial != 0) {
    serial->write("JOS Demo!");
    serial->writeln();
    serial->write(JOS::Format(16), counter);
    serial->writeln();
    if (serial->read(&i)) {
      serial->write("Int: ");
      serial->write(JOS::Format(4), i);
      serial->write(JOS::Format(4, 16), i);
      serial->write(JOS::Format(8, 2), i);
      serial->writeln();
    }
    if (i = serial->read(buf, 16)) {
      serial->write("Buf: ");
      serial->write(buf, i);
      serial->writeln();
    }
  };
  ++counter;
  rest(1000000);
  return false;
}

JOS::Text_serial* serial;

void setup() 
{
  D_JOS("");
  D_JOS("Constructing Serial");
  serial = new JOS::Text_serial(9600, 0);
  
  D_JOS("Constructing MyTask");
  My_task* task = new My_task;
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

