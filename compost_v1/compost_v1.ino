
#include <Wire.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <LiquidCrystal_I2C.h>  // Includes the LiquidCrystal_I2C library

/*
 * Global Variables
 */
  #define lcdAddress 0x3F 
  //#define lcdAddress 0x27 //if using PCF8574T backpack
  // LCD I2C Address
 
  #define TMP  A0
  #define REFVOLT 5 //3.3 or 5 dependent on what you want to use as a reference
  #define BUTTON 2 //button to turn on display
  
  double temp = 0.00;
  unsigned long currentMillis = 0;
  unsigned long previousMillis = 0;
  unsigned int interval = 10000; //interval of 30 seconds
   
LiquidCrystal_I2C lcd( lcdAddress ); //import I2C LCD library
 
void setup() 
{
  Serial.begin(9600);
  //set clock registers
  cli(); //disable global interrupts
  TCCR1A = 0; //set TCCR1A register to 0
  TCCR1B = 0;
  

  //set compare register
 
  OCR1A = 0xFFFF; //max time between readings ~4s
  OCR1B = 0x00;
  
  //turn on CTC mode
  TCCR1B |= (1 << WGM12);
  
  
  //set prescaler of 1024
  TCCR1B |= (1 << CS10); 
  TCCR1B |= (1 << CS12);
  
  
  //enable timer1 overflow
  TIMSK1 = (1 << OCIE1A);
  

  //enable interrupts
  sei();

//initialize all LCD functions
  lcd.begin(16,2);              // initialize the lcd as a 16 characters by 2 row display
  lcd.setBacklight(HIGH);       // Turn on the LCD's backlight 

pinMode(BUTTON, INPUT); //button is now input

  lcd.print("Compost Smarter"); //initialize message
  delay(5000); //hold message for 5 seconds
  lcd.clear(); //clear message
}

void loop() 
{

 currentMillis = millis();
 lcd.off();
 
 if(digitalRead(BUTTON))
 {
  while(currentMillis - previousMillis < interval)//during interval
  {
    previousMillis = currentMillis;
    lcd.on();
    temp = tempMeasure();
    String LCDmessage = message(temp);
    lcd.print(temp);
    lcd.setCursor(0,1);
    lcd.print(LCDmessage);
    delay(interval);
    break;
  }
  lcd.clear();
  
 }
 previousMillis = currentMillis;
}

/********************************************************************************
 * function sends string message telling user if compost is effectively working.
 ********************************************************************************/
String message(double t)
{
  String message;

  if ( t < 130)
  {
    message = "compost too cold";
  }
  else if (t >= 130 && t <= 150) //optimal temperature for compost to work at
  {
    message = "feelin' fine";
  }
  else
  {
    message = "might be on fire";
  }
  
  return message;
}

/*********************************************************************************
 ******************ISR calls temperature function to update tempreadout***********
 *********************************************************************************/
ISR(TIMER1_COMPA_vect)
{
  tempMeasure();
}

//takes temperature reading using tmp36
double tempMeasure()
{
 double temp;
 double ftemp = 0.00;
 int reading = analogRead(TMP); //take voltage reading
 double voltage = reading * REFVOLT;//take voltage reading and multiply by aref voltage

 voltage /= 1024.0; //divide reading by 1024 to map (10 bit)

 temp = (voltage - 0.5) * 100; //convert 10mV per degree with a 500 mV offset
 ftemp=(temp * 1.8 + 32);
 Serial.println(ftemp);
 return ftemp;//return temperature
}//end



