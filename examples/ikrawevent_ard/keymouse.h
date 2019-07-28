// USB HID keyboard and mouse
// TinyUSB works very differently from the Arduino Keyboard and Mouse
// libraries.

#ifndef __KEYMOUSE_H__
#define __KEYMOUSE_H__

#if DEBUG_SERIAL
#define DBSerial    Serial
#else
#define DBSerial    if(0)Serial
#endif
#define IKSerial    Serial1

#include <Adafruit_TinyUSB.h>
// Report ID
enum
{
    RID_KEYBOARD = 1,
    RID_MOUSE
};

// HID report descriptor using TinyUSB's template
uint8_t const desc_hid_report[] =
{
    TUD_HID_REPORT_DESC_KEYBOARD( HID_REPORT_ID(RID_KEYBOARD), ),
    TUD_HID_REPORT_DESC_MOUSE   ( HID_REPORT_ID(RID_MOUSE), )
};

// USB HID object
Adafruit_USBD_HID usb_hid;


// the setup function runs once when you press reset or power the board
void tinyusb_setup()
{
    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));

    usb_hid.begin();

    // wait until device mounted
    while( !USBDevice.mounted() ) delay(1);
}

void tinyusb_loop()
{
}

void tinyusb_mouse_move(int8_t x, int8_t y)
{
    if (usb_hid.ready()) {
        int8_t const delta = 5;
        usb_hid.mouseMove(RID_MOUSE, delta, delta); // right + down

        // delay a bit before attempt to send keyboard report
        delay(1);
    }
}

uint8_t KeysDown[6];
uint8_t KeyModifiers;

inline bool isModifierKey(uint8_t hid_keycode)
{
    return ((hid_keycode >= HID_KEY_CONTROL_LEFT) &&
        (hid_keycode <= HID_KEY_GUI_RIGHT));
}

inline uint8_t maskModifierKey(uint8_t hid_keycode)
{
    return (1<<(hid_keycode - HID_KEY_CONTROL_LEFT));
}

void tinyusb_key_press(uint8_t hid_keycode)
{
    // Remote wakeup
    if ( USBDevice.suspended() )
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        USBDevice.remoteWakeup();
    }
    if (isModifierKey(hid_keycode)) {
        KeyModifiers |= maskModifierKey(hid_keycode);
        DBSerial.printf("KeyModifers=%X\n", KeyModifiers);
    }
    if ( usb_hid.ready() ) {
        DBSerial.printf("KeysDown[0]=%X\n", KeysDown[0]);
        // Add keycode to KeysDown
        for (int i = 0; i < sizeof(KeysDown); i++) {
            if ((KeysDown[i] == 0) || (KeysDown[i] == hid_keycode)) {
                KeysDown[i] = hid_keycode;
                break;
            }
        }
        usb_hid.keyboardReport(RID_KEYBOARD, KeyModifiers, KeysDown);
    }
}

void tinyusb_key_release(uint8_t hid_keycode)
{
    // Remote wakeup
    if ( USBDevice.suspended() )
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        USBDevice.remoteWakeup();
    }
    if (isModifierKey(hid_keycode)) {
        KeyModifiers &= ~(maskModifierKey(hid_keycode));
    }
    if ( usb_hid.ready() )
    {
        // Remove keycode from KeysDown but also pack all non-zero
        // values to left side.
        int dest = 0;
        for (int i = 0; i < sizeof(KeysDown); i++) {
            if (KeysDown[i] == hid_keycode) {
                KeysDown[i] = 0;
            }
            else if (KeysDown[i] != 0) {
                KeysDown[dest++] = KeysDown[i];
            }
        }
        usb_hid.keyboardReport(RID_KEYBOARD, KeyModifiers, KeysDown);
    }
}

void tinyusb_key_releaseAll()
{
    // Remote wakeup
    if ( USBDevice.suspended() )
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        USBDevice.remoteWakeup();
    }
    KeyModifiers = 0;
    memset(KeysDown, 0, sizeof(KeysDown));
    if ( usb_hid.ready() )
    {
        usb_hid.keyboardReport(RID_KEYBOARD, KeyModifiers, KeysDown);
    }
}

#endif /* __KEYMOUSE_H__ */
