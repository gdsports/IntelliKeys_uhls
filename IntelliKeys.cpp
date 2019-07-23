/*
   MIT License

   Copyright (c) 2018-2019 gdsports625@gmail.com

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
#include "IntelliKeys.h"

const uint8_t IntelliKeys::epDataInIndex = 1;
const uint8_t IntelliKeys::epDataOutIndex = 2;

IntelliKeys::IntelliKeys(USBHost *p) :
    pUsb(p),
    bAddress(0),
    bIface(0),
    bNumEP(1),
    bPollEnable(false),
    qNextPollTime(0),
    ready(false),
    IK_state(0)
{
    for(uint8_t i = 0; i < IK_MAX_ENDPOINTS; i++) {
        epInfo[i].epAddr = 0;
        epInfo[i].maxPktSize = (i) ? 0 : 8;
        epInfo[i].bmAttribs  = 0;
        epInfo[i].bmSndToggle = 0;
        epInfo[i].bmRcvToggle = 0;
        epInfo[i].bmNakPower = (i == epDataInIndex) ? USB_NAK_NOWAIT : USB_NAK_NONAK;

    }
    if(pUsb)
        pUsb->RegisterDeviceClass(this);
}

uint32_t IntelliKeys::Init(uint32_t parent, uint32_t port, uint32_t lowspeed) {

    const uint8_t constBufSize = sizeof (USB_DEVICE_DESCRIPTOR);

    uint8_t buf[constBufSize];
    USB_DEVICE_DESCRIPTOR * udd = reinterpret_cast<USB_DEVICE_DESCRIPTOR*>(buf);

    uint8_t rcode;
    UsbDeviceDefinition *p = NULL;
    EpInfo *oldep_ptr = NULL;
    uint8_t num_of_conf; // number of configurations

    AddressPool &addrPool = pUsb->GetAddressPool();

    USBTRACE("IntelliKeys Init\r\n");

    if(bAddress)
        return USB_ERROR_CLASS_INSTANCE_ALREADY_IN_USE;

    // Get pointer to pseudo device with address 0 assigned
    p = addrPool.GetUsbDevicePtr(0);

    if(!p)
        return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;

    if(!p->epinfo) {
        USBTRACE("epinfo\r\n");
        return USB_ERROR_EPINFO_IS_NULL;
    }

    // Save old pointer to EP_RECORD of address 0
    oldep_ptr = p->epinfo;

    // Temporary assign new pointer to epInfo to p->epinfo in order to avoid toggle inconsistence
    p->epinfo = epInfo;

    p->lowspeed = lowspeed;

    // Get device descriptor
    rcode = pUsb->getDevDescr(0, 0, constBufSize, (uint8_t*)buf);

    // Restore p->epinfo
    p->epinfo = oldep_ptr;

    if(rcode)
        goto FailGetDevDescr;

#define IK_VID          0x095e
#define IK_PID_FWLOAD   0x0100  // Firmware load required
#define IK_PID_RUNNING  0x0101  // Firmware running

    if (udd->idVendor != IK_VID) goto FailUnknownDevice;
    if (udd->idProduct == IK_PID_FWLOAD) {
        USBTRACE("found IK, need FW load\r\n");
        if (IK_state == 0) IK_state = 2;
    }
    else if (udd->idProduct == IK_PID_RUNNING) {
        USBTRACE("found IK, FW running\r\n");
    }
    else
        goto FailUnknownDevice;

    // Allocate new address according to device class
    bAddress = addrPool.AllocAddress(parent, false, port);

    if(!bAddress)
        return USB_ERROR_OUT_OF_ADDRESS_SPACE_IN_POOL;

    // Extract Max Packet Size from the device descriptor
    epInfo[0].maxPktSize = udd->bMaxPacketSize0;

    // Assign new address to the device
    rcode = pUsb->setAddr(0, 0, bAddress);

    if(rcode) {
        p->lowspeed = false;
        addrPool.FreeAddress(bAddress);
        bAddress = 0;
        USBTRACE2("setAddr:", rcode);
        return rcode;
    }

    USBTRACE2("Addr:", bAddress);

    p->lowspeed = false;

    p = addrPool.GetUsbDevicePtr(bAddress);

    if(!p)
        return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;

    p->lowspeed = lowspeed;

    num_of_conf = udd->bNumConfigurations;

    // Assign epInfo to epinfo pointer
    rcode = pUsb->setEpInfoEntry(bAddress, 1, epInfo);

    if(rcode)
        goto FailSetDevTblEntry;

    USBTRACE2("NC:", num_of_conf);

    if (udd->idProduct == IK_PID_RUNNING) {
        for(uint8_t i = 0; i < num_of_conf; i++) {
            ConfigDescParser< USB_CLASS_HID,
                0,
                0,
                CP_MASK_COMPARE_ALL > IK_HID(this);

            rcode = pUsb->getConfDescr(bAddress, 0, i, &IK_HID);

            if(rcode)
                goto FailGetConfDescr;

            if(bNumEP > 1) {
                bPollEnable = true;
                IK_state = 1;
                break;
            }
        }
    }
    else {
        bConfNum = 1;
    }

    USBTRACE2("bNumEP:", bNumEP);
    // Assign epInfo to epinfo pointer
    rcode = pUsb->setEpInfoEntry(bAddress, bNumEP, epInfo);

    USBTRACE2("Conf:", bConfNum);

    // Set Configuration Value
    rcode = pUsb->setConf(bAddress, 0, bConfNum);

    if(rcode)
        goto FailSetConfDescr;

    USBTRACE("IntelliKeys configured\r\n");

    ready = true;

    return 0;

FailGetDevDescr:
#ifdef DEBUG_USB_HOST
    NotifyFailGetDevDescr();
    goto Fail;
#endif

FailSetDevTblEntry:
#ifdef DEBUG_USB_HOST
    NotifyFailSetDevTblEntry();
    goto Fail;
#endif

FailGetConfDescr:
#ifdef DEBUG_USB_HOST
    NotifyFailGetConfDescr();
    goto Fail;
#endif

FailSetConfDescr:
#ifdef DEBUG_USB_HOST
    NotifyFailSetConfDescr();
    goto Fail;
#endif

FailUnknownDevice:
#ifdef DEBUG_USB_HOST
    NotifyFailUnknownDevice(udd->idVendor, udd->idProduct);
#endif
    rcode = USB_DEV_CONFIG_ERROR_DEVICE_NOT_SUPPORTED;

#ifdef DEBUG_USB_HOST
Fail:
    NotifyFail(rcode);
#endif
    Release();
    return rcode;
}

void IntelliKeys::EndpointXtract(uint32_t conf, uint32_t iface, uint32_t alt __attribute__((unused)), uint32_t proto __attribute__((unused)), const USB_ENDPOINT_DESCRIPTOR *pep) {
    bConfNum = conf;
    bIface = iface;

    uint8_t index;

    if((pep->bmAttributes & bmUSB_TRANSFER_TYPE) == USB_TRANSFER_TYPE_INTERRUPT)
        index = ((pep->bEndpointAddress & 0x80) == 0x80) ? epDataInIndex : epDataOutIndex;
    else
        return;

    // Fill in the endpoint info structure
    epInfo[index].epAddr = (pep->bEndpointAddress & 0x0F);
    epInfo[index].maxPktSize = (uint8_t)pep->wMaxPacketSize;
    epInfo[index].bmAttribs  = pep->bmAttributes;
    epInfo[index].bmSndToggle = 0;
    epInfo[index].bmRcvToggle = 0;

    bNumEP++;

    PrintEndpointDescriptor(pep);
}

uint32_t IntelliKeys::Release() {
    ready = false;
    pUsb->GetAddressPool().FreeAddress(bAddress);

    bIface = 0;
    bNumEP = 1;

    bAddress = 0;
    qNextPollTime = 0;
    return 0;
}

void IntelliKeys::ezusb_8051Reset(uint8_t resetBit)
{
    static uint8_t reg_value;
    reg_value = resetBit;
    uint32_t rv = pUsb->ctrlReq(bAddress, 0, 0x40, ANCHOR_LOAD_INTERNAL,
            (uint8_t)CPUCS_REG, (uint8_t)(CPUCS_REG>>8), 0, 1, 1, &reg_value, NULL);
    if(rv && rv != USB_ERRORFLOW) {
        Release();
    }
}

static PINTEL_HEX_RECORD pHex;
static uint8_t pHexBuf[MAX_INTEL_HEX_RECORD_LENGTH];

int IntelliKeys::ezusb_DownloadIntelHex(bool internal)
{
    while (pHex->Type == 0) {
        if (INTERNAL_RAM(pHex->Address) == internal) {
            if (pHex->Length > sizeof(pHexBuf)) USBTRACE2("pHex->Length ", pHex->Length);
            memcpy(pHexBuf, pHex->Data, pHex->Length);
            uint32_t rv = pUsb->ctrlReq(bAddress, 0, 0x40,
                    (internal)?ANCHOR_LOAD_INTERNAL:ANCHOR_LOAD_EXTERNAL,
                    (uint8_t)pHex->Address, (uint8_t)(pHex->Address>>8),
                    0, pHex->Length, pHex->Length, pHexBuf, NULL);
            if(rv && rv != USB_ERRORFLOW) {
                Release();
                return 1;
            }
            pHex++;
            return 0;
        }
        pHex++;
    }
    return 1;
}

void IntelliKeys::IK_firmware_load()
{
    uint32_t rv;

    IK_state = 1;
    USBTRACE("set interface(0,0)\r\n");
    rv = pUsb->ctrlReq(bAddress, 0, 1, 11, 0, 0, 0, 0, 0, NULL, NULL);
    if(rv && rv != USB_ERRORFLOW) {
        Release();
        return;
    }

    ezusb_8051Reset(1);
    pHex = (PINTEL_HEX_RECORD)loader;
    // Download external records first
    while (ezusb_DownloadIntelHex(false) == 0) delay(1);
    pHex = (PINTEL_HEX_RECORD)loader;
    while (ezusb_DownloadIntelHex(true) == 0) delay(1);

    ezusb_8051Reset(0);
    pHex = (PINTEL_HEX_RECORD)firmware;
    while (ezusb_DownloadIntelHex(false) == 0) delay(1);

    ezusb_8051Reset(1);
    pHex = (PINTEL_HEX_RECORD)firmware;
    while (ezusb_DownloadIntelHex(true) == 0) delay(1);
    ezusb_8051Reset(0);
}

void IntelliKeys::sensorUpdate(int sensor, int value)
{
    int midpoint = 150;

    if (eeprom_all_valid) {
        midpoint = (50*eeprom_data.sensorBlack[sensor] +
                50*eeprom_data.sensorWhite[sensor]) / 100;
    }
    int sensorOn = (value > midpoint);
    if (sensorStatus[sensor] != sensorOn) {
        if (sensor_callback) (*sensor_callback)(sensor, sensorOn);
        sensorStatus[sensor] = sensorOn;
    }
}

void IntelliKeys::handleEvents(const uint8_t *rxpacket, size_t len)
{
    if ((rxpacket == NULL) || (len == 0)) return;
    if (raw_event_callback) (*raw_event_callback)(rxpacket, len);
    switch (*rxpacket) {
        case IK_EVENT_ACK:
            //USBTRACE("IK_EVENT_ACK\r\n");
            break;
        case IK_EVENT_MEMBRANE_PRESS:
            if(membrane_press_callback) (*membrane_press_callback)(rxpacket[1], rxpacket[2]);
            break;
        case IK_EVENT_MEMBRANE_RELEASE:
            if (membrane_release_callback) (*membrane_release_callback)(rxpacket[1], rxpacket[2]);
            break;
        case IK_EVENT_SWITCH:
            if (switch_callback) (*switch_callback)(rxpacket[1], rxpacket[2]);
            break;
        case IK_EVENT_SENSOR_CHANGE:
            sensorUpdate(rxpacket[1], rxpacket[2]);
            break;
        case IK_EVENT_VERSION:
            if (!version_done && version_callback) (*version_callback)(rxpacket[1], rxpacket[2]);
            version_done = true;
            break;
        case IK_EVENT_EEPROM_READ:
            USBTRACE("IK_EVENT_EEPROM_READ\r\n");
            break;
        case IK_EVENT_ONOFFSWITCH:
            if (on_off_callback) {
                if (rxpacket[1]) {
                    get_correct();
                    get_all_sensors();
                }
                (*on_off_callback)(rxpacket[1]);
            }
            break;
        case IK_EVENT_NOMOREEVENTS:
            USBTRACE("IK_EVENT_NOMOREEVENTS\r\n");
            break;
        case IK_EVENT_MEMBRANE_REPEAT:
            USBTRACE("IK_EVENT_MEMBRANE_REPEAT\r\n");
            break;
        case IK_EVENT_SWITCH_REPEAT:
            USBTRACE("IK_EVENT_SWITCH_REPEAT\r\n");
            break;
        case IK_EVENT_CORRECT_MEMBRANE:
            if (correct_membrane_callback) (*correct_membrane_callback)(rxpacket[1], rxpacket[2]);
            break;
        case IK_EVENT_CORRECT_SWITCH:
            if (correct_switch_callback) (*correct_switch_callback)(rxpacket[1], rxpacket[2]);
            break;
        case IK_EVENT_CORRECT_DONE:
            USBTRACE("IK_EVENT_CORRECT_DONE\r\n");
            if (correct_done_callback) (*correct_done_callback)();
            break;
        case IK_EVENT_EEPROM_READBYTE:
            {
                if (rxpacket[2] >= 0x80) {
                    uint8_t idx = rxpacket[2] - 0x80;
                    uint8_t *p = (uint8_t *)&eeprom_data;
                    p[idx] = rxpacket[1];
                    eeprom_valid[idx] = true;
                }
                else {
                    USBTRACE("IK_EVENT_EEPROM_READBYTE idx bad");
                }
            }
            break;
        case IK_EVENT_DEVICEREADY:
            USBTRACE("IK_EVENT_DEVICEREADY\r\n");
            break;
        case IK_EVENT_AUTOPILOT_STATE:
            USBTRACE("IK_EVENT_AUTOPILOT_STATE\r\n");
            break;
        case IK_EVENT_DELAY:
            USBTRACE("IK_EVENT_DELAY\r\n");
            break;
        case IK_EVENT_ALL_SENSORS:
            USBTRACE("IK_EVENT_ALL_SENSORS\r\n");
            break;
        default:
            USBTRACE2("Unknown event code=", *rxpacket);
            break;
    }
}

uint32_t IntelliKeys::IK_poll()
{
    uint8_t rxpacket[64];

    uint16_t pktSize = epInfo[epDataInIndex].maxPktSize;
    uint32_t rv = pUsb->inTransfer((uint32_t)bAddress, epInfo[epDataInIndex].epAddr, &pktSize, rxpacket);
    if(rv && rv != USB_ERRORFLOW) {
        Release();
        return rv;
    }
    handleEvents(rxpacket, pktSize);
    return rv;
}

void IntelliKeys::Task()
{
    if (IK_state == 2) IK_firmware_load();
    if (IK_state == 1) {
        IK_state = 4;
        start();
    }
    if(!bPollEnable) return;

    IK_poll();

    if (!eeprom_all_valid) get_eeprom();
}

inline int IntelliKeys::PostCommand(uint8_t *command)
{
    uint32_t rv;
    rv = pUsb->outTransfer(bAddress, epInfo[epDataOutIndex].epAddr, IK_REPORT_LEN, command);
    if (rv) {
        Serial.print("IntelliKeys::PostCommand|outTransfer ");
        Serial.println(rv);
    }
    if(rv && rv != USB_ERRORFLOW) {
        Release();
    }
    IK_poll();
    return rv;
}

/*
 * parameter number must be set to one of these.
 * enum IK_LEDS {
 *  IK_LED_SHIFT=1,
 *  IK_LED_CAPS_LOCK=4,
 *  IK_LED_MOUSE=7,
 *  IK_LED_ALT=2,
 *  IK_LED_CTRL_CMD=5,
 *  IK_LED_NUM_LOCK=8
 * }
 *
 * parameter value is 1 for ON, 0 for OFF
 */
