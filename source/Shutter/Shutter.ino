
#include <SPI.h>
#include "Clock.h"
#include "Button.h"
 
#define PIN_LOAD     10
#define PIN_BTN_0    7
#define PIN_BTN_1    8
#define PIN_BTN_2    9
#define PIN_MOT_EN   6
#define PIN_MOT_1A   5
#define PIN_MOT_2A   4
#define PIN_MOT_3A   3
#define PIN_MOT_4A   2


#define MAXIM_DECODE       0x09
#define MAXIM_INTENSITY    0x0A
#define MAXIM_SCAN         0x0B
#define MAXIM_ENABLE       0x0C
#define MAXIM_TEST         0x0F

#define LETTER_4           B00110011
#define LETTER_A           B01110111
#define LETTER_C           B01001110
#define LETTER_b           B00011111
#define LETTER_I           B00000110
#define LETTER_0           B01111110
#define LETTER_1           B00110000
#define LETTER_2           B01101101
#define LETTER_3           B01111001
#define LETTER_4           B00110011
#define LETTER_5           B01011011
#define LETTER_6           B01011111
#define LETTER_7           B01110000
#define LETTER_8           B01111111
#define LETTER_9           B01111011
#define LETTER_n           B00010101
#define LETTER_u           B00011100
#define LETTER_l           B00000110
#define LETTER_i           B00000100
#define LETTER_E           B01001111
#define LETTER_X           B00110111
#define LETTER_t           B00001111
#define LETTER_MINUS       B00000001
#define LETTER_Q           B11111110
#define LETTER_r           B00000101
#define LETTER_q           B01110011
#define LETTER_h           B00010111
#define LETTER_o           B00011101
#define LETTER_F           B01000111
#define LETTER_S           B01011011
#define LETTER_U           B00111110
#define LETTER_P           B01100111
#define LETTER_J           B00111100
#define LETTER_j           B00111000
#define LETTER_d           B00111101
//                          PABCDEFG 

byte digits[10] = {
    LETTER_0,
    LETTER_1,
    LETTER_2,
    LETTER_3,
    LETTER_4,
    LETTER_5,
    LETTER_6,
    LETTER_7,
    LETTER_8,
    LETTER_9,
};


#define MESSAGE_NULL         0
#define MESSAGE_QUIT         1
#define MESSAGE_CHASI        2
#define MESSAGE_BRIGHT       3
#define MESSAGE_ON           4
#define MESSAGE_OFF          5
#define MESSAGE_SETUP        6
#define MESSAGE_ADJUST       7
#define MESSAGE_CORRECT      8

byte messages[] = {
    LETTER_n, LETTER_u, LETTER_l, LETTER_l, 0,
    LETTER_q, LETTER_u, LETTER_i, LETTER_t, 0,
    LETTER_4, LETTER_A, LETTER_C, LETTER_b, LETTER_I,
    LETTER_b, LETTER_r, LETTER_i, LETTER_h, LETTER_t,
    LETTER_0, LETTER_n, 0, 0, 0,
    LETTER_0, LETTER_F, LETTER_F, 0, 0,
    LETTER_S, LETTER_E, LETTER_t, LETTER_U, LETTER_P,  
    LETTER_A, LETTER_d, LETTER_j, LETTER_S, LETTER_t,  
    LETTER_C, LETTER_o, LETTER_r, LETTER_r, 0,  
};

#define MENU_DISABLED    0
#define MENU_CHOOSE      1
#define MENU_MODIFY      2

#define MENU_ITEM_EDITABLE    0
#define MENU_ITEM_READONLY    1
#define MENU_ITEM_ENUM        2

class MenuItem {
public:
    byte message;
    byte type;
    long min;
    long max;
    long delta;
    long *value;
    
    MenuItem() {
        type = 0;
        message = 0;
        delta = 1;
        min = 0;
        max = 10;
        value = NULL;
    }
    MenuItem(byte message, byte type, long min, long max, long delta, long *value) {
        this->type = type;
        this->message = message;
        this->min = min;
        this->max = max;
        this->delta = delta;
        this->value = value;
    }
};


class Menu {
    
    MenuItem *items;
    byte num_items;
    byte cur_item;
    byte mode;
    byte display[5];
    unsigned long last_activity;
    
