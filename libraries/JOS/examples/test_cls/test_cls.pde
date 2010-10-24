#include <JCls.h>

class MyStream: JOS::Stream {
  virtual boolean write(const byte*, int len) {
    return true;
  }
  virtual int in_avail() {
    return 3;
  }
  virtual int read(byte*, int len) {
    return len;
  }
};

void setup()
{
  MyStream* strm = new MyStream; 
  double pi = 3.14159265359;
  strm->write(pi);
}

void loop()
{
}
