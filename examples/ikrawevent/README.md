# IntelliKeys binary events and commands

The ikrawevent.ino sketch is the bridge between the IK USB host driver API
defined in IntelliKeys.h and the UART. It currently uses binary structures for
events and commands but could be modified to implement a different protocol
over the UART.

## Events

All are generated automatically except Firmware Version and Serial
Number. See the next section for commands. All appear one per line.

All events are a variable number unsigned 8 bit integers. The receiver should
scan the UART input stream for 0xFF. The first byte of an event is always equal
to 0xFF. The second byte holds the number of bytes in the event starting with
the next byte. If the receiver does not understand how to handle the event, it
can use the length to skip over the remaining bytes so it can easily find the
start of the next event.

The third byte is the event type. See the IK_EVENT symbols. There may be more
bytes for parameters.

If 0xFF appears in the payload, this can cause problems. Hopefully, this will
resync on the next 0xFF. This scheme ssumes dropped and corrupted bytes are
rare.

Solving this problem in general means adding flag byte escaping, ACK/NAK, retry
limits, inter-byte and inter-packet timeouts, etc. XMODEM or HDLC type of
protocol. Probably not a good candidate for CP.

```
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

### Membrane Press
    {0xFF, 0x03, IK_EVENT_MEMBRANE_PRESS, x, y}
    where x=0..23 and y=0..23

### Membrane Release
    {0xFF, 0x03, IK_EVENT_MEMBRANE_RELEASE, x, y}
    where x=0..23 and y=0..23

### AT switch inputs
    {0xFF, 0x03, IK_EVENT_SWITCH, num, state}
    where num = 1,2 and state=0,1

### Overlay sensors
    {0xFF, 0x03, IK_EVENT_SENSOR_CHANGE, num, state}
    where num = 0,1,2 and state=0,1

### Firmware Version
    {0xFF, 0x03, IK_EVENT_VERSION, major, minor}
    where major=0..255 and minor=0..255

### Correct Membrane
    {0xFF, 0x03, IK_EVENT_CORRECT_MEMBRANE, x, y}
    where x=0..23, y=0..23

### Correct Switch
    {0xFF, 0x03, IK_EVENT_CORRECT_SWITCH, num, state}
    where num = 1,2 and state=0,1

### Correct Done
    {0xFF, 0x01, IK_EVENT_CORRECT_DONE}
    Final correction event. No more CORRECTION events.

### On/Off Switch
    {0xFF, 0x02, IK_EVENT_ONOFFSWITCH, state}
    where state=0,1

### Serial Number
    {0xFF, 0x1E, IK_EVENT_SERNUM, sernum[29]}
    where sernum is 29 bytes long

### Connect
    {0xFF, 0x01, IK_EVENT_CONNECT}

### Disconnect
    {0xFF, 0x01, IK_EVENT_DISCONNECT}

## Commands

All commands are a variable number unsigned 8 bit integers. The receiver should
scan the UART input stream for 0xFF. The first byte of a command is always
equal to 0xFF. The second byte holds the number of bytes in the command
starting with the next byte. If the receiver does not understand how to handle
the command, it can use the length to skip over the remaining bytes so it can
easily find the start of the next command.

The third byte is the command type. See the IK_CMD symbols. There may be more
bytes for parameters.

```
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
```
### Get Version
    {0xFF, 0x01, IK_CMD_GET_VERSION}
    Send this command to trigger the IK_EVENT_VERSION event.

### Get All Sensors
    {0xFF, 0x01, IK_CMD_GET_ALL_SENSORS}
    Send this command to trigger all overlay sensor events.

### Get On/Off Switch
    {0xFF, 0x01, IK_CMD_ONOFFSWITCH}
    Send this command to trigger the onoff event and the sensor events.

### Get Serial Number
    {0xFF, 0x01, IK_CMD_GET_SN}
    Send this command to trigger the serial number event.

### Get Correct
    {0xFF, 0x01, IK_CMD_CORRECT}
    Send this command to trigger correct done, correct membrane,
    and correct switch events. This allows the sender to capture the
    current state of the membrane and AT switches.

### Reset
    {0xFF, 0x01, IK_CMD_RESET_DEVICE}
    Reset/reboot IntelliKeys board.

### Set LED
    {0xFF, 0x03, IK_CMD_LED, n, state}

    n = 1 SHIFT LED
        2 ALT LED
        3 ?
        4 CAPS Lock LED
        5 CTRL/CMD LED
        6 ?
        7 Mouse LED
        8 NUM Lock LED
    state is 1 for ON, 0 for OFF

### Set Sound
    {0xFF, 0x03, IK_CMD_TONE, frequency, duration, volume}

    frequency = 0..255, duration=0..255, volume=0..255

    I think n is really 0..95 to select a note from 8 octaves with 12 notes
    per octave. But I have not experimented with this feature.
