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

#include "SPI.h"
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"
#include <Adafruit_TinyUSB.h>

#if defined(__SAMD51__) || defined(NRF52840_XXAA)
Adafruit_FlashTransport_QSPI flashTransport(PIN_QSPI_SCK, PIN_QSPI_CS, PIN_QSPI_IO0, PIN_QSPI_IO1, PIN_QSPI_IO2, PIN_QSPI_IO3);
#else
#if (SPI_INTERFACES_COUNT == 1)
Adafruit_FlashTransport_SPI flashTransport(SS, &SPI);
#else
Adafruit_FlashTransport_SPI flashTransport(SS1, &SPI1);
#endif
#endif

Adafruit_SPIFlash flash(&flashTransport);

// file system object from SdFat
FatFileSystem fatfs;

FatFile root;
FatFile file;

// USB Mass Storage object
Adafruit_USBD_MSC usb_msc;

// Set to true when PC write to flash
bool Flash_changed;

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

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
    // Note: SPIFLash Bock API: readBlocks/writeBlocks/syncBlocks
    // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
    return flash.readBlocks(lba, (uint8_t*) buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
    digitalWrite(LED_BUILTIN, HIGH);

    // Note: SPIFLash Bock API: readBlocks/writeBlocks/syncBlocks
    // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
    return flash.writeBlocks(lba, buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_cb (void)
{
    // sync with flash
    flash.syncBlocks();

    // clear file system's cache to force refresh
    fatfs.cacheClear();

    Flash_changed = true;

    digitalWrite(LED_BUILTIN, LOW);
}

// the setup function runs once when you press reset or power the board
void tinyusb_setup()
{
    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));

    usb_hid.begin();

    flash.begin();

    // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
    usb_msc.setID("Adafruit", "External Flash", "1.0");

    // Set callback
    usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

    // Set disk size, block size should be 512 regardless of spi flash page size
    usb_msc.setCapacity(flash.pageSize()*flash.numPages()/512, 512);

    // MSC is ready for read/write
    usb_msc.setUnitReady(true);

    usb_msc.begin();

    // Init file system on the flash
    fatfs.begin(&flash);

    Flash_changed = true; // to print contents initially

    // wait until device mounted
    while( !USBDevice.mounted() ) delay(1);

    DBSerial.println("Adafruit TinyUSB Mass Storage External Flash example");
    DBSerial.print("JEDEC ID: "); DBSerial.println(flash.getJEDECID(), HEX);
    DBSerial.print("Flash size: "); DBSerial.println(flash.size());
}

void tinyusb_loop()
{
    if ( Flash_changed )
    {
        Flash_changed = false;

        if ( !root.open("/") )
        {
            DBSerial.println("open root failed");
            return;
        }

        DBSerial.println("Flash contents:");

        // Open next file in root.
        // Warning, openNext starts at the current directory position
        // so a rewind of the directory may be required.
        while ( file.openNext(&root, O_RDONLY) )
        {
            file.printFileSize(&DBSerial);
            DBSerial.write(' ');
            file.printName(&DBSerial);
            if ( file.isDir() )
            {
                // Indicate a directory.
                DBSerial.write('/');
            }
            DBSerial.println();
            file.close();
        }

        root.close();

        DBSerial.println();
    }
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
