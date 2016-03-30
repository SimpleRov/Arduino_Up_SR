//*******************************  Библиотеки  ******************************//
// Класс Arduino.
#include "Arduino.h"

// Класс работы с битами.
#include "BitsMacros.h"

// Файл конфигурации.
#include "Config.h"

// Класс определений.
#include "Def.h"

// Класс глобальные переменные.
#include "GlobalVar.h"

// Класс работы с портами.
#include "FastIO.h"

// Класс работы с UART.
#include "UART.h"

// Класс протокола передачи данных.
#include "Protocol.h"

// Библиотека для работы с Playstation joystick.
#include "PS2X.h" 

// Библиотека для работы с задержками.
#include <util/delay.h>

#include <SoftwareSerial.h>
//******************************* /Библиотеки  ******************************//

//*************  Настройки и переменные для Playstation joystick  ***********//
// Объявляем класс - для работы с Playstation joystick.
PS2X ps2x;

// Структура хранящая предыдущие значения нажатых кнопок и положения стиков джойстика от PS2.
ps2Struct ps2SPrev;

// Переменная хранящая тип Playstation joystick.
uint8_t ps2Type = 0;

// Переменная хранящая наличие ошибок для Playstation joystick.
uint8_t ps2Error = 0;

// Переменная хранящая время последнего чтения данных от Playstation joystick.
uint16_t ps2PreviousT = 0;
 
// Константа хранящая промежуток между чтениями данных от Playstation joystick.
#define ps2T 100

// Константа хранящая промежуток между чтениями данных от Playstation joystick.
#define PS2_JOYSTICK_DEF_VALUE 123
//*************  /Настройки и переменные для Playstation joystick  **********//

//*************************  Настройки SoftwareSerial  **********************//
SoftwareSerial DebugSoftwareSerial(3, 2);       // RX | TX
//************************* /Настройки SoftwareSerial  **********************//

//*****************************  Основные функции  **************************//
/// <summary>
/// Основная функция main.
/// </summary>
int main(void)
{   
    init();

    #if defined(USBCON)
      USBDevice.attach();
    #endif
    
    setup();
   
    for (;;) 
    {
        loop();
        // В проекте не используется стандартная библиотека Serial. 
        //if (serialEventRun) serialEventRun();
    }
       
    return 0;
}

void setup()
{
  // Настраиваем входы/выходы.
  SetupPin();
  
  // Настраиваем и открываем необходимые UART порты.
  SetupUart();
  
  // Настраиваем Playstation joystick.
  SetupPlaystationJoystick();
  
  DebugSerial->println("Begin");
  
  // Немного подождем, вдруг, что-то не успело инициализировать.
  _delay_ms(3000);
}

/// <summary>
/// Настраиваем входы/выходы.
/// </summary>
void SetupPin()
{
  // Настройка пина дебага.
  #if defined(LED_PIN)
    MC_SET_PIN_OUTPUT(LED_PIN);
    
    MC_WRITE_PIN(LED_PIN, LOW);
  #endif 
}

/// <summary>
/// Настраиваем и открываем необходимые UART порты.
/// </summary>
void SetupUart()
{
  // Инициализируем и открываем UART порт связи с Rov
  UARTOpen(UART_PORT, UART_PORT_SPEED);
  
  DebugSoftwareSerial.begin(9600);
  SetSSerial(&DebugSoftwareSerial);
}

/// <summary>
/// Настраиваем Playstation joystick.
/// </summary>
void SetupPlaystationJoystick()
{
  // Настраиваем Playstation joystick, указываем пины и настройки в формате (Clock, Command, Attention, Data, Pressures, Rumble), возвращает int говорящий о ошибке.
  ps2Error = ps2x.config_gamepad(PS2_PIN_CLK, PS2_PIN_CMD, PS2_PIN_SEL, PS2_PIN_DAT);
  
  // Получение типа Playstation joystick.
  ps2Type = ps2x.readType();
}

