/*
 * Demonstrate the use of the USB Host Library for SAMD IntelliKeys (IK) USB
 * host driver. Prints IK events as JSON on serial port.
 */

#include <IntelliKeys.h>

// On Arduino Zero debug on and send JSON to debug port
#if defined(ARDUINO_SAMD_ZERO)
#define DBSerial  if(1)Serial
#define JSON      Serial
#else
// All other boards including Trinket M0, debug off and send JSON to Serial1
#define DBSerial  if(0)Serial
#define JSON      Serial1
#endif

USBHost myusb;
IntelliKeys ikey1(&myusb);

void IK_press(int x, int y)
{
  char buf[80];
  int buflen;
  buflen = snprintf(buf, sizeof(buf),
      "{\"evt\":\"press\",\"x\":%d,\"y\":%d}", x, y);
  if (buflen > 0) {
    JSON.println(buf);
  }
}

void IK_release(int x, int y)
{
  char buf[80];
  int buflen;
  buflen = snprintf(buf, sizeof(buf),
      "{\"evt\":\"release\",\"x\":%d,\"y\":%d}", x, y);
  if (buflen > 0) {
    JSON.println(buf);
  }
}

void IK_switch(int switch_number, int switch_state)
{
  char buf[80];
  int buflen;
  buflen = snprintf(buf, sizeof(buf),
      "{\"evt\":\"switch\",\"num\":%d,\"st\":%d}",
      switch_number, switch_state);
  if (buflen > 0) {
    JSON.println(buf);
  }
}

void IK_sensor(int sensor_number, int sensor_value)
{
  char buf[80];
  int buflen;
  buflen = snprintf(buf, sizeof(buf),
      "{\"evt\":\"sensor\",\"num\":%d,\"val\":%d}",
      sensor_number, sensor_value);
  if (buflen > 0) {
    JSON.println(buf);
  }
}

void IK_version(int major, int minor)
{
  char buf[80];
  int buflen;
  buflen = snprintf(buf, sizeof(buf),
      "{\"evt\":\"fwver\",\"major\":%d,\"minor\":%d}", major, minor);
  if (buflen > 0) {
    JSON.println(buf);
  }
}

void IK_connect(void)
{
  JSON.println("{\"evt\":\"connect\"}");
}

void IK_disconnect(void)
{
  JSON.println("{\"evt\":\"disconnect\"}");
}

void IK_onoff(int onoff)
{
  char buf[80];
  int buflen;
  buflen = snprintf(buf, sizeof(buf),
      "{\"evt\":\"onoff\",\"val\":%d}", onoff);
  if (buflen > 0) {
    JSON.println(buf);
  }
}

char mySN[IK_EEPROM_SN_SIZE+1]; //+1 NUL

void IK_get_SN(uint8_t SN[IK_EEPROM_SN_SIZE])
{
  // The SN parameter is not NUL terminated!
  memcpy(mySN, SN, IK_EEPROM_SN_SIZE);
  mySN[IK_EEPROM_SN_SIZE] = '\0';
  JSON.print("{\"evt\":\"sernum\",\"sn\":\"");
  JSON.print(mySN);
  JSON.println("\"}");
}

void setup() {
  DBSerial.begin(115200);
  DBSerial.println("IntelliKeys USB Test");
  // If there are concerns about JSON transmission being too slow, boost
  // the UART speed. 4*115200 or 8*115200 works on the SAMD21. Make sure to
  // match the UART speed on the other side.
  JSON.begin(115200);
  myusb.Init();

  ikey1.onConnect(IK_connect);
  ikey1.onDisconnect(IK_disconnect);
  ikey1.onMembranePress(IK_press);
  ikey1.onMembraneRelease(IK_release);
  ikey1.onSwitch(IK_switch);
  ikey1.onSensor(IK_sensor);
  ikey1.onVersion(IK_version);
  ikey1.onOnOffSwitch(IK_onoff);
  ikey1.onSerialNum(IK_get_SN);
}

void loop() {
  myusb.Task();
  ikey1.Task();
}

