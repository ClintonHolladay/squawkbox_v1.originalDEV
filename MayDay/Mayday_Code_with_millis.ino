/*This code will incorporate
  the use of the SS library, AltSoftSerial.  Doing so means
  pins 8 and 9 must be used for software serial.  */
//THIS CODE NEEDS CONTACT INFO UPDATED AND WILL NOT LOAD WITH THIS LINE PRESENT

#include <AltSoftSerial.h>
#include <SPI.h>
#include <ModbusMaster.h>

String pnumber1;
String pnumber2;
const int low1 = 4;
const int low2 = 12;
const int alarmPin = 6;
const int gsmRebootPin = 7;
const int debounceInterval = 3000;
int primaryCutoff;
int counter1;
int secondaryCutoff;
int counter2;
int alarm;
int counter3;
int hplcIN = A2;
int hplcOUT = A1;
int hlpcCOMMON;
int hlpcNC;
int counter4;
String message1;
String message2;
String message3;
char incomingChar;

unsigned long currentMillis = 0;

unsigned long difference = 0;
unsigned long difference2 = 0;
unsigned long difference3 = 0;
unsigned long difference4 = 0;

unsigned long alarmTime = 0;
unsigned long alarmTime2 = 0;
unsigned long alarmTime3 = 0;
unsigned long alarmTime4 = 0;

bool alarmSwitch = false;
bool alarmSwitch2 = false;
bool alarmSwitch3 = false;
bool alarmSwitch4 = false;

//Set up the modbus module.  It is declared as "node"

//the following lines set up the MAX485 board that is used for ttl to rs485 conversion
#define MAX485_DE 3
#define MAX485_RE_NEG 2
ModbusMaster node;
void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}
void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

// Configure software serial port for using the SIM900
AltSoftSerial SIM900;

void setup() {
  pinMode(gsmRebootPin, OUTPUT);
  digitalWrite(gsmRebootPin, LOW);
  rebootGSM();
  Serial.begin(9600);
  Serial.println("Mayday's code that uses millis() counter.  Updated 5/27/2020");
  SIM900.begin(19200);
  // Give time to your GSM shield log on to network
  delay(7000);
  pinMode(low1, INPUT);
  pinMode(low2, INPUT);
  pinMode(alarmPin, INPUT);
  pinMode(hplcIN, INPUT);
  pinMode(hplcOUT, INPUT);
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);

  //putting these pins to zero sets Modbus module to receive mode.
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  node.begin(1, Serial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  sendSMS1(F("Systems booted successfully.  Contacts loaded successfully."));
  sendSMS2(F("Systems booted successfully.  Contacts loaded successfully."));
  sendSMS3(F("Systems booted successfully.  Contacts loaded successfully."));
  Serial.println(F("Setup complete.  Entering main loop"));
}
//=================================================================//
//=================================================================//

void loop()
{
  primaryCutoff = digitalRead(low1);
  secondaryCutoff = digitalRead(low2);
  alarm = digitalRead(alarmPin);
  hlpcCOMMON = digitalRead(hplcIN);
  hlpcNC = digitalRead(hplcOUT);
  currentMillis = millis();

  resetCounters();
  primary_LW();
  secondary_LW();
  // HPLC();
  Honeywell_alarm();
  SMSRequest();
}

//-------------------------------------------------------------------------//

void resetCounters()
{
  if (primaryCutoff == LOW)
  {
    alarmSwitch = false;
    difference = 0;
    alarmTime = 0;
    counter1 = 0;

  }
  if (secondaryCutoff == LOW)
  {
    alarmSwitch2 = false;
    difference2 = 0;
    alarmTime2 = 0;
    counter2 = 0;
  }
  if (alarm == LOW)
  {
    alarmSwitch3 = false;
    counter3 = 0;
    difference3 = 0;
    alarmTime3 = 0;
  }
  if ((hlpcCOMMON == HIGH) && (hlpcNC == HIGH))
  {
    counter4 = 0;
    difference4 = 0;
    alarmSwitch4 = false;
    alarmTime4 = 0;
  }
  //this next line may not be necessary, but I think it could help prevent against false alarms on HPLC
  if (hlpcCOMMON == LOW)
  {
    counter4 = 1;
  }
}

