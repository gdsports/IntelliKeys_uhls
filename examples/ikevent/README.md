# IntelliKeys JSON events and commands

The ikevent.ino sketch is the bridge between the IK USB host driver API
defined in IntelliKeys.h and the UART. It currently uses JSON for events
and commands but could be modified to implement a different protocol over
the UART.

## JSON Events

All are generated automatically except Firmware Version and Serial
Number. See the next section for commands. All appear one per line.

### Membrane Press
    {"evt":"press","x":n,"y":m}
    where n=0..23, m=0..23

### Membrane Release
    {"evt":"release","x":n,"y":m}
    where n=0..23, m=0..23

### AT switch inputs
    {"evt":"switch","num":n,"st":m}
    where n = 0,1 and m=0,1

### Overlay sensors
    {"evt":"sensor","num":n,"val":m}
    where n = 0,1,2 and m=0,1

### Connect
    {"evt":"connect"}

### Disconnect
    {"evt":"disconnect"}

### On/Off Switch
    {"evt":"onoff","val":n}
    where n=0,1

### Firmware Version
    {"evt":"fwver","major":n,"minor":m}
    n=0..255, m=0..255

### Serial Number
    {"evt":"sernum","sn":"SN"}
    where SN=string

## JSON Commands

Send commands one per line. The line must be terminated with '\n'.

### Get Version
    {"cmd":"getver"}
    Send this JSON command to trigger the fwver event.

### Get Sensors
    {"cmd":"getsnsrs"}
    Send this JSON command to trigger the sensor events.

### Get Serial Number
    {"cmd":"getsn"}
    Send this JSON command to trigger the serial number event.

### Get Correct???
    {"cmd":"getcorr"}
    I am not sure how this works. If it is needed perhaps some input from
    the IK experts would help.

### Set LED
    {"cmd":"setled", "num":n, "val":m}

    n = 1 SHIFT LED
        2 ALT LED
        3 ?
        4 CAPS Lock LED
        5 CTRL/CMD LED
        6 ?
        7 Mouse LED
        8 NUM Lock LED
    m is 1 for ON, 0 for OFF

### Set Sound
    {"cmd":"setsnd", "freq":n, "dura":m, "vol":l}

    n = 0..255, m=0..255, l=0..255

    I think n is really 0..95 to select a note from 8 octaves with 12 notes
    per octave. But I have not experimented with this feature.