    void updateActivity() {
        last_activity = millis();
    }
    
    void checkActivity() {
        if((unsigned long)(millis() - last_activity) > 10000) {
            setMode(MENU_DISABLED);
        }
    }
    
public:
    
    Menu(MenuItem *items, byte num_items) {
        this->items = items;
        this->num_items = num_items;
        cur_item = 0;
        mode = MENU_DISABLED;
        last_activity = millis();
    }
    
    
    void setMode(byte mode) {
        this->mode = mode;
    }
    
    void change(int delta) {
        updateActivity();
        switch(mode) {
            case MENU_DISABLED: 
                break;
            case MENU_CHOOSE:
                cur_item = (cur_item + num_items + 1 + delta) % (num_items + 1);
                break;
            case MENU_MODIFY:
                if(items[cur_item].type == MENU_ITEM_READONLY) break;
                if(items[cur_item].value == NULL) break;
                long *value = items[cur_item].value;
                if(items[cur_item].type != MENU_ITEM_ENUM) delta *= items[cur_item].delta;
                *value += delta;
                if(items[cur_item].type == MENU_ITEM_ENUM) {
                    long min = items[cur_item].min;
                    long max = items[cur_item].max;
                    long len = max - min + 1;
                    *value = (*value - min + len) % len + min;
                } else {
                    if(*value > items[cur_item].max) *value = items[cur_item].max;
                    if(*value < items[cur_item].min) *value = items[cur_item].min;
                }
                break;
        }
    }
    
    void up() {
        change(+1);
    }
    
    void down() {
        change(-1);
    }
    
    void use() {
        updateActivity();
        switch(mode) {
            case MENU_DISABLED:
                cur_item = 0;
                mode = MENU_CHOOSE; 
                break;
            case MENU_CHOOSE:
                if(cur_item == num_items) {
                    mode = MENU_DISABLED;
                    break;
                }
                mode = MENU_MODIFY;
                break;
            case MENU_MODIFY:
                mode = MENU_DISABLED;
                break;
        }
    }
    
    byte *getDisplay() {
        
        if(mode == MENU_CHOOSE) {
            
            // exit message by default
            byte message = 1;
            
            if(cur_item < num_items) message = items[cur_item].message;
            
            return &messages[message * 5];            
        }
        
        if(mode == MENU_MODIFY) {
            int cur_letter = 0;
            if(items[cur_item].value == NULL) return &messages[0];
            long value = *items[cur_item].value;
            if(items[cur_item].type == MENU_ITEM_ENUM) {
                byte message = items[cur_item].delta + value - items[cur_item].min;
                return &messages[message * 5];
            }
            
            
            for(int i=0; i<5; i++) {
                display[i] = 0;
            }
            
            byte neg = 0;
            if(value < 0) {
                neg = 1;
                value = -value;
            }
            
            cur_letter = 4;
            long div = 1;
            
            while(cur_letter >= 0) {
                long dig = (value / div) % 10;
                display[cur_letter--] = digits[dig];
                value -= dig * div;
                if(value == 0) break;
                div *= 10;
            }
            if(cur_letter >= 0 && neg) display[cur_letter] = LETTER_MINUS;
            return display;
        }
        return &messages[0];
    }
    
    byte isEnabled() {
        return mode != MENU_DISABLED;
    }
    
    void update() {
        switch(mode) {
            case MENU_DISABLED:
                break;
            case MENU_CHOOSE:
                checkActivity();
                break;
            case MENU_MODIFY:
                checkActivity();
                break;
        }
    }
    
    byte getCurrentItem() {
        return cur_item;
    }
    
    byte getMode() {
        return mode;
    }
    
};


Clock clock;

long value_1 = 10;
long value_enum = 0;
long brightness = 5;

MenuItem menu_items[4] = {
    //MenuItem(MESSAGE_NULL, MENU_ITEM_EDITABLE, 0, 10, 1, &value_0),
    MenuItem(MESSAGE_CHASI, MENU_ITEM_EDITABLE, 10, 20, 2, &value_1),
    MenuItem(MESSAGE_BRIGHT, MENU_ITEM_EDITABLE, 0, 15, 1, &brightness),
    MenuItem(MESSAGE_CORRECT, MENU_ITEM_EDITABLE, -9999, 9999, 1, clock.getCorrectionPointer()),
};

