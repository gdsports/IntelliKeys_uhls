/* USB Intellikeys driver
 * Copyright 2018-2019 gdsports625@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Usb.h>
#include "intellikeysdefs.h"

#define IK_EEPROM_SN_SIZE   (29)
#define IK_MAX_ENDPOINTS    (3)

class IntelliKeys: public USBDeviceConfig, public UsbConfigXtracter {
    public:
        static const uint8_t epDataInIndex; // DataIn endpoint index
        static const uint8_t epDataOutIndex; // DataOUT endpoint index
        EpInfo epInfo[IK_MAX_ENDPOINTS];

    protected:
        USBHost *pUsb;
        uint8_t bAddress;
        uint8_t bConfNum; // configuration number
        uint8_t bIface; // interface value
        uint8_t bNumEP; // total number of EP in the configuration
        bool bPollEnable;
        uint32_t qNextPollTime; // next poll time
        volatile bool ready; //device ready indicator
        void PrintEndpointDescriptor(const USB_ENDPOINT_DESCRIPTOR* ep_ptr);

    public:
        IntelliKeys(USBHost *pusb);

        int setLED(uint8_t number, uint8_t value);
        int sound(int freq, int duration, int volume);
        int get_version(void);
        int get_all_sensors(void);
        int get_correct(void);

        // Event callback functions
        void onMembranePress(void (*function)(int x, int y)) {
            membrane_press_callback = function;
        }
        void onMembraneRelease(void (*function)(int x, int y)) {
            membrane_release_callback = function;
        }
        void onSwitch(void (*function)(int switch_number, int switch_state)) {
            switch_callback = function;
        }
        void onSensor(void (*function)(int sensor_number, int sensor_value)) {
            sensor_callback = function;
        }
        void onVersion(void (*function)(int major, int minor)) {
            version_callback = function;
        }
        void onConnect(void (*function)(void)) {
            connect_callback = function;
        }
        void onDisconnect(void (*function)(void)) {
            disconnect_callback = function;
        }
        void onOnOffSwitch(void (*function)(int switch_status)) {
            on_off_callback = function;
        }
        void onSerialNum(void (*function)(uint8_t serial[IK_EEPROM_SN_SIZE])) {
            on_SN_callback = function;
        }
        void onCorrectMembrane(void (*function)(int x, int y)) {
            correct_membrane_callback = function;
        }
        void onCorrectSwitch(void (*function)(int switch_number, int switch_state)) {
            correct_switch_callback = function;
        }
        void onCorrectDone(void (*function)(void)) {
            correct_done_callback = function;
        }
        /* USBDeviceConfig virtual functions */
        virtual uint32_t Init(uint32_t /* parent */, uint32_t /* port */, uint32_t /* lowspeed */);
        virtual uint32_t ConfigureDevice(uint32_t /* parent */, uint32_t /* port */, uint32_t /* lowspeed */) {
            return 0;
        }
        virtual uint32_t Release();
        virtual uint32_t Poll() {
            return 0;
        }
        virtual uint32_t GetAddress() {
            return 0;
        }
        virtual void ResetHubPort(uint32_t /* port */) {
            return;
        } // Note used for hubs only!
        virtual uint32_t VIDPIDOK(uint32_t /* vid */, uint32_t /* pid */) {
            return false;
        }
        virtual uint32_t DEVCLASSOK(uint32_t /* klass */) {
            return false;
        }
        virtual void Task();

    protected:
        /* UsbConfigXtracter virtual functions */
        void EndpointXtract(uint32_t conf __attribute__((unused)), uint32_t iface __attribute__((unused)), uint32_t alt __attribute__((unused)), uint32_t proto __attribute__((unused)), const USB_ENDPOINT_DESCRIPTOR *ep __attribute__((unused)));

    private:
        volatile uint8_t  IK_state;
        int  ezusb_DownloadIntelHex(bool internal);
        void ezusb_8051Reset(uint8_t resetBit);
        void IK_firmware_load();
        void (*membrane_press_callback)(int x, int y);
        void (*membrane_release_callback)(int x, int y);
        void (*switch_callback)(int switch_number, int switch_state);
        void (*sensor_callback)(int sensor_number, int sensor_value);
        void (*version_callback)(int major, int minor);
        void (*connect_callback)(void);
        void (*disconnect_callback)(void);
        void (*on_off_callback)(int switch_status);
        void (*on_SN_callback)(uint8_t serial[IK_EEPROM_SN_SIZE]);
        void (*correct_membrane_callback)(int x, int y);
        void (*correct_switch_callback)(int switch_number, int switch_state);
        void (*correct_done_callback)(void);
        uint32_t IK_poll();
        int PostCommand(uint8_t *command);
        void handleEvents(const uint8_t *rxpacket, size_t len);
        void sensorUpdate(int sensor, int value);
        void start();
        typedef struct
        {
            uint8_t serialnumber[IK_EEPROM_SN_SIZE];
            uint8_t sensorBlack[IK_NUM_SENSORS];
            uint8_t sensorWhite[IK_NUM_SENSORS];
        } eeprom_t;
        void get_eeprom(void);
        void clear_eeprom();
        eeprom_t eeprom_data;
        bool eeprom_valid[sizeof(eeprom_t)];
        bool eeprom_all_valid;
        uint8_t sensorStatus[IK_NUM_SENSORS] = {255, 255, 255};
        //elapsedMillis eeprom_period;
        bool version_done;
};