void primary_LW()
{
  if ((primaryCutoff == HIGH) && (counter1 == 0))
  {
    if (alarmSwitch == false)
    {
      alarmTime = currentMillis;
      alarmSwitch = true;
      Serial.println("alarmSwitch is true");
    }
    difference = currentMillis - alarmTime;

    if ( difference >= debounceInterval)
    {
      Serial.println(F("Primary low water.  Sending message"));
      sendSMS1(F("primary low water alarm"));
      sendSMS2(F("primary low water alarm"));
      sendSMS3(F("primary low water alarm"));
      Serial.println("message sent or simulated");
      delay(10);
      counter1 = 1;
      difference = 0;
      alarmSwitch = false;
      alarmTime = 0;
    }
    if (difference < debounceInterval)
    {
      Serial.println(difference);
      return;
    }
  }
  else
  {

    if ((primaryCutoff == LOW) || (counter1 == 1))
    {
      alarmSwitch = false;
      difference = 0;
      alarmTime = 0;
      return;
    }
  }
}
void secondary_LW()
{
  if ((secondaryCutoff == HIGH) && (counter2 == 0))
  {
    if (alarmSwitch2 == false)
    {
      alarmTime2 = currentMillis;
      alarmSwitch2 = true;
      Serial.println("alarmSwitch2 is true");
    }
    difference2 = currentMillis - alarmTime2;

    if ( difference2 >= debounceInterval)
    {
      Serial.println(F("Secondary low water.  Sending message."));
      sendSMS1(F("Secondary low water alarm"));
      sendSMS2(F("Secondary low water alarm"));
      sendSMS3(F("Secondary low water alarm"));
      Serial.println(F("message sent or simulated"));
      delay(10);
      counter2 = 1;
      difference2 = 0;
      alarmSwitch2 = false;
      alarmTime2 = 0;
    }
    if (difference2 < debounceInterval)
    {
      Serial.println(difference2);
      return;
    }
  }
  else
  {

    if ((secondaryCutoff == LOW) || (counter2 == 1))
    {
      alarmSwitch2 = false;
      difference2 = 0;
      alarmTime2 = 0;
      return;
    }
  }
}

void HPLC()
{

  if ((hlpcCOMMON == HIGH) && (hlpcNC == LOW) && (counter4 == 0))
  {

    if (alarmSwitch4 == false)
    {
      alarmTime4 = currentMillis;
      alarmSwitch4 = true;
      Serial.println("alarmSwitch is true");
    }
    difference4 = currentMillis - alarmTime4;

    if ( difference4 >= debounceInterval)
    {
      Serial.println("Sending HPLC alarm message");
      sendSMS1(F("High pressure alarm"));
      sendSMS2(F("High pressure alarm"));
      sendSMS3(F("High pressure alarm"));
      delay(100);
      Serial.println("message sent or simulated");
      counter4 = 1;
      difference4 = 0;
      alarmSwitch4 = false;
      alarmTime4 = 0;
    }
    if (difference4 < debounceInterval)
    {
      Serial.println(difference4);
      return;
    }
  }
  else
  {
    if ((hlpcCOMMON == LOW) || (counter4 == 1) || ((hlpcCOMMON == HIGH) && hlpcNC == (HIGH)))
    {
      alarmSwitch4 = false;
      difference4 = 0;
      alarmTime4 = 0;
      return;
    }
  }
}

void Honeywell_alarm()
{
  if ((alarm == HIGH) && (counter3 == 0))
  {
    if (alarmSwitch3 == false)
    {
      alarmTime3 = currentMillis;
      alarmSwitch3 = true;
      Serial.println(F("alarmSwitch is true"));
    }
    difference3 = currentMillis - alarmTime3;

    if ( difference3 >= debounceInterval)
    {
      Serial.println("sending alarm message");
      sendSMS1(F("Boiler down on alarm."));
      sendSMS2(F("Boiler down on alarm."));
      sendSMS3(F("Boiler down on alarm."));
      delay(100);
      readModbus();
      Serial.println(F("message sent or simulated"));
      counter3 = 1;
      difference3 = 0;
      alarmSwitch3 = false;
      alarmTime3 = 0;
    }
    if (difference3 < debounceInterval)
    {
      Serial.println(difference3);
      return;
    }
  }
  else
  {

    if ((alarm == LOW) || (counter3 == 1))
    {
      alarmSwitch3 = false;
      difference3 = 0;
      alarmTime3 = 0;
      return;
    }
  }
}

