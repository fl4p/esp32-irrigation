#include "esphome.h"

// References
// https://esphome.io/components/sensor/custom.html?highlight=custom

class Uart;
static Uart*inst=nullptr;
class Uart: public Component, public UARTDevice {
public:

Uart(UARTComponent *parent) : UARTDevice(parent) {
ESP_LOGI("uart_sensor", "Uart() %p", this);
inst=this;
}

  int readline(int readch, char *buffer, int len)
  {
    static int pos = 0;
    int rpos;

    if (readch > 0) {
      switch (readch) {
        case '\n': // Ignore new-lines
          break;
        case '\r': // Return on CR
          buffer[pos] = 0;
          rpos = pos;
          pos = 0;  // Reset position index ready for next time
          return rpos;
        default:
          if (pos < len-1) {
            buffer[pos++] = readch;
          }
      }
    }
    // No end of line has been found, so return -1.
    return -1;
  }

  const char * getLine(int timeout=2000) {
    const int max_line_length = 30;
    static char buffer[max_line_length];
    auto t_start = millis();

    do {
    while (available()) {
      if(readline(read(), buffer, max_line_length) > 0) {
       ESP_LOGI("uart_sensor", "readLine %s", buffer);
        return buffer;
      }
    }
    delay(1);
    } while(millis() - t_start < timeout);
    buffer[max_line_length-1] = 0;
    ESP_LOGW("uart_sensor", "Timeout or line too long, read '%s'", buffer);
    return "";
  }

  size_t writeLine(std::string line) {
  for(auto c : line) write(c);
  write('\r');
  write('\n');
  return 0;
  }

  float queryFloat(std::string cmd, int timeout=2000) {
      ESP_LOGI("uart_sensor", "queryFloat");
    writeLine(cmd);
    ESP_LOGI("uart_sensor", "wrote %s", cmd.c_str());
    auto line= getLine(timeout);
     ESP_LOGI("uart_sensor", "read %s", line);
    if(strlen(line) == 0) return NAN;
    return std::atof(line);
  }
};

class UartPollingSensor : public PollingComponent, public Sensor {
 public:
  std::string cmd;
  Uart *uart;
  UartPollingSensor( const std::string &cmd, int interval) : PollingComponent(interval), cmd(cmd)
  //, uart((Uart*)const_cast<esphome::custom_component::CustomComponentConstructor*>(&uart))
  { }

  void setup() override {

  }

  void update() override {
  ESP_LOGI("uart_sensor", "query %s %p", cmd.c_str(), inst);
    auto value = inst->queryFloat(cmd);
    //if(std::isnan(value
    publish_state(value);
    }
};
