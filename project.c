// LCD Registers
sbit LCD_RS at RD2_bit;
sbit LCD_EN at RD3_bit;
sbit LCD_D4 at RD4_bit;
sbit LCD_D5 at RD5_bit;
sbit LCD_D6 at RD6_bit;
sbit LCD_D7 at RD7_bit;

sbit LCD_RS_Direction at TRISD2_bit;
sbit LCD_EN_Direction at TRISD3_bit;
sbit LCD_D4_Direction at TRISD4_bit;
sbit LCD_D5_Direction at TRISD5_bit;
sbit LCD_D6_Direction at TRISD6_bit;
sbit LCD_D7_Direction at TRISD7_bit;
// End LCD Registers

//LM35 Variables
int Temp_sensor;                  // Temperature from Sensor
int Temp;                         // Temperature read from TEMP_Sensor, used for calculations
int desiredTemp = 25;

int tempDiff = 1;
int absDiff = 1;                 // Difference of desiredTemp to Temp
int tempOp = 0;                   // 0,1 Addition or Subtraction depending on tempDiff

char Temp_text[7];                //Holds Temp value as text for LCD
char desiredTempText[7];          //Holds desiredTempText as text for LCD

//Button Variables & Functions
unsigned char old_PORTB = 0;

//Control Variables
int state = 1;                    //State controls
int clear = 0;                    //Allows clearing
int read = 0;                     //Allows Temp to read from Temp_sensor
int j = 0, sec = 0;                       //Timer Variables
int start = 0;

//Motor
int fanSpeed[5] = {0,64,128,192,255};
int fanTime[4] = {4,3,2,1};
int fanLevel = 1;                 //Default Fan Speed i.e. fanSpeed[fanLevel]
char fanText[7];                  //Holds fanLevel as text for LCD

//Process Functions
void TEMP_read();                 //Read from RA0 i.e. LM35 sensor, uses ADC, reads only once
void TEMP_desired();              //Set desired temperature
void TEMP_auto();                 //Displays the room temp, and fanLevel on auto mode
void TEMP_calculate();            //Calculate the difference between the desired Temp nd room temp

//Other Functions
void setup();                     //Setups the Ports, Special registers, LCD, and Motor
void LCD_clear();                 //Clears the LCD when called
void LED_control();
void FAN_display();

void main() {

     setup();
     
     while(1){
         switch(state){
             case 1: TEMP_read(); break;       // Read Sensor Mode
             case 2: TEMP_desired(); break;    // Set Desired Mode
             case 3:                           //Automode
                  TEMP_auto();
                  TEMP_calculate();
                  break;
             default: break;
         }
     }
}

void LCD_clear(){
  if (clear == 1){
       Lcd_cmd(_LCD_CLEAR);
       clear = 0;
   }
}

void TEMP_read(){
     Lcd_clear();

     if(read == 0){
       Temp_sensor = ADC_Read(0) * 0.489; // Read analog voltage and convert it to degree Celsius (0.489 = 500/1023)
       
       if(Temp_sensor < 18){
         Temp_sensor = 18;
       }else if (Temp_sensor > 30){
         Temp_sensor = 30;
       }else{;}
       
       Temp = Temp_sensor; //Temp ONLY reads once
     }
     
     Lcd_out(1,1,"Room Temperature:");
     IntToStr(Temp, Temp_text);
     LTrim(Temp_text);
     
     Lcd_out(2,7,Temp_text);
     Lcd_chr(2,9,223);
     Lcd_out(2,10,"C");
}

void TEMP_desired(){
  Lcd_clear();

  Lcd_out(1, 2, "Desired");
  Lcd_out(1,10, "Temp.:");

  IntToStr(desiredTemp, desiredTempText);
  LTrim(desiredTempText);

  Lcd_out(2,7, desiredTempText);
  Lcd_chr(2,9,223);
  Lcd_out(2,10,"C");
}

void TEMP_auto(){
  Lcd_clear();
  
  read++;
  
  IntToStr(Temp, Temp_text);
  LTrim(Temp_text);

  Lcd_out(1,1,"Room Temp.: ");
  Lcd_out(1,13,Temp_text);
  Lcd_chr(1,15,223);
  Lcd_out(1,16,"C");
}

void TEMP_calculate(){

     tempDiff = desiredTemp - Temp;
     if (tempDiff > 0){ // Add
        tempOp = 1;
     }else if (tempDiff < 0){ // Subtract
        tempOP = 0;
     }
     
     absDiff = fabs(tempDiff);
     
     switch(absDiff){
         case 0:
           fanLevel = 1;
           FAN_display();
           start = 0;
           break;
         default: start = 1;
           if(absDiff >= 10){ //Level 4
               fanLevel = 4;
               FAN_display();
           }else if(absDiff >= 7 && absDiff < 10){ //Level 3
               fanLevel = 3;
               FAN_display();
           }else if (absDiff >= 4 && absDiff < 7){ //Level 2
               fanLevel = 2;
               FAN_display();
           }else{ //Level 1
             fanLevel = 1;
             FAN_display();
           }
           break;
     }

     LED_control();
     
     PWM1_Set_Duty(fanSpeed[fanLevel]);

     if (start){
        if (sec == fanTime[fanLevel]){
           switch (tempOp){
                  case 0: Temp--; break;
                  case 1: Temp++; break;
                  default:break;
           }
           sec = 0; //Reset timer
        }
     }
}