int IntelliKeys::setLED(uint8_t number, uint8_t value)
{
    uint8_t command[IK_REPORT_LEN] = {IK_CMD_LED,number,value,0,0,0,0,0};
    return PostCommand(command);
}

/*
 * parameter freq = note frequency 0..255 ?
 * parameter duration = note duration in 10 ms increments
 * parameter volume = 0..255
 */
int IntelliKeys::sound(int freq, int duration, int volume)
{
    uint8_t report[IK_REPORT_LEN] = {IK_CMD_TONE,0,0,0,0,0,0,0};

    //  set parameters and blow
    report[1] = freq;
    report[2] = volume;
    report[3] = duration/10;
    return PostCommand(report);
}


int IntelliKeys::get_version(void) {
    uint8_t command[IK_REPORT_LEN] = {IK_CMD_GET_VERSION,0,0,0,0,0,0,0};
    version_done = false;
    return PostCommand(command);
}

int IntelliKeys::get_all_sensors(void) {
    uint8_t command[IK_REPORT_LEN] = {IK_CMD_ALL_SENSORS,0,0,0,0,0,0,0};
    memset(sensorStatus, 255, sizeof(sensorStatus));
    return PostCommand(command);
}

int IntelliKeys::get_onoff(void) {
    uint8_t command[IK_REPORT_LEN] = {IK_CMD_ONOFFSWITCH,0,0,0,0,0,0,0};
    return PostCommand(command);
}

