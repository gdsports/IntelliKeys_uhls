# CircuitPython IntelliKeys Example using binary protocol
#
# see the Arduino ikrawevent example for details
# On Trinket M0 and other non-Express boards, add adafruit_bus_device to 
# lib directory. This is to get the uart object.
# Tested on Trinket M0 running
#   Adafruit CircuitPython 4.1.0-rc.1 on 2019-07-19; Adafruit Trinket M0 with samd21e18

import array
import board
import busio
import time

# Try timeout 0 for non-blocking read so other things can be done
# in the main loop.
uart = busio.UART(board.TX, board.RX, baudrate=115200, timeout=0)

# IntelliKeys Events
eventState=0
eventLen=0
eventBuf=array.array('B', [0]*64)

IK_EVENT_ACK                = 51
IK_EVENT_MEMBRANE_PRESS     = 52
IK_EVENT_MEMBRANE_RELEASE   = 53
IK_EVENT_SWITCH             = 54
IK_EVENT_SENSOR_CHANGE      = 55
IK_EVENT_VERSION            = 56
IK_EVENT_EEPROM_READ        = 57
IK_EVENT_ONOFFSWITCH        = 58
IK_EVENT_NOMOREEVENTS       = 59
IK_EVENT_MEMBRANE_REPEAT    = 60
IK_EVENT_SWITCH_REPEAT      = 61
IK_EVENT_CORRECT_MEMBRANE   = 62
IK_EVENT_CORRECT_SWITCH     = 63
IK_EVENT_CORRECT_DONE       = 64
IK_EVENT_EEPROM_READBYTE    = 65
IK_EVENT_DEVICEREADY        = 66
IK_EVENT_AUTOPILOT_STATE    = 67
IK_EVENT_DELAY              = 68
IK_EVENT_ALL_SENSORS        = 69
IK_EVENT_CONNECT            = 81
IK_EVENT_DISCONNECT         = 82
IK_EVENT_SERNUM             = 83

def IK_press(x, y):
    print("press", x, y)

def IK_release(x, y):
    print("release", x, y)

def IK_switch(num, state):
    print("switch", num, state)

def IK_sensor(num, state):
    print("sensor", num, state)

def IK_version(major, minor):
    print("FW version", major, minor)

def IK_onoffswitch(state):
    print("Top switch", state)

def IK_corrmemb(x, y):
    print("correct membrane", x, y)

def IK_corrswitch(x, y):
    print("correct switch", x, y)

def IK_corrdone():
    print("correct done")

def IK_connect():
    print("IK connect")

def IK_disconnect():
    print("IK disconnect")

def IK_sernum(buf, len):
    # The first byte is the event type so skip it
    sernum=""
    for i in range(1, len):
        sernum = sernum + chr(buf[i])
    print("IK serial number", sernum)

def eventDecode(buf, len):
    if (buf[0] == IK_EVENT_MEMBRANE_PRESS):
        IK_press(buf[1], buf[2])
    elif (buf[0] == IK_EVENT_MEMBRANE_RELEASE):
        IK_release(buf[1], buf[2])
    elif (buf[0] == IK_EVENT_SWITCH):
        IK_switch(buf[1], buf[2])
    elif (buf[0] == IK_EVENT_SENSOR_CHANGE):
        IK_sensor(buf[1], buf[2])
    elif (buf[0] == IK_EVENT_VERSION):
        IK_version(buf[1], buf[2])
    elif (buf[0] == IK_EVENT_ONOFFSWITCH):
        IK_onoffswitch(buf[1])
    elif (buf[0] == IK_EVENT_CORRECT_MEMBRANE):
        IK_corrmemb(buf[1], buf[2])
    elif (buf[0] == IK_EVENT_CORRECT_SWITCH):
        IK_corrswitch(buf[1], buf[2])
    elif (buf[0] == IK_EVENT_CORRECT_DONE):
        IK_corrdone()
    elif (buf[0] == IK_EVENT_CONNECT):
        IK_connect()
    elif (buf[0] == IK_EVENT_DISCONNECT):
        IK_disconnect()
    elif (buf[0] == IK_EVENT_SERNUM):
        IK_sernum(buf, len)
    else:
        print("Unknown event")

# IntelliKeys Commands

IK_CMD_GET_VERSION          = 1
IK_CMD_LED                  = 2
IK_CMD_SCAN                 = 3
IK_CMD_TONE                 = 4
IK_CMD_GET_EVENT            = 5
IK_CMD_INIT                 = 6
IK_CMD_EEPROM_READ          = 7
IK_CMD_EEPROM_WRITE         = 8
IK_CMD_ONOFFSWITCH          = 9
IK_CMD_CORRECT              = 10
IK_CMD_EEPROM_READBYTE      = 11
IK_CMD_RESET_DEVICE         = 12
IK_CMD_START_AUTO           = 13
IK_CMD_STOP_AUTO            = 14
IK_CMD_ALL_LEDS             = 15
IK_CMD_START_OUTPUT         = 16
IK_CMD_STOP_OUTPUT          = 17
IK_CMD_ALL_SENSORS          = 18
IK_CMD_GET_SN               = 40

def IK_get_version():
    uart.write(array.array('B', [0xFF, 0x01, IK_CMD_GET_VERSION]))

def IK_set_led(num, state):
    uart.write(array.array('B', [0xFF, 0x03, IK_CMD_LED, num, state]))

def IK_set_tone(frequency, duration, volume):
    uart.write(array.array('B', [0xFF, 0x03, IK_CMD_TONE, frequency, duration, volume]))

def IK_get_onoff():
    uart.write(array.array('B', [0xFF, 0x01, IK_CMD_ONOFFSWITCH]))

def IK_get_correct():
    uart.write(array.array('B', [0xFF, 0x01, IK_CMD_CORRECT]))

def IK_reset():
    uart.write(array.array('B', [0xFF, 0x01, IK_CMD_RESET_DEVICE]))

def IK_get_all_sensors():
    uart.write(array.array('B', [0xFF, 0x01, IK_CMD_ALL_SENSORS]))

def IK_get_sn():
    uart.write(array.array('B', [0xFF, 0x01, IK_CMD_GET_SN]))

# All LEDs on
for i in range(0,15):
    IK_set_led(i, 1)

IK_set_tone(0,0,0)
IK_get_onoff()
IK_get_all_sensors();
IK_get_sn()
IK_get_version()
# TODO not sure why sleep is needed
time.sleep(0.1)
IK_get_correct()

time.sleep(1)
# All LEDs off
for i in range(0,15):
    IK_set_led(i, 0)

while True:
    data = uart.read(64)    # read up to 64 bytes
    #print(data)  # this is a bytearray type

    # An IK binary event starts with 0xFF.
    # The next byte is the event length
    # The next byte is the event type
    # The 0 or more following bytes are based on the event type
    if data is not None:
        for i in range(0,len(data)):
            # Show the byte as 2 hex digits
            #print("%02x " % (data[i]), end='')
            if eventState == 0:
                if data[i] == 0xFF:
                    eventState = 1
            elif eventState == 1:
                eventLen = data[i]
                eventState = 2
                eventIndex = 0
            elif eventState == 2:
                eventBuf[eventIndex] = data[i]
                eventIndex += 1
                if eventIndex >= eventLen:
                    eventState = 0
                    eventDecode(eventBuf, eventLen)

