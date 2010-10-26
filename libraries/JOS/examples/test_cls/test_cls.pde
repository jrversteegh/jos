#define DEBUG
#include <JCls.h>

void setup()
{
  D_JOS("Starting tests!");
  JOS::String str;
  
  str = "e";
  str = "pi";
  J_ASSERT(str == "pi", "Failed string assignment");
  
  str += " = ";
  J_ASSERT(str == "pi = ", "Failed string concatenation");

  double pi = 3.14159265359;
  str.prec = 5;
  str.write(pi);
  J_ASSERT(str == "pi = 3.14159", "Failed double concatenation");
  
  int i = 265;
  str.write(i);
  J_ASSERT(str == "pi = 3.14159265", "Failed int concatenation");
  
  i = 857;
  str.base = 16;
  str.write(i);
  J_ASSERT(str == "pi = 3.14159265359", "Failed hex concatenation");

  str.setw(10);
  str += "e = 2.71";
  J_ASSERT(str == "pi = 3.14159265359  e = 2.71", "Failed spaced concatenation");
  
  str.skip(4);
  double d;
  str.read(&d);
  J_ASSERT(d == 3.1415927, "Failed double read");
  
  char c;
  while (!isdigit(str.peek()) && str.skip())
    ;
  str.read(&i);
  J_ASSERT(i == 2, "Failed int read");
  
  str = "-23";
  str.read(&i);
  J_ASSERT(i == -23, "Failed neg int read");
  
  D_JOS("Tests successful!");
}

void loop()
{
  JOS::tasks.run();
}