void loop()
{
  CheckUart();
  
  // Мой Playstation joystick - DualShock Controller.
  ps2x.read_gamepad(false, 0);
      
  // Переменная для определения необходимо ли отправлять данные или нет.
  uint8_t sendPS2Data = 0;
            
  // Проверка, изменился ли статус кнопок.
  if (ps2x.NewButtonState())
  {
    // Обнуляем 1 массив кнопок.
    ps2S.bfirst = 0;
    
    // Обнуляем 2 массив кнопок.
    ps2S.bsecond = 0;
        
    // Необходимо отправить данные.
    sendPS2Data = 1;                 
        
    //TODO: Вынести в отдельную функцию.
    // Проверка, нажата кнопка вверх, 1 массив.
    if(ps2x.Button(PSB_PAD_UP))
    {
      MC_SET_BIT(ps2S.bfirst, 0);
    }

    //TODO: Вынести в отдельную функцию.
    // Проверка, нажата кнопка вправо, 1 массив.
    if(ps2x.Button(PSB_PAD_RIGHT))
    {
      MC_SET_BIT(ps2S.bfirst, 1);
    }

    //TODO: Вынести в отдельную функцию.
    // Проверка, нажата кнопка влево, 1 массив.
    if(ps2x.Button(PSB_PAD_LEFT))
    {
      MC_SET_BIT(ps2S.bfirst, 2);
    }

    //TODO: Вынести в отдельную функцию.
    // Проверка, нажата кнопка вниз, 1 массив.
    if(ps2x.Button(PSB_PAD_DOWN))
    {
      MC_SET_BIT(ps2S.bfirst, 3);
    }
        
    //TODO: Вынести в отдельную функцию.
    // Проверка, нажата кнопка "треугольник", 1 массив.
    if(ps2x.Button(PSB_TRIANGLE))
    {
      MC_SET_BIT(ps2S.bfirst, 4);   
    }

    //TODO: Вынести в отдельную функцию.
    // Проверка, нажата кнопка "круг", 1 массив.
    if(ps2x.Button(PSB_CIRCLE))
    {
      MC_SET_BIT(ps2S.bfirst, 5);
    }

    //TODO: Вынести в отдельную функцию.
    // Проверка, нажата кнопка "крест", 1 массив.
    if(ps2x.Button(PSB_CROSS))
    {
      MC_SET_BIT(ps2S.bfirst, 6);
    }

    //TODO: Вынести в отдельную функцию.
    // Проверка, нажата кнопка "квадрат", 1 массив.
    if(ps2x.Button(PSB_SQUARE))
    {
      MC_SET_BIT(ps2S.bfirst, 7);
    }
        
    //TODO: Вынести в отдельную функцию.
    // Проверка, нажата кнопка L1, 2 массив.
    if(ps2x.Button(PSB_L1))
    {
      MC_SET_BIT(ps2S.bsecond, 0);
    }

    //TODO: Вынести в отдельную функцию.
    // Проверка, нажата кнопка R1, 2 массив.
    if(ps2x.Button(PSB_R1))
    {
      MC_SET_BIT(ps2S.bsecond, 1);
    }
        
    //TODO: Вынести в отдельную функцию
    // Проверка, нажата кнопка L2, 2 массив.
    if(ps2x.Button(PSB_L2))
    {
      MC_SET_BIT(ps2S.bsecond, 2);
    }

    //TODO: Вынести в отдельную функцию.
    // Проверка, нажата кнопка R2, 2 массив.
    if(ps2x.Button(PSB_R2))
    {
      MC_SET_BIT(ps2S.bsecond, 3);
    }
        
    //TODO: Вынести в отдельную функцию.
    // Проверка, нажат левый джойстик, 2 массив.
    if(ps2x.Button(PSB_L3))
    {
      MC_SET_BIT(ps2S.bsecond, 4);
    }

    //TODO: Вынести в отдельную функцию.
    // Проверка, нажат правый джойстик, 2 массив.
    if(ps2x.Button(PSB_R3))
    {
      MC_SET_BIT(ps2S.bsecond, 5);
    }
        
    //TODO: Вынести в отдельную функцию.
    // Проверка, нажата кнопка Start, 2 массив.
    if(ps2x.Button(PSB_START))
    {
      MC_SET_BIT(ps2S.bsecond, 6);
    }

    //TODO: Вынести в отдельную функцию.
    // Проверка, нажата кнопка Select, 2 массив.
    if(ps2x.Button(PSB_SELECT))
    {
      MC_SET_BIT(ps2S.bsecond, 7);
    }
  }
  
  if((ps2x.Analog(PSS_LY) != ps2SPrev.ly
    || ps2x.Analog(PSS_LX) != ps2SPrev.lx
    || ps2x.Analog(PSS_RY) != ps2SPrev.ry
    || ps2x.Analog(PSS_RX) != ps2SPrev.rx) && (MC_BIT_IS_SET(ps2S.bsecond, 0) || MC_BIT_IS_SET(ps2S.bsecond, 1)))
  {
    if (sendPS2Data == 0)
    {
      // Необходимо отправить данные.
      sendPS2Data = 1;
    }

    //TODO: Вынести в отдельную функцию.
    ps2S.lx = ps2x.Analog(PSS_LX);
    ps2S.ly = ps2x.Analog(PSS_LY);
    ps2S.rx = ps2x.Analog(PSS_RX);
    ps2S.ry = ps2x.Analog(PSS_RY);
  }
  else if (sendPS2Data == 1)
  {
    //TODO: Вынести в отдельную функцию.
    ps2S.lx = PS2_JOYSTICK_DEF_VALUE;
    ps2S.ly = PS2_JOYSTICK_DEF_VALUE;
    ps2S.rx = PS2_JOYSTICK_DEF_VALUE;
    ps2S.ry = PS2_JOYSTICK_DEF_VALUE;  
  }
  
  // Если необходимо отправить данные.
  if (sendPS2Data == 1)
  {
    // Сохраняем предыдущее значение кнопок и стиков джойстика.
    ps2SPrev = ps2S;
        
    // Устанавливаем статус.
    ps2S.statuswork = sendPS2Data;
    
    //HeadCmdSend(0, 12, 1);
    HeadCmdSend(0, 8, 1);
                      
    //SerializeStruct((uint8_t*)&ps2S, 12);
    SerializeStruct((uint8_t*)&ps2S, 8);
        
    TailCmdSend();  
  }   
   
  if (rovDataS.errore == 1)
  { 
    DebugSerial->println("Return");
      
    rovDataS.errore = 0;
  }
}
//***************************** /Основные функции  **************************//
