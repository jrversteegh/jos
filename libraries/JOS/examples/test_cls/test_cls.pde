#define DEBUG
#include <JOS.h>
#include <JCls.h>

void setup()
{
  D_JOS("");
  D_JOS("Starting tests!");
  JOS::String str;
  
  str = "e";
  str = "pi";
  J_ASSERT(str == "pi", "Failed string assignment");
  
  str += " = ";
  J_ASSERT(str == "pi = ", "Failed string concat");

  double pi = 3.14159265359;
  str.format.precision = 5;
  str.write(pi);
  J_ASSERT(str == "pi = 3.14159", "Failed double concat");
  
  int i = 265;
  str.write(i);
  J_ASSERT(str == "pi = 3.14159265", "Failed int concat");
  
  i = 857;
  str.format.base = 16;
  str.write(i);
  J_ASSERT(str == "pi = 3.14159265359", "Failed hex concat");

  str.format.width = 10;
  str += "e = 2.71";
  J_ASSERT(str == "pi = 3.14159265359  e = 2.71", "Failed spaced concat");
  
  str.skip(4);
  J_ASSERT(str[5] == '3', "Failed string char check");
  double d;
  str.read(&d);
  J_ASSERT(d == 3.1415927, "Failed double read");
  
  while (!isdigit(str.peek()) && str.skip())
    ;
  str.read(&i);
  J_ASSERT(i == 2, "Failed int read");
  
  str = "-23";
  str.read(&i);
  J_ASSERT(i == -23, "Failed neg int read");
  
  str = "value = 88";
  str.read(&i);
  J_ASSERT(i == -23, "Failed noskip int read");
  
  str.skipall = true;
  str.read(&i);
  J_ASSERT(i == 88, "Failed skip int read");

  D_JOS("Tests successful!");
}

void loop()
{
  JOS::tasks.run();
}
