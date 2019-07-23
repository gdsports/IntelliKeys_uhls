/*
 * Demonstrate the use of the USB Host Library for SAMD IntelliKeys (IK) USB
 * host driver. Send raw IK events out a Serial port
 */

#include <IntelliKeys.h>

// On Arduino Zero debug on and send JSON to debug port
#if defined(ARDUINO_SAMD_ZERO)
#define DBSerial  if(1)Serial
#define IKSerial  Serial1
#else
// All other boards including Trinket M0, debug off and send JSON to Serial1
#define DBSerial  if(0)Serial
#define IKSerial  Serial1
#endif

USBHost myusb;
IntelliKeys ikey1(&myusb);

char mySN[IK_EEPROM_SN_SIZE+1]; //+1 NUL

/******************************************************
#define EVENT_BASE 50
#define IK_EVENT_ACK                EVENT_BASE+1
#define IK_EVENT_MEMBRANE_PRESS     EVENT_BASE+2
#define IK_EVENT_MEMBRANE_RELEASE   EVENT_BASE+3
#define IK_EVENT_SWITCH             EVENT_BASE+4
#define IK_EVENT_SENSOR_CHANGE      EVENT_BASE+5
#define IK_EVENT_VERSION            EVENT_BASE+6
#define IK_EVENT_EEPROM_READ        EVENT_BASE+7
#define IK_EVENT_ONOFFSWITCH        EVENT_BASE+8
#define IK_EVENT_NOMOREEVENTS       EVENT_BASE+9
#define IK_EVENT_MEMBRANE_REPEAT    EVENT_BASE+10
#define IK_EVENT_SWITCH_REPEAT      EVENT_BASE+11
#define IK_EVENT_CORRECT_MEMBRANE   EVENT_BASE+12
#define IK_EVENT_CORRECT_SWITCH     EVENT_BASE+13
#define IK_EVENT_CORRECT_DONE       EVENT_BASE+14
#define IK_EVENT_EEPROM_READBYTE    EVENT_BASE+15
#define IK_EVENT_DEVICEREADY        EVENT_BASE+16
#define IK_EVENT_AUTOPILOT_STATE    EVENT_BASE+17
#define IK_EVENT_DELAY              EVENT_BASE+18
#define IK_EVENT_ALL_SENSORS        EVENT_BASE+19

// Arduino IK driver events
#define AIK_EVENT_BASE 80
#define IK_EVENT_CONNECT            AIK_EVENT_BASE+1
#define IK_EVENT_DISCONNECT         AIK_EVENT_BASE+2
#define IK_EVENT_SERNUM             AIK_EVENT_BASE+3

struct IKEvent
{
uint8_t startFlag;  // Always 0xFF.
uint8_t evLength;   // Total number of bytes starting with evType
uint8_t evType;     // See IK_EVENT_* above
uint8_t payload[];  // The actual number of bytes that follow
// could be 0..254.
}

The receiver should scan the stream of oxFF. The next byte is the number of
bytes in the event including evType. The next byte is the event type.
If the receiver does not understand how to handle the event, it can use the
length to skip over the remaining bytes so it can easily find the start of the
next event startFlag. This makes it possible to add new events which the
receiver cannot handle. The receiver can skip over unknown events until the
receiver is enhanced to handle the new events.

If 0xFF appears in the payload, this can cause problems. Hopefully, this will
resync on the next of 0xFF. Assuming dropped and corrupted characters are rare.

Solving this problem in general means adding flag byte escaping, ACK/NAK, retry
limits, inter-byte and inter-packet timeouts, etc. XMODEM or HDLC type of
protocol. Probably not a good candidate for CP.

 *********************************************************/

// Raw undecoded events from the IK. Just send them as-is.
// The byte following the length is the event type.
void IK_raw_event(const uint8_t *rxevent, size_t len)
{
  switch (*rxevent) {
    case IK_EVENT_MEMBRANE_PRESS:
    case IK_EVENT_MEMBRANE_RELEASE:
    case IK_EVENT_SWITCH:
    case IK_EVENT_CORRECT_MEMBRANE:
    case IK_EVENT_CORRECT_SWITCH:
    case IK_EVENT_VERSION:
      IKSerial.write(0xFF);
      IKSerial.write(3);
      IKSerial.write(rxevent, 3);
      break;

    case IK_EVENT_ONOFFSWITCH:
      IKSerial.write(0xFF);
      IKSerial.write(2);
      IKSerial.write(rxevent, 2);
      break;

    case IK_EVENT_CORRECT_DONE:
      IKSerial.write(0xFF);
      IKSerial.write(1);
      IKSerial.write(rxevent, 1);
      break;

    case IK_EVENT_SENSOR_CHANGE:    // See IK_sensor
    case IK_EVENT_EEPROM_READBYTE:  // See IK_get_SN
      break;

    default:
      USBTRACE2("Unknown event code=", *rxevent);
      break;
  }
}

void IK_sensor(int sensor_number, int sensor_value)
{
  uint8_t evt[5] = {0xFF, 3, IK_EVENT_SENSOR_CHANGE,
    (uint8_t)sensor_number, (uint8_t)sensor_value};
  IKSerial.write(evt, sizeof(evt));
}

// The following events are generated by the driver, not the IK.
void IK_connect(void)
{
  uint8_t buf[3] = {0xFF, 1, IK_EVENT_CONNECT};
  IKSerial.write(buf, sizeof(buf));
}

void IK_disconnect(void)
{
  uint8_t buf[3] = {0xFF, 1, IK_EVENT_DISCONNECT};
  IKSerial.write(buf, sizeof(buf));
}