void FAN_display(){
     switch(absDiff){
         case 0:
              Lcd_out(2,1, "Desired Temp!   ");
              start = 0;
              break;
         default:
              IntToStr(fanLevel, fanText);
              LTrim(fanText);
              Lcd_out(2,2, "Fan Level: ");
              Lcd_out(2,13, fanText);
              break;
     }
}

void LED_control(){
     switch(fanLevel){
         case 1:
              PORTC.F3 = 1;
              PORTC.F4 = 0;
              PORTC.F5 = 0;
              PORTC.F6 = 0;
              break;
         case 2:
              PORTC.F3 = 1;
              PORTC.F4 = 1;
              PORTC.F5 = 0;
              PORTC.F6 = 0;
              break;
         case 3:
              PORTC.F3 = 1;
              PORTC.F4 = 1;
              PORTC.F5 = 1;
              PORTC.F6 = 0;
              break;
         case 4:
              PORTC.F3 = 1;
              PORTC.F4 = 1;
              PORTC.F5 = 1;
              PORTC.F6 = 1;
              break;
         default: break;
     }
     
     if (tempDiff == 0){
        PORTC.F7 = 1;
     }
}

void ISR()iv 0x0004 ics ICS_AUTO{
  INTCON.f7 = 0;         // disable global interrupts
  INTCON.f6 = 0;         // disable peripheral interrupts

  if (INTCON.f0 == 1){
      unsigned char key_pressed;
      unsigned char new_PORTB = PORTB;
      key_pressed =  old_PORTB ^ new_PORTB;

      if(key_pressed & 0x10){ //RB4 Mode Button
          state++;
          key_pressed = 0;
          if(state == 4){
              state = 1;
          }
          clear = 1;
      }
      else if (key_pressed & 0x20){ //RB5 Increment Buttom
           key_pressed = 0;
           switch(state){
               case 1: break;
               case 2:
                    if (desiredTemp < 30)
                        desiredTemp++;
                    break;
               case 3: break;
               default: break;
           }
       }
       else if (key_pressed & 0x40){ //RB6 Decrement Buttom
           key_pressed = 0;
           switch(state){
               case 1: break;
               case 2:
                    if (desiredTemp > 16)
                        desiredTemp--;
                    break;
               case 3: break;
               default: break;
           }
       }else{;}
       INTCON.f0 = 0;

  }

  if (INTCON.f2){
     switch (start){
         case 1:
              switch (j){
                   case 75:
                        j = 0;
                        sec++;
                        break;
                   default:
                       j++;
                       break;
              }
         default:break;
     }
     INTCON.f2 = 0;          // clear timer 0 overflow interrupt flag bit
  }
  
  INTCON.f7 = 1;         // enable global interrupts
  INTCON.f6 = 1;
}

void setup(){
     TRISB = 0xFF;
     TRISC = 0x00;
     TRISD = 0x00;

     //Buttons Interrups
      INTCON.f7 = 1;         // enable global interrupts
      INTCON.f6 = 1;         // enable peripheral interrupts
      INTCON.f5 = 1;         // enable timer0 overflow interrupt
      INTCON.f3 = 1;         // enables the RB port change interrupt
      
     OPTION_REG.f5 = 0;     // set timer0 clk src to internal cycle clk
     OPTION_REG.f3 = 0;     // set prescaler to timer0 module
     OPTION_REG.f2 = 1;     // set prescaler to 1:256 (bits 2 to 0)
     OPTION_REG.f1 = 1;
     OPTION_REG.f0 = 1;

     // Configure RA0/AN0 as analog pin
     ADCON1 |= 0x0E;
     // Enable the A/D converter
     ADCON0 |= 0x01;

     old_PORTB = PORTB;     // for RB port change

     //LCD
     Lcd_Init();
     Lcd_Cmd(_LCD_TURN_ON);       //Turn on LCD
     Lcd_Cmd(_LCD_CLEAR);         // Clear display
     Lcd_Cmd(_LCD_CURSOR_OFF);    // Cursor off

     //MOTOR
     PORTC = 0;
     PORTC.F0 = 1;
     PORTC.F1 = 0;
     PORTC.F2 = 1;
     PWM1_Init(1221);
     PWM1_start();
     PWM1_Set_Duty(fanSpeed[1]);
}