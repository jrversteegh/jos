#define DEBUG
#include <JOS.h>

struct MyObj {
  MyObj() {
    D_JOS("Constructing myobj");
  }
  ~MyObj() {
    D_JOS("Destroying myobj");
  }
};

static MyObj* myobj = 0;

struct MyTask: JOS::Task {
  int i;
  MyTask(): JOS::Task(), i(0) {
    D_JOS("Constructing task");
  }
  ~MyTask() {
    D_JOS("Destroying task");
  }
protected:
  virtual boolean run() {
    if (myobj == 0) {
      myobj = new MyObj();
    }
    else {
      delete myobj;
      myobj = 0;
      ++i;
      if (i > 8) {
        return true;
      }
    }
    return false;
  }
};

void setup() 
{
  D_SETUP;
  D_JOS("Running object new/delete sequence");
  MyTask* task = new MyTask();
  JOS::tasks.add(task);
}

void loop()
{
  JOS::tasks.run();
}