void IK_get_SN(uint8_t SN[IK_EEPROM_SN_SIZE])
{
  memcpy(mySN, SN, IK_EEPROM_SN_SIZE);
  mySN[IK_EEPROM_SN_SIZE] = '\0';
}

void IK_put_SN()
{
  uint8_t buf[3] = {0xFF, IK_EEPROM_SN_SIZE+1, IK_EVENT_SERNUM};
  IKSerial.write(buf, sizeof(buf));
  IKSerial.write(mySN, IK_EEPROM_SN_SIZE);
}

/*
 * Commands

#define CMD_BASE 0
#define IK_CMD_GET_VERSION          CMD_BASE+1
#define IK_CMD_LED                  CMD_BASE+2
#define IK_CMD_SCAN                 CMD_BASE+3
#define IK_CMD_TONE                 CMD_BASE+4
#define IK_CMD_GET_EVENT            CMD_BASE+5
#define IK_CMD_INIT                 CMD_BASE+6
#define IK_CMD_EEPROM_READ          CMD_BASE+7
#define IK_CMD_EEPROM_WRITE         CMD_BASE+8
#define IK_CMD_ONOFFSWITCH          CMD_BASE+9
#define IK_CMD_CORRECT              CMD_BASE+10
#define IK_CMD_EEPROM_READBYTE      CMD_BASE+11
#define IK_CMD_RESET_DEVICE         CMD_BASE+12
#define IK_CMD_START_AUTO           CMD_BASE+13
#define IK_CMD_STOP_AUTO            CMD_BASE+14
#define IK_CMD_ALL_LEDS             CMD_BASE+15
#define IK_CMD_START_OUTPUT         CMD_BASE+16
#define IK_CMD_STOP_OUTPUT          CMD_BASE+17
#define IK_CMD_ALL_SENSORS          CMD_BASE+18

#define IK_CMD_GET_SN               CMD_BASE+40

struct IKCommand
{
uint8_t startFlag;  // Always 0xFF.
uint8_t cmdLength;  // Total number of bytes starting with cmdType
uint8_t cmdType;    // See IK_COMMAND_* above
uint8_t payload[];  // The actual number of bytes that follow
// could be 0..254.
}
*/

void readCommand()
{
  static uint8_t command[16];
  static size_t cmdLen;
  static uint8_t *p;
  static uint8_t cmdState;

  while (IKSerial.available() > 0) {
    size_t bytesIn;
    switch (cmdState) {
      case 0: // Scan for 0xFF
        if (IKSerial.read() == 0xFF) {
          cmdState = 1;
          DBSerial.println("cmdState>1");
        }
        break;
      case 1: // Get cmdLen value
        cmdLen = IKSerial.read();
        if (cmdLen > sizeof(command)) cmdLen = sizeof(command);
        p = command;
        cmdState = 2;
        DBSerial.println("cmdState>2");
        break;
      case 2: // Get cmdLen bytes
        bytesIn = IKSerial.readBytes(p, cmdLen - (p - command));
        if (bytesIn > 0) {
          p += bytesIn;
          if (cmdLen == (p - command)) {
            cmdState = 0;
            // Have a complete command
            execCommand(command, cmdLen);
            DBSerial.println("cmdState>0");
          }
        }
        break;
      default:
        DBSerial.print("Invalid cmdState ");
        DBSerial.println(cmdState);
        break;
    }
  }
}

void execCommand(const uint8_t *command, size_t len)
{
  DBSerial.print("cmd "); DBSerial.println(command[0]);
  switch (command[0]) {
    case IK_CMD_GET_VERSION:
      ikey1.get_version();
      break;
    case IK_CMD_LED:
      ikey1.setLED(command[1], command[2]);
      break;
    case IK_CMD_TONE:
      ikey1.sound(command[1], command[3], command[2]);
      break;
    case IK_CMD_ONOFFSWITCH:
      ikey1.get_onoff();
      break;
    case IK_CMD_CORRECT:
      ikey1.get_correct();
      break;
    case IK_CMD_RESET_DEVICE:
      ikey1.reset();
      break;
    case IK_CMD_ALL_SENSORS:
      ikey1.get_all_sensors();
      break;
    case IK_CMD_GET_SN:
      IK_put_SN();
      break;
    case IK_CMD_EEPROM_READBYTE:
    case IK_CMD_SCAN:
    case IK_CMD_GET_EVENT:
    case IK_CMD_INIT:
    case IK_CMD_EEPROM_READ:
    case IK_CMD_EEPROM_WRITE:
    case IK_CMD_START_AUTO:
    case IK_CMD_STOP_AUTO:
    case IK_CMD_ALL_LEDS:
    case IK_CMD_START_OUTPUT:
    case IK_CMD_STOP_OUTPUT:
      break;
    default:
      break;
  }
}

void setup() {
  DBSerial.begin(115200);
  DBSerial.println("IntelliKeys USB Test");
  // If there are concerns about IKSerial transmission being too slow, boost
  // the UART speed. 4*115200 or 8*115200 works on the SAMD21. Make sure to
  // match the UART speed on the other side.
  IKSerial.begin(115200);
  myusb.Init();

  ikey1.onConnect(IK_connect);
  ikey1.onDisconnect(IK_disconnect);
  ikey1.onSensor(IK_sensor);
  ikey1.onSerialNum(IK_get_SN);
  ikey1.onRawEvent(IK_raw_event);
  memset(mySN, 0, sizeof(mySN));
}

void loop() {
  myusb.Task();
  ikey1.Task();
  readCommand();
}