void sendSMS1(String message1)
{
  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r");
  delay(100);

  // REPLACE THE X's WITH THE RECIPIENT'S MOBILE NUMBER
  // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
  SIM900.println("AT+CMGS=\"+17065755866\"");
  delay(100);

  // REPLACE WITH YOUR OWN SMS MESSAGE CONTENT
  SIM900.println(message1);
  delay(100);

  // End AT command with a ^Z, ASCII code 26
  SIM900.println((char)26);
  delay(100);
  SIM900.println();
  // Give module time to send SMS
  delay(5000);
}
void sendSMS2(String message2)
{
  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r");
  delay(100);

  // REPLACE THE X's WITH THE RECIPIENT'S MOBILE NUMBER
  // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
  SIM900.println("AT+CMGS=\"+17065755866\"");
  delay(100);

  // REPLACE WITH YOUR OWN SMS MESSAGE CONTENT
  SIM900.println(message2);
  delay(100);

  // End AT command with a ^Z, ASCII code 26
  SIM900.println((char)26);
  delay(100);
  SIM900.println();
  // Give module time to send SMS
  delay(5000);
}

void sendSMS3(String message3)
{
  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r");
  delay(100);

  // REPLACE THE X's WITH THE RECIPIENT'S MOBILE NUMBER
  // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
  SIM900.println("AT+CMGS=\"+17065755866\"");
  delay(100);

  // REPLACE WITH YOUR OWN SMS MESSAGE CONTENT
  SIM900.println(message3);
  delay(100);

  // End AT command with a ^Z, ASCII code 26
  SIM900.println((char)26);
  delay(100);
  SIM900.println();
  // Give module time to send SMS
  delay(5000);
}

void SMSRequest()
{
  if (SIM900.available() > 0) {
    incomingChar = SIM900.read();
    if (incomingChar == 'C') {
      delay(10);
      Serial.print(incomingChar);
      incomingChar = SIM900.read();
      if (incomingChar == 'H') {
        delay(10);
        Serial.print(incomingChar);
        incomingChar = SIM900.read();
        if (incomingChar == 'E') {
          delay(10);
          Serial.print(incomingChar);
          incomingChar = SIM900.read();
          if (incomingChar == 'C') {
            delay(10);
            Serial.print(incomingChar);
            incomingChar = SIM900.read();
            if (incomingChar == 'K') {
              delay(10);
              Serial.print(incomingChar);
              incomingChar = "";
              Serial.println(F("GOOD CHECK. SMS SYSTEMS ONLINE"));
              Serial.println(F("SENDING CHECK VERIFICATION MESSAGE"));
              sendSMS1(F("SMS systems are online"));
              sendSMS2(F("SMS systems are online"));
              sendSMS3(F("SMS systems are online"));
              Serial.println(F("verification message sent"));
              return;
            }
          }
        }
      }
    }

    else if (incomingChar == 'B') {
      delay(10);
      Serial.print(incomingChar);
      incomingChar = SIM900.read();
      if (incomingChar == 'E') {
        delay(10);
        Serial.print(incomingChar);
        incomingChar = SIM900.read();
        if (incomingChar == 'A') {
          delay(10);
          Serial.print(incomingChar);
          incomingChar = SIM900.read();
          if (incomingChar == 'N') {
            delay(10);
            Serial.print(incomingChar);
            incomingChar = SIM900.read();
            if (incomingChar == 'S') {
              delay(10);
              Serial.print(incomingChar);
              incomingChar = "";
              
              sendSMS2(F("this is a beans message"));
              return;
            }
          }
        }
      }
    }
    else if (incomingChar == 'A') {
      delay(10);
      Serial.print(incomingChar);
      incomingChar = SIM900.read();
      if (incomingChar == 'B') {
        delay(10);
        Serial.print(incomingChar);
        incomingChar = SIM900.read();
        if (incomingChar == 'O') {
          delay(10);
          Serial.print(incomingChar);
          incomingChar = SIM900.read();
          if (incomingChar == 'R') {
            delay(10);
            Serial.print(incomingChar);
            incomingChar = SIM900.read();
            if (incomingChar == 'T') {
              delay(10);
              Serial.print(incomingChar);
              incomingChar = "";
              sendSMS1(F("Shutting down SMS systems now.  Manual reset of SIM module required."));
              sendSMS2(F("Shutting down SMS systems now.  Manual reset of SIM module required."));
              sendSMS3(F("Shutting down SMS systems now.  Manual reset of SIM module required."));
              abortSMS();
              return;
            }
          }
        }
      }
    }
  }
  incomingChar = "";
  return;
}

