#include <JCls.h>

struct Test: JOS::Task {
  virtual boolean run() {
    return false;
  }
};

void setup()
{
  JOS::String str;
  double pi = 3.14159265359;
  str.write(pi);
}

void loop()
{
  JOS::tasks.run();
}
