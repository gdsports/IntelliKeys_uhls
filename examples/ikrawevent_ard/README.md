# ikrawevent_ard

Map IK to QWERTY USB keyboard. The ordinary keys works as usual. The mouse
keys do not work yet.

The modifier keys (shift, alt, ctrl, gui) work in locking mode. Locking
modifier keys means the keyboard can be used in one finger mode.

Pressing and releasing the SHIFT key then pressing and releasing the Q key
sends an upper case 'Q' to the computer. Pressing and release the 'W' key sends
a lower case 'w'. The SHIFT works for the next key then it turns off.

To lock the SHIFT on for more than key, pressing and releasing the SHIFT key
twice locks the SHIFT on. All following keys are sent in upper case. Pressing
and releasing the SHIFT key turns off the SHIFT lock feature.

The locking feature works for all modifier keys.

```
IK - USB OTG host - TM0-A ---- 2XUARTs - TM0-B          - Computer
                    ikrawevent           ikrawevent_ard
```
* IK = IntelliKeys board
* USB OTG host = USB OTG to host cable
* TM0-A = Trinket M0 running ikrawevent.
* 2XUARTs = Trinket M0 UARTs cross connected
* TM0-B = Trinket M0 running ikrawevent_ard
* Computer = Computer that supports USB keyboard and mouse

ikrawevent bridges between the IK USB and its UART. ikrawevent_ard bridges
between its UART and USB device keyboard and mouse. TBD USB device
mass storage.

ikrawevent_ard is written in Arduino C/C++. It currently uses the following
libraries

* Adafruit tinyUSB
* Adafruit DotStar
* Adafruit SPIFlash
* Adafruit SdFat

The DotStar library is currently only used to turn off the TM0 RGB LED.

TinyUSB keyboard, mouse, serial, and mass storage work at the same time on
an M4 Express board. Any M0 or M4 board with SPI or QSPI Flash should work.

Looks like SPI Flash will be required unless there is a way to use part of the
SAMD21 Flash for MSC. This is not supported in TinyUSB but might be possible.
