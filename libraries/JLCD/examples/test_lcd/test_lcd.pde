#define DEBUG
#include <JOS.h>
#include <JLCD.h>

struct MyTask: JOS::Task {
  unsigned long counter;
  JOS::LCD* lcd;
  virtual boolean run();
  MyTask(): JOS::Task(), counter(0), lcd(0) {}
};

boolean MyTask::run() {
  if (counter == 0) {
    if (lcd != 0) {
      lcd->set_line(0);
      lcd->write("JOS Demo!");
    }
  }
  if (lcd != 0) {
    lcd->set_pos(1, 4);
    lcd->write(JOS::Format(8), counter);
  }
  ++counter;
  rest(1000000);
  return false;
}


void setup() 
{
  D_JOS("Constructing LCD");
  JOS::LCD* lcd = new JOS::LCD;
  
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

