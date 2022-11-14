#include<LiquidCrystal.h>

LiquidCrystal lcd(6,7,8,9,10,11);   //rs,e,d4,d5,d6,d7
int Vin=5;        //voltage at 5V pin of arduino
float Vout=0;     //voltage at A0 pin of arduino
float R1=5100;    //value of known resistance
float R2=0;       //value of unknown resistance
int a2d_data=0;    
float buffer=0;            
const int green = 3;
const int yellow = 4;
const int red = 5;
void setup() 
{

 pinMode (green, OUTPUT);
 pinMode (yellow, OUTPUT);
 pinMode (red, OUTPUT);
}

void loop()
{
  a2d_data=analogRead(A0);
  if(a2d_data)
  {
    buffer=a2d_data*Vin;
    Vout=(buffer)/1024.0;
    buffer=Vout/(Vin-Vout); 
    R2=R1*buffer;
 lcd.setCursor(4,0);
    lcd.print("ohm meter");

    lcd.setCursor(0,1);
    lcd.print("R (ohm) = ");
    lcd.print(R2);
    
    delay(1000);
   
    
    
  }
  if (R2 >9000) {
    digitalWrite(3, HIGH);
    digitalWrite (4, LOW);
    digitalWrite (5, LOW);}
    if (R2 < 9000 && R2 > 2000){
      digitalWrite(3, LOW);
    digitalWrite (4, HIGH);
    digitalWrite (5, LOW);}
      if (R2 <1000) {
        digitalWrite(3, LOW);
    digitalWrite (4, LOW);
    digitalWrite (5, HIGH);
    }
  }
