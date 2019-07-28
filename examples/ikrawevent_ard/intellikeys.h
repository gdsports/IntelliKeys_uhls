#ifndef _INTELLIKEYSDEFS_H_
#define _INTELLIKEYSDEFS_H_

enum IK_LEDS {
    IK_LED_SHIFT=1,
    IK_LED_CAPS_LOCK=4,
    IK_LED_MOUSE=7,
    IK_LED_ALT=2,
    IK_LED_CTRL_CMD=5,
    IK_LED_NUM_LOCK=8
};

//  resolution of the device
#define IK_RESOLUTION_X 24
#define IK_RESOLUTION_Y 24

//  number of switches
#define IK_NUM_SWITCHES  2

//  number of sensors
#define IK_NUM_SENSORS   3

//
//  command codes sent to the device
//  see firmware documentation for details
//

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

//
//  result codes/data sent to the software
//  see firmware documentation for details
//
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


#define IK_FIRSTUNUSED_EVENTCODE    EVENT_BASE+20

// Arduino IK driver events
#define AIK_EVENT_BASE 80
#define IK_EVENT_CONNECT            AIK_EVENT_BASE+1
#define IK_EVENT_DISCONNECT         AIK_EVENT_BASE+2
#define IK_EVENT_SERNUM             AIK_EVENT_BASE+3

//
//  number of light sensors for reading overlay bar codes
//
#define IK_MAX_SENSORS              3

#endif /* _INTELLIKEYSDEFS_H_ */
