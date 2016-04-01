/******************************************************************************
     * Based on the file PS2X_lib.cpp from library Arduino-PS2X. 
     * Author project library Arduino-PS2X - Bill Porter.
     * Version on commit 48df1d6 on 7 Jul 2013.
     * Project description Arduino-PS2X - http://www.billporter.info/2010/06/05/playstation-2-controller-arduino-library-v1-0/. 
     * You can find the code of the Arduino-PS2X project here - https://github.com/madsci1016/Arduino-PS2X
     * License GNU GPL v3
******************************************************************************/

//*******************************  Библиотеки  ******************************//
// Класс Arduino.
#include "Arduino.h"

// Библиотека для работы с Playstation joystick.
#include "PS2X.h"

// Класс определений.
#include "Def.h"

// Класс работы с портами.
#include "FastIO.h"

// Библиотека работы с математическими операциями.
#include <math.h>

// Библиотека работы с стандартными средствами ввода / вывода (IO).
#include <stdio.h>

// Библиотека целочисленных типов.
#include <stdint.h>

// Библиотека ввода вывода.
#include <avr/io.h>

// Библиотека для работы с задержками.
#include <util/delay.h>
//*******************************  /Библиотеки  *****************************//

static byte enter_config[]={0x01,0x43,0x00,0x01,0x00};
static byte set_mode[]={0x01,0x44,0x00,0x01,0x03,0x00,0x00,0x00,0x00};
static byte set_bytes_large[]={0x01,0x4F,0x00,0xFF,0xFF,0x03,0x00,0x00,0x00};
static byte exit_config[]={0x01,0x43,0x00,0x00,0x5A,0x5A,0x5A,0x5A,0x5A};
static byte enable_rumble[]={0x01,0x4D,0x00,0x00,0x01};
static byte type_read[]={0x01,0x45,0x00,0x5A,0x5A,0x5A,0x5A,0x5A,0x5A};

boolean PS2X::NewButtonState() 
{
  return ((last_buttons ^ buttons) > 0);
}

boolean PS2X::NewButtonState(unsigned int button) 
{
  return (((last_buttons ^ buttons) & button) > 0);
}

boolean PS2X::ButtonPressed(unsigned int button) 
{
  return(NewButtonState(button) & Button(button));
}

boolean PS2X::ButtonReleased(unsigned int button) 
{
  return((NewButtonState(button)) & ((~last_buttons & button) > 0));
}

boolean PS2X::Button(uint16_t button) 
{
  return ((~buttons & button) > 0);
}

unsigned int PS2X::ButtonDataByte() 
{
   return (~buttons);
}

byte PS2X::Analog(byte button) 
{
   return PS2data[button];
}

unsigned char PS2X::_gamepad_shiftinout (char byte) 
{
   unsigned char tmp = 0;
   for(unsigned char i=0;i<8;i++) 
   {
      if(CHK(byte,i)) CMD_SET();
      else CMD_CLR();
	  
      CLK_CLR();
      _delay_us(CTRL_CLK);

      if(DAT_CHK()) bitSet(tmp,i);

      CLK_SET();
      
      #if CTRL_CLK_HIGH
	_delay_us(CTRL_CLK_HIGH);
      #endif
   }
   CMD_SET();
   _delay_us(CTRL_BYTE_DELAY);
   return tmp;
}

void PS2X::read_gamepad() 
{
   read_gamepad(false, 0x00);
}

