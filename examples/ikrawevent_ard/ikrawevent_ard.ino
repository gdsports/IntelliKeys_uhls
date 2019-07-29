/*
 * Companion program to ikrawevent.
 *
 * ikrawevent runs on a Trinket M0 that plugs into the IK and handles USB host.
 * The events/commands are send/received over a UART interface.
 *
 * ikrawevent_cp runs on a Trinket M0 or any board running CircuitPython.
 * It talks over its UART interface to the Trinket M0 running the above
 * program.
 *
 * This program ikrawevent_ard does the same as ikrawevent_cp but is
 * written in C/C++ Arduino.
 */

#define DEBUG_SERIAL 1

#ifdef ADAFRUIT_TRINKET_M0
// setup Dotstar LED on Trinket M0
#include <Adafruit_DotStar.h>
#define DATAPIN    7
#define CLOCKPIN   8
Adafruit_DotStar strip = Adafruit_DotStar(1, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
#endif

#include "intellikeys.h"
#include "keymouse.h"

/*
 * The native touch resolution is 24x24. For this example, each virtual button
 * is 2 native columns by 3 rows. This array represents the 8 rows of 12
 * virtual buttons. The elements are incremented with each native touch press
 * and decremented with each native touch release. When the count goes from 0
 * to 1, press action (see membrane_actions[]) is performed. When the count
 * goes from 1 to 0, the release action is performed.
 */
uint8_t membrane[8][12];

const uint8_t membrane_actions[8][12] = {
  // Top row = 0
  HID_KEY_ESCAPE,       // [0,0]
  HID_KEY_TAB,          // [1,0]
  HID_KEY_GRAVE,        // [2,0]
  HID_KEY_NUM_LOCK,     // [3,0]
  0,                    // [4,0]
  HID_KEY_INSERT,       // [5,0]
  HID_KEY_HOME,         // [6,0]
  HID_KEY_END,          // [7,0]
  0,                    // [8,0]
  HID_KEY_PAGE_UP,      // [9,0]
  HID_KEY_PAGE_DOWN,    // [10,0]
  HID_KEY_DELETE,       // [11,0]
  // row = 1
  HID_KEY_F1,
  HID_KEY_F2,
  HID_KEY_F3,
  HID_KEY_F4,
  HID_KEY_F5,
  HID_KEY_F6,
  HID_KEY_F7,
  HID_KEY_F8,
  HID_KEY_F9,
  HID_KEY_F10,
  HID_KEY_F11,
  HID_KEY_F12,
  // row = 2
  HID_KEY_1,
  HID_KEY_2,
  HID_KEY_3,
  HID_KEY_4,
  HID_KEY_5,
  HID_KEY_6,
  HID_KEY_7,
  HID_KEY_8,
  HID_KEY_9,
  HID_KEY_0,
  HID_KEY_MINUS,
  HID_KEY_EQUAL,
  // row = 3
  HID_KEY_Q,
  HID_KEY_W,
  HID_KEY_E,
  HID_KEY_R,
  HID_KEY_T,
  HID_KEY_Y,
  HID_KEY_U,
  HID_KEY_I,
  HID_KEY_O,
  HID_KEY_P,
  HID_KEY_BACKSPACE,
  HID_KEY_BACKSPACE,
  // row = 4
  HID_KEY_A,
  HID_KEY_S,
  HID_KEY_D,
  HID_KEY_F,
  HID_KEY_G,
  HID_KEY_H,
  HID_KEY_J,
  HID_KEY_K,
  HID_KEY_L,
  0,
  0,
  0,
  // row = 5
  HID_KEY_Z,
  HID_KEY_X,
  HID_KEY_C,
  HID_KEY_V,
  HID_KEY_B,
  HID_KEY_N,
  HID_KEY_M,
  HID_KEY_SEMICOLON,
  HID_KEY_APOSTROPHE,
  0,
  0,
  0,
  // row = 6
  HID_KEY_CAPS_LOCK,
  HID_KEY_SHIFT_LEFT,
  HID_KEY_SHIFT_LEFT,
  HID_KEY_SPACE,
  HID_KEY_SPACE,
  HID_KEY_SPACE,
  HID_KEY_COMMA,
  HID_KEY_PERIOD,
  HID_KEY_SLASH,
  0,
  0,
  0,
  // row = 7
  HID_KEY_CONTROL_LEFT,
  HID_KEY_ALT_LEFT,
  HID_KEY_GUI_LEFT,
  HID_KEY_ARROW_LEFT,
  HID_KEY_ARROW_RIGHT,
  HID_KEY_ARROW_UP,
  HID_KEY_ARROW_DOWN,
  HID_KEY_RETURN,
  HID_KEY_RETURN,
  0,
  0,
  0,
};

enum mouse_actions {
  MOUSE_MOVE_NW, MOUSE_MOVE_N, MOUSE_MOVE_NE,
  MOUSE_MOVE_W,  MOUSE_CLICK,  MOUSE_MOVE_E,
  MOUSE_MOVE_SW, MOUSE_MOVE_S, MOUSE_MOVE_SE,
  MOUSE_DOUBLE_CLICK, MOUSE_RIGHT_CLICK, MOUSE_PRESS
};

const uint8_t membrane_actions_mouse[8][12] = {
  // Top row = 0
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  // row = 1
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  // row = 2
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  // row = 3
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  // row = 4, mousepad
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  MOUSE_MOVE_NW,  // mouse move NW
  MOUSE_MOVE_N, // mouse move N
  MOUSE_MOVE_NE,  // mouse move NE
  // row = 5, mousepad
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  MOUSE_MOVE_W, // mouse move W
  MOUSE_CLICK,  // mouse click
  MOUSE_MOVE_E, // mouse move E
  // row = 6, mousepad
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  MOUSE_MOVE_SW,  // mouse move SW
  MOUSE_MOVE_S, // mouse move S
  MOUSE_MOVE_SE,  // mouse move SE
  // row = 7, mousepad
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  MOUSE_DOUBLE_CLICK, // mouse double click
  MOUSE_RIGHT_CLICK,  // mouse right click
  MOUSE_PRESS,  // mouse press/release
};

#define MOUSE_MOVE  (10)
static bool num_lock=false;
static bool caps_lock=false;
static uint8_t shift_lock=0;  //0=off,1=on next key,2=lock on
static uint8_t ctrl_lock=0; //0=off,1=on next key,2=lock on
static uint8_t alt_lock=0;  //0=off,1=on next key,2=lock on
static uint8_t gui_lock=0;  //0=off,1=on next key,2=lock on

void clear_membrane(void)
{
  memset((void *)membrane, 0, sizeof(membrane));
  if (num_lock) tinyusb_key_press(HID_KEY_NUM_LOCK);
  if (caps_lock) tinyusb_key_press(HID_KEY_CAPS_LOCK);
  num_lock = caps_lock = false;
  shift_lock = ctrl_lock = alt_lock = gui_lock = 0;
  //Mouse.release(MOUSE_ALL);
  tinyusb_key_releaseAll();
}

void process_membrane_release(int x, int y)
{
  uint8_t row, col;
  uint16_t keycode, mousecode;
  col = x / 2;
  row = y / 3;
  membrane[row][col]--;
  if (membrane[row][col] < 0) membrane[row][col] = 0;
  if (membrane[row][col] != 0) return;
  keycode = membrane_actions[row][col];
  if (keycode) {
    switch (keycode) {
      case HID_KEY_SHIFT_LEFT:
        /* fall through */
      case HID_KEY_ALT_LEFT:
        /* fall through */
      case HID_KEY_CONTROL_LEFT:
        /* fall through */
      case HID_KEY_GUI_LEFT:
        /* do nothing */
        break;
      default:
        tinyusb_key_release(keycode);
        break;
    }
  }
  else {
    mousecode = membrane_actions_mouse[row][col];
    if (mousecode) {
      switch (mousecode) {
        case MOUSE_MOVE_NW:
          break;
        case MOUSE_MOVE_N:
          break;
        case MOUSE_MOVE_NE:
          break;
        case MOUSE_MOVE_W:
          break;
        case MOUSE_CLICK:
          break;
        case MOUSE_MOVE_E:
          break;
        case MOUSE_MOVE_SW:
          break;
        case MOUSE_MOVE_S:
          break;
        case MOUSE_MOVE_SE:
          break;
        case MOUSE_DOUBLE_CLICK:
          break;
        case MOUSE_RIGHT_CLICK:
          break;
        case MOUSE_PRESS:
          // Mouse.?
          break;
        default:
          break;
      }
    }
  }
}

void process_locking(uint8_t &lock_status, uint8_t LED, uint16_t keycode)
{
  switch (lock_status) {
    case 0:
      lock_status = 1;
      IK_set_led(LED, 1);
      break;
    case 1:
      lock_status = 2;
      IK_set_led(LED, 1);
      break;
    case 2:
      /* fall through */
    default:
      lock_status = 0;
      tinyusb_key_release(keycode);
      break;
  }
}

void process_membrane_press(int x, int y)
{
  uint8_t row, col;
  uint16_t keycode, mousecode;
  col = x / 2;
  row = y / 3;
  DBSerial.printf("col,row (%d,%d) membrane %d\n", col, row, membrane[row][col]);
  if (membrane[row][col] == 0) {
    keycode = membrane_actions[row][col];
    DBSerial.printf("keycode %X\n", keycode);
    if (keycode) {
      tinyusb_key_press(keycode);
      switch (keycode) {
        case HID_KEY_CAPS_LOCK:
          caps_lock = !caps_lock;
          IK_set_led(IK_LED_CAPS_LOCK, caps_lock);
          break;
        case HID_KEY_NUM_LOCK:
          num_lock = !num_lock;
          IK_set_led(IK_LED_NUM_LOCK, num_lock);
          break;
        case HID_KEY_SHIFT_LEFT:
          process_locking(shift_lock, IK_LED_SHIFT, keycode);
          if (shift_lock == 0) IK_set_led(IK_LED_SHIFT, 0);
          break;
        case HID_KEY_ALT_LEFT:
          process_locking(alt_lock, IK_LED_ALT, keycode);
          if (alt_lock == 0) IK_set_led(IK_LED_ALT, 0);
          break;
        case HID_KEY_CONTROL_LEFT:
          process_locking(ctrl_lock, IK_LED_CTRL_CMD, keycode);
          if (!ctrl_lock && !gui_lock) {
            IK_set_led(IK_LED_CTRL_CMD, 0);
          }
          break;
        case HID_KEY_GUI_LEFT:
          process_locking(gui_lock, IK_LED_CTRL_CMD, keycode);
          if (!ctrl_lock && !gui_lock) {
            IK_set_led(IK_LED_CTRL_CMD, 0);
          }
          break;
        default:
          if (shift_lock == 1) {
            shift_lock = 0;
            IK_set_led(IK_LED_SHIFT, 0);
            tinyusb_key_release(HID_KEY_SHIFT_LEFT);
          }
          if (alt_lock == 1) {
            alt_lock = 0;
            IK_set_led(IK_LED_ALT, 0);
            tinyusb_key_release(HID_KEY_ALT_LEFT);
          }
          if (ctrl_lock == 1) {
            ctrl_lock = 0;
            if (!ctrl_lock && !gui_lock) {
              IK_set_led(IK_LED_CTRL_CMD, 0);
            }
            tinyusb_key_release(HID_KEY_CONTROL_LEFT);
          }
          if (gui_lock == 1) {
            gui_lock = 0;
            if (!ctrl_lock && !gui_lock) {
              IK_set_led(IK_LED_CTRL_CMD, 0);
            }
            tinyusb_key_release(HID_KEY_GUI_LEFT);
          }
          break;
      }
    }
    else {
      mousecode = membrane_actions_mouse[row][col];
      if (mousecode) {
        switch (mousecode) {
          case MOUSE_MOVE_NW:
            // Mouse.move(-MOUSE_MOVE, -MOUSE_MOVE, 0);
            break;
          case MOUSE_MOVE_N:
            // Mouse.move(0, -MOUSE_MOVE, 0);
            break;
          case MOUSE_MOVE_NE:
            // Mouse.move(MOUSE_MOVE, -MOUSE_MOVE, 0);
            break;
          case MOUSE_MOVE_W:
            // Mouse.move(-MOUSE_MOVE, 0, 0);
            break;
          case MOUSE_CLICK:
            // Mouse.click();
            break;
          case MOUSE_MOVE_E:
            // Mouse.move(MOUSE_MOVE, 0, 0);
            break;
          case MOUSE_MOVE_SW:
            // Mouse.move(MOUSE_MOVE, -MOUSE_MOVE, 0);
            break;
          case MOUSE_MOVE_S:
            // Mouse.move(0, MOUSE_MOVE, 0);
            break;
          case MOUSE_MOVE_SE:
            // Mouse.move(MOUSE_MOVE, MOUSE_MOVE, 0);
            break;
          case MOUSE_DOUBLE_CLICK:
            break;
          case MOUSE_RIGHT_CLICK:
            break;
          case MOUSE_PRESS:
            break;
          default:
            break;
        }
      }
    }
  }
  membrane[row][col]++;
}

void setup()
{
  // Turn off built-in RED LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#ifdef ADAFRUIT_TRINKET_M0
  // Turn off built-in Dotstar RGB LED
  strip.begin();
  strip.clear();
  strip.show();
#endif
  // Do this before using Serial
  tinyusb_setup();
#if DEBUG_SERIAL
  DBSerial.begin(115200);
  while(!DBSerial) delay(1);
#endif
  IKSerial.begin(115200);

  IK_uart_setup();
  DBSerial.println("ikrawevent_ard setup done");
}

// IK events
void IK_press(int x, int y)
{
  DBSerial.printf("press(%d,%d)\n", x, y);
  process_membrane_press(x, y);
}

void IK_release(int x, int y)
{
  DBSerial.printf("release(%d,%d)\n", x, y);
  process_membrane_release(x, y);
}

void IK_switch(int num, int state)
{
  DBSerial.print("switch ");
  DBSerial.print(num);
  DBSerial.print(',');
  DBSerial.println(state);
}

void IK_sensor(int num, int state)
{
  DBSerial.print("sensor ");
  DBSerial.print(num);
  DBSerial.print(',');
  DBSerial.println(state);
}

void IK_version(int major, int minor)
{
  DBSerial.print("FW version ");
  DBSerial.print(major);
  DBSerial.print('.');
  DBSerial.println(minor);
}

void IK_onoffswitch(int state)
{
  DBSerial.print("Top on/off switch ");
  DBSerial.println(state);
  if (state == 0) {
    clear_membrane();
    IK_set_led(IK_LED_SHIFT, 0);
    IK_set_led(IK_LED_CAPS_LOCK, 0);
    IK_set_led(IK_LED_MOUSE, 0);
    IK_set_led(IK_LED_ALT, 0);
    IK_set_led(IK_LED_CTRL_CMD, 0);
    IK_set_led(IK_LED_NUM_LOCK, 0);
  }
}

void IK_corrmemb(int x, int y)
{
  DBSerial.print("correct membrane ");
  DBSerial.print(x);
  DBSerial.print(',');
  DBSerial.println(y);
}

void IK_corrswitch(int x, int y)
{
  DBSerial.print("correct switch ");
  DBSerial.print(x);
  DBSerial.print(',');
  DBSerial.println(y);
}

void IK_corrdone()
{
  DBSerial.println("correct done");
}

void IK_connect()
{
  DBSerial.println("IK connect");
}

void IK_disconnect()
{
  DBSerial.println("IK disconnect");
}

void IK_sernum(const uint8_t *buf, size_t len)
{
  // The first byte is the event type so skip it
  DBSerial.print("IK serial number ");
  DBSerial.write(buf+1, len-1);
  DBSerial.println();
}

void eventDecode(const uint8_t *buf, size_t len)
{
  switch (buf[0]) {
    case IK_EVENT_MEMBRANE_PRESS:
      IK_press(buf[1], buf[2]);
      break;
    case IK_EVENT_MEMBRANE_RELEASE:
      IK_release(buf[1], buf[2]);
      break;
    case IK_EVENT_SWITCH:
      IK_switch(buf[1], buf[2]);
      break;
    case IK_EVENT_SENSOR_CHANGE:
      IK_sensor(buf[1], buf[2]);
      break;
    case IK_EVENT_VERSION:
      IK_version(buf[1], buf[2]);
      break;
    case IK_EVENT_ONOFFSWITCH:
      IK_onoffswitch(buf[1]);
      break;
    case IK_EVENT_CORRECT_MEMBRANE:
      IK_corrmemb(buf[1], buf[2]);
      break;
    case IK_EVENT_CORRECT_SWITCH:
      IK_corrswitch(buf[1], buf[2]);
      break;
    case IK_EVENT_CORRECT_DONE:
      IK_corrdone();
      break;
    case IK_EVENT_CONNECT:
      IK_connect();
      break;
    case IK_EVENT_DISCONNECT:
      IK_disconnect();
      break;
    case IK_EVENT_SERNUM:
      IK_sernum(buf, len);
      break;
    default:
      DBSerial.print("IK eventDecode Unknown event ");
      DBSerial.println(buf[0]);
      break;
  }
}

// IK commands
void IK_get_version()
{
  uint8_t outbuf[] = {0xFF, 0x01, IK_CMD_GET_VERSION};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_set_led(uint8_t num, uint8_t state)
{
  uint8_t outbuf[] = {0xFF, 0x03, IK_CMD_LED, num, state};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_set_tone(uint8_t frequency, uint8_t duration, uint8_t volume)
{
  uint8_t outbuf[] = {0xFF, 0x03, IK_CMD_TONE, frequency, duration, volume};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_get_onoff()
{
  uint8_t outbuf[] = {0xFF, 0x01, IK_CMD_ONOFFSWITCH};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_get_correct()
{
  uint8_t outbuf[] = {0xFF, 0x01, IK_CMD_CORRECT};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_reset()
{
  uint8_t outbuf[] = {0xFF, 0x01, IK_CMD_RESET_DEVICE};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_get_all_sensors()
{
  uint8_t outbuf[] = {0xFF, 0x01, IK_CMD_ALL_SENSORS};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_get_sn()
{
  uint8_t outbuf[] = {0xFF, 0x01, IK_CMD_GET_SN};
  IKSerial.write(outbuf, sizeof(outbuf));
  delay(2);
}

void IK_uart_setup()
{
  int i;

  // All LEDs on
  for (i = 0; i < 12; i++) IK_set_led(i, 1);

  IK_set_tone(0,0,0);
  IK_get_onoff();
  IK_get_all_sensors();
  IK_get_version();
  IK_get_sn();
  IK_uart_loop();
  IK_get_correct();
  IK_uart_loop();

  // All LEDs off
  for (i = 0; i < 12; i++) IK_set_led(i, 0);
}

void IK_uart_loop()
{
  static uint8_t eventState=0;
  static size_t  eventLen=0;
  static uint8_t eventBuf[64];
  static uint8_t eventIndex;
  uint8_t rawBuf[64];

  int bytesAvail;
  while ((bytesAvail = IKSerial.available()) > 0) {
    size_t bytesIn = IKSerial.readBytes(rawBuf, min(bytesAvail, sizeof(rawBuf)));
    for (size_t i = 0; i < bytesIn; i++) {
      switch(eventState) {
        case 0:
          if (rawBuf[i] == 0xFF) {
            eventState = 1;
          }
          break;
        case 1:
          eventLen = rawBuf[i];
          eventState = 2;
          eventIndex = 0;
          break;
        case 2:
          eventBuf[eventIndex] = rawBuf[i];
          eventIndex++;
          if (eventIndex >= eventLen) {
            eventState = 0;
            eventDecode(eventBuf, eventLen);
          }
          break;
        default:
          DBSerial.println("Invalid eventState");
          break;
      }
    }
  }
}

void loop()
{
  IK_uart_loop();
  tinyusb_loop();
}