int IntelliKeys::reset(void) {
    uint8_t command[IK_REPORT_LEN] = {IK_CMD_RESET_DEVICE,0,0,0,0,0,0,0};
    return PostCommand(command);
}

int IntelliKeys::get_correct(void) {
    USBTRACE("get_correct\r\n");
    uint8_t command[IK_REPORT_LEN] = {IK_CMD_CORRECT,0,0,0,0,0,0,0};
    return PostCommand(command);
}

void IntelliKeys::get_eeprom(void)
{
    uint8_t report[IK_REPORT_LEN] = {IK_CMD_EEPROM_READBYTE,0,0x1F,0,0,0,0,0};
    uint8_t pending = 0;

    USBTRACE("get_eeprom\r\n");
    for (uint8_t i=0; i < sizeof(eeprom_t); i++) {
        if (!eeprom_valid[i]) {
            report[1] = 0x80 + i;
            PostCommand(report);
            if (pending++ > 8) break;
        }
    }
    if (pending == 0) {
        eeprom_all_valid = true;
        // Get sensor status events because eeprom_data.sensorBlack and White
        // now have valid data.
        get_all_sensors();

        if (on_SN_callback) (*on_SN_callback)(eeprom_data.serialnumber);
    }
}

void IntelliKeys::clear_eeprom()
{
    eeprom_all_valid = false;
    memset(eeprom_valid, 0, sizeof(eeprom_valid));
}

