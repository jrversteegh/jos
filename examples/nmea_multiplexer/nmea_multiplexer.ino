/* vim: set filetype=cpp: */
//#define DEBUG
#include <JDbg.h>
#include <JOS.h>
#include <JSer.h>
#include <JCls.h>

// Whether to prefix the NMEA output with the port number
// the data was received from
static boolean send_port_no = false;

int checksum(JOS::String& s) {
  int i = 0;
  byte c;
  s.rewind();
  while(s.read(&c, 1))
    if (c == '$') 
      break;
  while(s.read(&c, 1)) {
    if (c == '*') 
      break;
    i ^= c;
  }
  s.rewind();  
  return i;
}

static const JOS::Format cks_fmt(2, 16, 0, '0');

JOS::String& append_checksum(JOS::String& s) {
  int cks = checksum(s);
  s.write(cks_fmt, cks);
  return s;
}

boolean check_checksum(JOS::String& s) {
  static JOS::String sum;
  if (s.len() < 2)
    return false;
  int cks = checksum(s);
  sum.clear();
  sum.write(cks_fmt, cks);
  JOS::Slice slc(s, -2, 0);
  return sum == slc;
}

struct LedFlash: JOS::Task {
  LedFlash(int led): Task(), led_(led) {}
  virtual boolean run();
private:
  int led_;
  boolean state_;
};

boolean LedFlash::run() {
  rest(500000); // Delay 500ms: flash led at 1 Hz 
  digitalWrite(led_, state_);  
  state_ = !state_;
  return false; // We're never done
}

struct Ping: JOS::Task {
  Ping(JOS::Output_stream* output): 
      output_(output), ping_("$PPING,MULTIPLEXER*"), port0_("0:") {
    append_checksum(ping_);
    ping_ << "\r\n";
  }
  virtual boolean run();
private:
  JOS::Output_stream* output_;
  JOS::String ping_;
  JOS::String port0_;
};

boolean Ping::run() {
  rest(10000000); // Delay 100000ms: send ping every 10s
  D_JOS("Ping!");
  ping_.rewind();
  if (send_port_no) {
    port0_.rewind();
    *output_ << port0_;
  }
  *output_ << ping_;
  return false; // We're never done
}

struct Multiplexer: JOS::Task {
  virtual boolean run();
  Multiplexer(
    JOS::Output_text* output,
    JOS::Input_stream* input1,
    JOS::Input_stream* input2,
    JOS::Input_stream* input3
  ): JOS::Task(), output_(output), input1_(input1), input2_(input2), input3_(input3),
                  sentence1_(), sentence2_(), sentence3_() {}
private:
  void handle_input(int port, JOS::Input_stream& input, JOS::String& sentence);
  void handle_command();
  JOS::Output_text* output_;
  JOS::Input_stream* input1_;
  JOS::Input_stream* input2_;
  JOS::Input_stream* input3_;
  JOS::String sentence1_;
  JOS::String sentence2_;
  JOS::String sentence3_;
};

boolean Multiplexer::run() {
  rest(20000); // Delay 20ms: run service at 50 Hz
  handle_input(1, *input1_, sentence1_);
  handle_input(2, *input2_, sentence2_);
  handle_input(3, *input3_, sentence3_);
  return false; // We're never done!
}

void Multiplexer::handle_input(int port, JOS::Input_stream& input, JOS::String& sentence)
{
  byte c;
  while (input.read(&c, 1)) {
    switch(c) {
      case '\n':
        if (sentence.len() == 0)
          break;
      case '\r': 
        if (check_checksum(sentence)) {
          if (send_port_no) {
            *output_ << port;
            *output_ << ":";
          }
          *output_ << sentence;
          *output_ << "\r\n";
        }
        else{
          D_JOS("Invalid NMEA checksum");
        }
        sentence.clear();
        break;
      default:
        sentence.write(&c, 1);
    }
  }
}

struct CommandHandler: public JOS::Task {
  virtual boolean run();
  CommandHandler(JOS::Input_stream* input): input_(input) {}
private:
  JOS::Input_stream* input_;
};

boolean CommandHandler::run() 
{
  static JOS::String command;
  rest(500000);
  byte c;
  while (input_->read(&c, 1)) {
    switch(c) {
      case '\n':
      case '\r': 
        D_JOS(command.c_str());
        if (command == "OPRT") {
          D_JOS("Enabling port output");
          send_port_no = true;
        }
        else if (command == "NPRT") {
          D_JOS("Disabling port output");
          send_port_no = false;
        }
        else {
          D_JOS("Unrecognised command");
        }
        command.clear();
        break;
      default:
        command.write(&c, 1);
    }
  }
  return false; // We're never done!
}

void setup() 
{
  D_JOS("Heap:");
  D_JOS((int)__malloc_heap_start);
  D_JOS((int)__malloc_heap_end);
  D_JOS("Constructing Serials");
  JOS::Text_serial* serial1 = new JOS::Text_serial(9600, 0);
  JOS::Serial* serial2 = new JOS::Serial(4800, 1);
  JOS::Serial* serial3 = new JOS::Serial(4800, 2);
  JOS::Serial* serial4 = new JOS::Serial(4800, 3);
  
  D_JOS("Constructing Multiplexer");
  Multiplexer* task = new Multiplexer(
    serial1, serial2, serial3, serial4
  );
  
  D_JOS("Adding Serial tasks");
  JOS::tasks.add(serial1);
  JOS::tasks.add(serial2);
  JOS::tasks.add(serial3);
  JOS::tasks.add(serial4);
  
  D_JOS("Adding Multiplexer");
  JOS::tasks.add(task);
  
  D_JOS("Constructing and adding LED flash");
  LedFlash* led = new LedFlash(13);
  JOS::tasks.add(led);
  
  D_JOS("Constructing and adding Ping");
  Ping* ping = new Ping(serial1);
  JOS::tasks.add(ping);
  
  D_JOS("Constructing and adding Command Handler");
  CommandHandler* command_handler = new CommandHandler(serial1);
  JOS::tasks.add(command_handler);

  // Pull RS485 enabler lines down
  pinMode(24, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(28, OUTPUT);
  pinMode(30, OUTPUT);
  digitalWrite(24, LOW);
  digitalWrite(26, LOW);
  digitalWrite(28, LOW);
  digitalWrite(30, LOW);
}

void loop()
{
  JOS::tasks.run();
}