boolean PS2X::read_gamepad(boolean motor1, byte motor2) 
{
   double temp = millis() - last_read;

   if (temp > 1500) 
   {
      //waited to long
      reconfig_gamepad();
   }

   if(temp < read_delay) 
   {
       //waited too short
       delay((read_delay - temp));
   }

   if(motor2 != 0x00)
   {
     //noting below 40 will make it spin
     motor2 = map(motor2,0,255,0x40,0xFF); 
   }

   char dword[9] = {0x01,0x42,0,motor1,motor2,0,0,0,0};
   byte dword2[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

   // Try a few times to get valid data...
   for (byte RetryCnt = 0; RetryCnt < 5; RetryCnt++) 
   {
      CMD_SET();
      CLK_SET();
      ATT_CLR(); // low enable joystick

      _delay_us(CTRL_BYTE_DELAY);

      //Send the command to send button and joystick data;
      for (int i = 0; i<9; i++) 
      {
         PS2data[i] = _gamepad_shiftinout(dword[i]);
      }

      if(PS2data[1] == 0x79) 
      {  
        //if controller is in full data return mode, get the rest of data
        for (int i = 0; i<12; i++) 
	      {
          PS2data[i+9] = _gamepad_shiftinout(dword2[i]);
        }
      }

      // HI disable joystick
      ATT_SET(); 

      // Check to see if we received valid data or not.  
      // We should be in analog mode for our data to be valid (analog == 0x7_)
      if ((PS2data[1] & 0xf0) == 0x70)
      {
        break;
      }

      // If we got to here, we are not in analog mode, try to recover...
      reconfig_gamepad(); // try to get back into Analog mode.
      delay(read_delay);
   }

   // If we get here and still not in analog mode (=0x7_), try increasing the read_delay...
   if ((PS2data[1] & 0xf0) != 0x70) 
   {
      if (read_delay < 10)
      {
        read_delay++;   // see if this helps out...
      }
   }

   last_buttons = buttons; //store the previous buttons states

   buttons = *(uint16_t*)(PS2data+3);   //store as one value for multiple functions

   last_read = millis();
   return ((PS2data[1] & 0xf0) == 0x70);  // 1 = OK = analog mode - 0 = NOK
}

byte PS2X::config_gamepad(uint8_t clk, uint8_t cmd, uint8_t att, uint8_t dat) 
{
   return config_gamepad(clk, cmd, att, dat, false, false);
}

byte PS2X::config_gamepad(uint8_t clk, uint8_t cmd, uint8_t att, uint8_t dat, bool pressures, bool rumble) 
{
  byte temp[sizeof(type_read)];

  _clk_mask = digitalPinToBitMask(clk);
  _clk_oreg = portOutputRegister(digitalPinToPort(clk));
  _cmd_mask = digitalPinToBitMask(cmd);
  _cmd_oreg = portOutputRegister(digitalPinToPort(cmd));
  _att_mask = digitalPinToBitMask(att);
  _att_oreg = portOutputRegister(digitalPinToPort(att));
  _dat_mask = digitalPinToBitMask(dat);
  _dat_ireg = portInputRegister(digitalPinToPort(dat));
  
  MC_SET_PIN_OUTPUT(PS2_PIN_CLK);
  MC_SET_PIN_OUTPUT(PS2_PIN_CMD);
  MC_SET_PIN_OUTPUT(PS2_PIN_SEL);
  MC_SET_PIN_INPUT(PS2_PIN_DAT);

  CMD_SET(); // SET(*_cmd_oreg,_cmd_mask);
  CLK_SET();

  //new error checking. First, read gamepad a few times to see if it's talking
  read_gamepad();
  read_gamepad();

  //see if it talked - see if mode came back. 
  //If still anything but 41, 73 or 79, then it's not talking
  if(PS2data[1] != 0x41 && PS2data[1] != 0x73 && PS2data[1] != 0x79)
  { 
    return 1; //return error code 1
  }

  //try setting mode, increasing delays if need be.
  read_delay = 1;

  for(int y = 0; y <= 10; y++) 
  {
    sendCommandString(enter_config, sizeof(enter_config)); //start config run

    //read type
    _delay_us(CTRL_BYTE_DELAY);

    CMD_SET();
    CLK_SET();
    ATT_CLR(); // low enable joystick

    _delay_us(CTRL_BYTE_DELAY);

    for (int i = 0; i<9; i++) 
	  {
      temp[i] = _gamepad_shiftinout(type_read[i]);
    }

    ATT_SET(); // HI disable joystick

    controller_type = temp[3];

    sendCommandString(set_mode, sizeof(set_mode));
    sendCommandString(exit_config, sizeof(exit_config));

    read_gamepad();

    if(pressures)
    {
      if(PS2data[1] == 0x79)
      {
        break;
      }

      if(PS2data[1] == 0x73)
      {
        return 3;
      }
    }

    if(PS2data[1] == 0x73)
    {
      break;
    }

    if(y == 10)
    {
      return 2; //exit function with error
    }
    read_delay += 1; //add 1ms to read_delay
  }
  return 0; //no error if here
}

void PS2X::sendCommandString(byte string[], byte len) 
{
  ATT_CLR(); // low enable joystick
  for (int y=0; y < len; y++)
  {
    _gamepad_shiftinout(string[y]);
  }
  ATT_SET(); //high disable joystick
  delay(read_delay);                  //wait a few
}

byte PS2X::readType() 
{
  if(controller_type == 0x03)
  {
    return 1;
  }
  else if(controller_type == 0x01)
  {
    return 2;
  }
  else if(controller_type == 0x0C)  
  {
    return 3;  //2.4G Wireless Dual Shock PS2 Game Controller	
  }
  return 0;
}

void PS2X::reconfig_gamepad()
{
  sendCommandString(enter_config, sizeof(enter_config));
  sendCommandString(set_mode, sizeof(set_mode));
  sendCommandString(exit_config, sizeof(exit_config));
}

inline void  PS2X::CLK_SET(void) 
{
  register uint8_t old_sreg = SREG;
  cli();
  *_clk_oreg |= _clk_mask;
  SREG = old_sreg;
}

inline void  PS2X::CLK_CLR(void) 
{
  register uint8_t old_sreg = SREG;
  cli();
  *_clk_oreg &= ~_clk_mask;
  SREG = old_sreg;
}

inline void  PS2X::CMD_SET(void) 
{
  register uint8_t old_sreg = SREG;
  cli();
  *_cmd_oreg |= _cmd_mask; 
  SREG = old_sreg;
}

inline void  PS2X::CMD_CLR(void) 
{
  register uint8_t old_sreg = SREG;
  cli();
  *_cmd_oreg &= ~_cmd_mask; 
  SREG = old_sreg;
}

inline void  PS2X::ATT_SET(void) 
{
  register uint8_t old_sreg = SREG;
  cli();
  *_att_oreg |= _att_mask ;
  SREG = old_sreg;
}

inline void PS2X::ATT_CLR(void) 
{
  register uint8_t old_sreg = SREG;
  cli();
  *_att_oreg &= ~_att_mask;
  SREG = old_sreg;
}

inline bool PS2X::DAT_CHK(void) 
{
  return (*_dat_ireg & _dat_mask) ? true : false;
}