void IntelliKeys::start()
{
    uint8_t command[IK_REPORT_LEN] = {0};

    USBTRACE("start\r\n");
    command[0] = IK_CMD_INIT;
    command[1] = 0;  //  interrupt event mode
    PostCommand(command);

    command[0] = IK_CMD_SCAN;
    command[1] = 1; //  enable
    PostCommand(command);

    //delay(250);

    get_all_sensors();

    get_version();

    clear_eeprom();

    if (connect_callback) (*connect_callback)();
}

void IntelliKeys::PrintEndpointDescriptor(const USB_ENDPOINT_DESCRIPTOR* ep_ptr) {
    Notify(PSTR("Endpoint descriptor:"), 0x80);
    Notify(PSTR("\r\nLength:\t\t"), 0x80);
    D_PrintHex<uint8_t > (ep_ptr->bLength, 0x80);
    Notify(PSTR("\r\nType:\t\t"), 0x80);
    D_PrintHex<uint8_t > (ep_ptr->bDescriptorType, 0x80);
    Notify(PSTR("\r\nAddress:\t"), 0x80);
    D_PrintHex<uint8_t > (ep_ptr->bEndpointAddress, 0x80);
    Notify(PSTR("\r\nAttributes:\t"), 0x80);
    D_PrintHex<uint8_t > (ep_ptr->bmAttributes, 0x80);
    Notify(PSTR("\r\nMaxPktSize:\t"), 0x80);
    D_PrintHex<uint16_t > (ep_ptr->wMaxPacketSize, 0x80);
    Notify(PSTR("\r\nPoll Intrv:\t"), 0x80);
    D_PrintHex<uint8_t > (ep_ptr->bInterval, 0x80);
    Notify(PSTR("\r\n"), 0x80);
}
