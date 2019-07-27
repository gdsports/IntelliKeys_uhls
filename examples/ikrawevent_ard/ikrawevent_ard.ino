/*
 * Companion program to ikrawevent.
 *
 * ikrawevent runs on a Trinket M0 that plugs into the IK and handles USB host.
 * The events/commands are send/received over a UART interface.
 *
 * ikrawevent_cp runs on a Trinket M0 or any board running CircuitPython.
 * It talks over its UART interface to the Trinket M0 running the above
 * program.
 *
 * This program ikrawevent_ard does the same as ikrawevent_cp but is
 * written in C/C++ Arduino.
 */

#include <intellikeysdefs.h>
#include <Keyboard.h>
#include <Mouse.h>

#define DBSerial    Serial
#define IKSerial    Serial1

void setup()
{
  DBSerial.begin(115200);
  while(!DBSerial) delay(1);
  IKSerial.begin(115200);

  Keyboard.begin();
  Mouse.begin();
  IK_uart_setup();
  DBSerial.println("ikrawevent_ard setup done");
}

// IK events
void IK_press(int x, int y)
{
  DBSerial.print("press ");
  DBSerial.print(x);
  DBSerial.print(',');
  DBSerial.println(y);
}

void IK_release(int x, int y)
{
  DBSerial.print("release ");
  DBSerial.print(x);
  DBSerial.print(',');
  DBSerial.println(y);
}

void IK_switch(int num, int state)
{
  DBSerial.print("switch ");
  DBSerial.print(num);
  DBSerial.print(',');
  DBSerial.println(state);
}

void IK_sensor(int num, int state)
{
  DBSerial.print("sensor ");
  DBSerial.print(num);
  DBSerial.print(',');
  DBSerial.println(state);
}

void IK_version(int major, int minor)
{
  DBSerial.print("FW version ");
  DBSerial.print(major);
  DBSerial.print('.');
  DBSerial.println(minor);
}

void IK_onoffswitch(int state)
{
  DBSerial.print("Top on/off switch ");
  DBSerial.println(state);
}

void IK_corrmemb(int x, int y)
{
  DBSerial.print("correct membrane ");
  DBSerial.print(x);
  DBSerial.print(',');
  DBSerial.println(y);
}

void IK_corrswitch(int x, int y)
{
  DBSerial.print("correct switch ");
  DBSerial.print(x);
  DBSerial.print(',');
  DBSerial.println(y);
}

void IK_corrdone()
{
  DBSerial.println("correct done");
}

void IK_connect()
{
  DBSerial.println("IK connect");
}

void IK_disconnect()
{
  DBSerial.println("IK disconnect");
}

void IK_sernum(const uint8_t *buf, size_t len)
{
  // The first byte is the event type so skip it
  DBSerial.print("IK serial number ");
  DBSerial.write(buf+1, len-1);
  DBSerial.println();
}

void eventDecode(const uint8_t *buf, size_t len)
{
  switch (buf[0]) {
    case IK_EVENT_MEMBRANE_PRESS:
      IK_press(buf[1], buf[2]);
      break;
    case IK_EVENT_MEMBRANE_RELEASE:
      IK_release(buf[1], buf[2]);
      break;
    case IK_EVENT_SWITCH:
      IK_switch(buf[1], buf[2]);
      break;
    case IK_EVENT_SENSOR_CHANGE:
      IK_sensor(buf[1], buf[2]);
      break;
    case IK_EVENT_VERSION:
      IK_version(buf[1], buf[2]);
      break;
    case IK_EVENT_ONOFFSWITCH:
      IK_onoffswitch(buf[1]);
      break;
    case IK_EVENT_CORRECT_MEMBRANE:
      IK_corrmemb(buf[1], buf[2]);
      break;
    case IK_EVENT_CORRECT_SWITCH:
      IK_corrswitch(buf[1], buf[2]);
      break;
    case IK_EVENT_CORRECT_DONE:
      IK_corrdone();
      break;
    case IK_EVENT_CONNECT:
      IK_connect();
      break;
    case IK_EVENT_DISCONNECT:
      IK_disconnect();
      break;
    case IK_EVENT_SERNUM:
      IK_sernum(buf, len);
      break;
    default:
      DBSerial.print("IK eventDecode Unknown event ");
      DBSerial.println(buf[0]);
      break;
  }
}

// IK commands
void IK_get_version()
{
  uint8_t outbuf[] = {0xFF, 0x01, IK_CMD_GET_VERSION};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_set_led(uint8_t num, uint8_t state)
{
  uint8_t outbuf[] = {0xFF, 0x03, IK_CMD_LED, num, state};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_set_tone(uint8_t frequency, uint8_t duration, uint8_t volume)
{
  uint8_t outbuf[] = {0xFF, 0x03, IK_CMD_TONE, frequency, duration, volume};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_get_onoff()
{
  uint8_t outbuf[] = {0xFF, 0x01, IK_CMD_ONOFFSWITCH};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_get_correct()
{
  uint8_t outbuf[] = {0xFF, 0x01, IK_CMD_CORRECT};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_reset()
{
  uint8_t outbuf[] = {0xFF, 0x01, IK_CMD_RESET_DEVICE};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_get_all_sensors()
{
  uint8_t outbuf[] = {0xFF, 0x01, IK_CMD_ALL_SENSORS};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_get_sn()
{
  uint8_t outbuf[] = {0xFF, 0x01, IK_CMD_GET_SN};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_uart_setup()
{
  int i;

  // All LEDs on
  for (i = 0; i < 12; i++) IK_set_led(i, 1);

  IK_set_tone(0,0,0);
  IK_get_onoff();
  IK_get_all_sensors();
  IK_get_version();
  IK_get_sn();
  IK_uart_loop();
  IK_get_correct();
  IK_uart_loop();

  // All LEDs off
  for (i = 0; i < 12; i++) IK_set_led(i, 0);
}

void IK_uart_loop()
{
  static uint8_t eventState=0;
  static size_t  eventLen=0;
  static uint8_t eventBuf[64];
  static uint8_t eventIndex;
  uint8_t rawBuf[64];

  int bytesAvail;
  while ((bytesAvail = IKSerial.available()) > 0) {
    size_t bytesIn = IKSerial.readBytes(rawBuf, min(bytesAvail, sizeof(rawBuf)));
    for (size_t i = 0; i < bytesIn; i++) {
      switch(eventState) {
        case 0:
          if (rawBuf[i] == 0xFF) {
            eventState = 1;
          }
          break;
        case 1:
          eventLen = rawBuf[i];
          eventState = 2;
          eventIndex = 0;
          break;
        case 2:
          eventBuf[eventIndex] = rawBuf[i];
          eventIndex++;
          if (eventIndex >= eventLen) {
            eventState = 0;
            eventIndex = 0;
            eventDecode(eventBuf, eventLen);
          }
          break;
        default:
          DBSerial.println("Invalid eventState");
          break;
      }
    }
  }
}

void loop()
{
  IK_uart_loop();
}