Menu menu(menu_items, 3);

Button button_0(PIN_BTN_0);
Button button_1(PIN_BTN_1);
Button button_2(PIN_BTN_2);

#define SET_DISABLED    0 
#define SET_HOURS       1
#define SET_MINUTES     2

byte set_mode = SET_DISABLED;
byte set_blink = 0;
unsigned long blink_time;


#define MOTOR_BREAKE    0
#define MOTOR_FORWARD   1
#define MOTOR_BACKWARD  2
#define MOTOR_DISABLE   3

void motorInit() {
    pinMode(PIN_MOT_EN, OUTPUT);
    pinMode(PIN_MOT_1A, OUTPUT);
    pinMode(PIN_MOT_2A, OUTPUT);
    pinMode(PIN_MOT_3A, OUTPUT);
    pinMode(PIN_MOT_4A, OUTPUT);
}

void motorMode(byte mode) {
    switch (mode) {
        case MOTOR_BREAKE: 
            digitalWrite(PIN_MOT_EN, HIGH);
            digitalWrite(PIN_MOT_3A, LOW);
            digitalWrite(PIN_MOT_4A, LOW);
            break;
        case MOTOR_FORWARD: 
            digitalWrite(PIN_MOT_EN, HIGH);
            digitalWrite(PIN_MOT_3A, HIGH);
            digitalWrite(PIN_MOT_4A, LOW);
            break;
        case MOTOR_BACKWARD: 
            digitalWrite(PIN_MOT_EN, HIGH);
            digitalWrite(PIN_MOT_3A, LOW);
            digitalWrite(PIN_MOT_4A, HIGH);
            break;
        case MOTOR_DISABLE: 
            digitalWrite(PIN_MOT_EN, LOW);
            digitalWrite(PIN_MOT_3A, LOW);
            digitalWrite(PIN_MOT_4A, LOW);
            break;
    }
}
/**
 * Transfers data to a MAX7219/MAX7221 register.
 * 
 * @param address The register to load data into
 * @param value   Value to store in the register
 */
void maxTransfer(uint8_t address, uint8_t value) {
 
  // Ensure LOAD/CS is LOW
  digitalWrite(PIN_LOAD, LOW);
 
  // Send the register address
  SPI.transfer(address);
 
  // Send the value
  SPI.transfer(value);
 
  // Tell chip to load in data
  digitalWrite(PIN_LOAD, HIGH);
  
  motorInit();
}
  
void setButtonLed(byte i) {
  switch(i % 3) {
      case 0: maxTransfer(6, B01000000); break;
      case 1: maxTransfer(6, B00100000); break;
      case 2: maxTransfer(6, B00000010); break;
  };
}

void setDisplay(byte *display) {
    for(int i=0; i<5; i++) {
         maxTransfer(i + 1, display[i]);
    }    
}

void setup() {

    #define TIME(a) (__TIME__[a] - 48)
    byte h = TIME(0) * 10 + TIME(1);
    byte m = TIME(3) * 10 + TIME(4);
    byte s = TIME(6) * 10 + TIME(7);
    #undef TIME
    clock.setHours(h);
    clock.setMinutes(m);
    clock.setSeconds(s + 12);  
        
  // Set load pin to output
  pinMode(PIN_LOAD, OUTPUT);
 
  // Reverse the SPI transfer to send the MSB first  
  SPI.setBitOrder(MSBFIRST);
  
  // Start SPI
  SPI.begin();
 
  // Run test
  // All LED segments should light up
  //maxTransfer(MAXIM_TEST, 0x01);
  //delay(1000);
  //maxTransfer(MAXIM_TEST, 0x00);
      
  // Only scan one digit
  maxTransfer(MAXIM_SCAN, 0x05);
  
  // Turn on chip
  maxTransfer(MAXIM_ENABLE, 0x01);
  
  // Disable mode B
  maxTransfer(MAXIM_DECODE, 0x00);
  maxTransfer(1, LETTER_4);
  maxTransfer(2, LETTER_A);
  maxTransfer(3, LETTER_C);
  maxTransfer(4, LETTER_b);
  maxTransfer(5, LETTER_I);
  
  for(int i=0; i<16; i++) {
      maxTransfer(MAXIM_INTENSITY, i);      
      setButtonLed((i / 4) % 3);
      delay(32);
  }
  for(int i=1; i<15; i++) {
      setButtonLed((i / 4) % 3);
      delay(32);
  }
  for(int i=0; i<16; i++) {
      maxTransfer(MAXIM_INTENSITY, 15 - i);
      setButtonLed((i / 3) % 3);
      delay(32);
  }
  for(int i=1; i<6; i++) {
      maxTransfer(i, 0x00);
  }
  for(int i=1; i<15; i++) {
      setButtonLed((i / 4) % 3);
      delay(32);
  }
    
 // Enable mode B
  maxTransfer(MAXIM_DECODE, ~(1 << 5));
}