void readModbus()
{
  delay(500);
  uint16_t result;
  result = node.readHoldingRegisters (0x0000, 1);
  if (result == node.ku8MBSuccess)
  {
    node.getResponseBuffer(result);
    if (node.getResponseBuffer(result)  == 1)
    {
      sendSMS1(F("Fault code 1\r\nNo Timer Card\r\nCheck timer card is seated properly.\r\nIf fault persists, replace timer card."));
      delay(100);
      sendSMS2(F("Fault code 1\r\nNo Timer Card\r\nCheck timer card is seated properly.\r\nIf fault persists, replace timer card."));
      delay(100);
      sendSMS3(F("Fault code 1\r\nNo Timer Card\r\nCheck timer card is seated properly.\r\nIf fault persists, replace timer card."));
      delay(100);
    }
    if (node.getResponseBuffer(result) == 15)
    {
      sendSMS1(F("Fault code 15\r\nFlame Present When No Flame Is Expected\r\nCheck flame\r\nCheck flame sensor and amplifier"));
      delay(100);
      sendSMS2(F("Fault code 15\r\nFlame Present When No Flame Is Expected\r\nCheck flame\r\nCheck flame sensor and amplifier"));
      delay(100);
      sendSMS3(F("Fault code 15\r\nFlame Present When No Flame Is Expected\r\nCheck flame\r\nCheck flame sensor and amplifier"));
      delay(100);
    }
    if (node.getResponseBuffer(result)  == 17)
    {
      sendSMS1(F("Fault code 17\r\nMain flame failure\r\nFLame failure during RUN"));
      sendSMS2(F("Fault code 17\r\nMain flame failure\r\nFLame failure during RUN"));
      delay(100);
      sendSMS3(F("Fault code 17\r\nMain flame failure\r\nFLame failure during RUN"));
      delay(100);
    }
    if (node.getResponseBuffer(result) == 19)
    {
      sendSMS1(F("Fault code 19\r\nMain flame ignition failure"));
      delay(100);
      sendSMS2(F("Fault code 19\r\nMain flame ignition failure"));
      delay(100);
      sendSMS3(F("Fault code 19\r\nMain flame ignition failure"));
      delay(100);
    }
    if (node.getResponseBuffer(result)  == 28)
    {
      sendSMS1(F("Fault code 28\r\nPilot Flame Failure"));
      delay(100);
      sendSMS2(F("Fault code 28\r\nPilot Flame Failure"));
      delay(100);
      sendSMS3(F("Fault code 28\r\nPilot Flame Failure"));
      delay(100);
    }
    if (node.getResponseBuffer(result)  == 29)
    {
      sendSMS1(F("Fault code 29\r\nLockout Interlock\r\nCheck blower rotation.\r\nCheck air switch."));
      delay(1000);
      sendSMS2(F("Fault code 29\r\nLockout Interlock\r\nCheck blower rotation.\r\nCheck air switch."));
      delay(100);
      sendSMS3(F("Fault code 29\r\nLockout Interlock\r\nCheck blower rotation.\r\nCheck air switch."));
      delay(100);
    }

    if (node.getResponseBuffer(result)  == 46)
    {
      sendSMS1(F("Fault code 46\r\nFlame Amplifier\r\nEnsure flame amplifier is installed properly"));
      delay(1000 );
      sendSMS2(F("Fault code 46\r\nFlame Amplifier\r\nEnsure flame amplifier is installed properly"));
      delay(100);
      sendSMS3(F("Fault code 46\r\nFlame Amplifier\r\nEnsure flame amplifier is installed properly"));
      delay(100);
    }
  }
  else
  {
    Serial.println(F("Alarm state active, but no info from Honeywell available."));
    sendSMS1(F("Alarm state active, but no details available."));
    sendSMS2(F("Alarm state active, but no details available."));
    sendSMS2(F("Alarm state active, but no details available."));
  }
}

void rebootGSM()
{
  digitalWrite (gsmRebootPin, HIGH);
  delay(3000);
  digitalWrite (gsmRebootPin, LOW);
  delay(3000);
  digitalWrite (gsmRebootPin, HIGH);
  delay(3000);
  digitalWrite (gsmRebootPin, LOW);
}

void abortSMS()
{
  digitalWrite (gsmRebootPin, HIGH);
  delay(3000);
  digitalWrite (gsmRebootPin, LOW);
}