void loop() {
    
    if(set_mode == SET_DISABLED) {
        clock.update();
    } else {        
        set_blink = (millis() - blink_time) / 250 % 2 == 0;
    }
    
    button_0.update();
    button_1.update();
    button_2.update();
    
    menu.update();
 	
    // hour button
    if(set_mode == SET_DISABLED && !menu.isEnabled()) {
        // hour button
        if (button_0.pressed()) {
            motorMode(MOTOR_BACKWARD);
        } else 
        // min button
        if (button_2.pressed()) {
            motorMode(MOTOR_FORWARD);
    	//clock.setMinutes(clock.getMinutes() + 1);
        } else {
            motorMode(MOTOR_BREAKE);
        }
        if (button_1.down()) {
            menu.use();    
        }
    } else {
        motorMode(MOTOR_BREAKE);
        
        if(set_mode != SET_DISABLED) {
            if (button_1.down()) {
                set_mode = (set_mode + 1) % 3;
                clock.restart();
                blink_time = millis();
                //clock.setMinutes(clock.getMinutes() + 1);
            }
        
            if(button_2.down()) {
                if(set_mode == SET_MINUTES) {
                    clock.setMinutes(clock.getMinutes() + 1);
                } else {
                    clock.setHours(clock.getHours() + 1);
                }
            }
            
            if(button_0.down()) {
                if(set_mode == SET_MINUTES) {
                    clock.setMinutes(clock.getMinutes() - 1);
                } else {
                    clock.setHours(clock.getHours() - 1);
                }
            }
        } else {
             
            if (button_0.down()) {
                menu.down();    
            }
         
            if (button_2.down()) {
                menu.up();    
            }
            if (button_1.down()) {
                menu.use();
                if(menu.getCurrentItem() == 0 && menu.getMode() == MENU_MODIFY) {
                    set_mode = 1;
                    clock.restart();
                    blink_time = millis();
                    menu.use();
                }  
            }
        }
        
    }



/*  
  maxTransfer(1, clock.getHours() / 10);
  maxTransfer(2, clock.getHours() % 10);
  maxTransfer(3, clock.isTire() ? 0xA : 0xF);
  maxTransfer(4, clock.getMinutes() / 10);
  maxTransfer(5, clock.getMinutes() % 10);
*/
      
   // Use lowest intensity
  maxTransfer(MAXIM_INTENSITY, brightness);
  
  if(menu.isEnabled() && set_mode == SET_DISABLED) {
      // Disable mode B
      maxTransfer(MAXIM_DECODE, 0);
      setDisplay(menu.getDisplay());
  } else {
      // Enable mode B
      maxTransfer(MAXIM_DECODE, ~(1 << 5));
      maxTransfer(1, (set_mode == SET_HOURS && set_blink) ? 0xF : clock.getHours() / 10);
      maxTransfer(2, (set_mode == SET_HOURS && set_blink) ? 0xF : clock.getHours() % 10);
      
      maxTransfer(3, (clock.isTire() || set_mode != SET_DISABLED) ? 0xA : 0xF);
      
      maxTransfer(4, (set_mode == SET_MINUTES && set_blink) ? 0xF : clock.getMinutes() / 10);
      maxTransfer(5, (set_mode == SET_MINUTES && set_blink) ? 0xF : clock.getMinutes() % 10);
  }
  switch(clock.getSeconds() % 3) {
      case 0: maxTransfer(6, B01000000); break;
      case 1: maxTransfer(6, B00100000); break;
      case 2: maxTransfer(6, B00000010); break;
  };

  delay(10);
  
}
