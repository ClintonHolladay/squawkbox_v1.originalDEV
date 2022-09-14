#include "RTClib.h"
#include <SPI.h>
#include <ModbusMaster.h>
#include <SD.h>
File myFile;
File bLog;

const int gsmRebootPin = A3;
String line1;
String line2;
String pnumber1;
String pnumber2;
const int low1 = 7; // primary low water input
const int low2 = 6; // secondary low water input
const int alarmPin = A0; // alarm circuit input(terminal 3 on Honeywell 7800 series)
int primaryCutoff;  // this is the variable used for the digitalRead of the primary cutoff pin
int counter1; // the counters are used to keep the loop from sending messages continuously while the pin status is high.
int secondaryCutoff;
int counter2;
int alarm;
int counter3;
String message;
char incomingChar;
int hplcIN = 24;
int hplcOUT = 25;
int hlpcCOMMON;
int hlpcNC;
int counter4;
String message2;
const int debounceInterval = 3000;
const long dailyInterval = 86400000;
const long twoHoursInterval = 7200000;

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
const int chipSelect = 10;

unsigned long cycleslong = 0;
unsigned long oldcycles = 0;
int cyclediff = 0;

#define MAX485_DE 3
#define MAX485_RE_NEG 2

//initiate modbus master object
ModbusMaster node;
RTC_PCF8523 rtc;

void setup() {
  pinMode(gsmRebootPin, OUTPUT);
  digitalWrite(gsmRebootPin, LOW);
  //rebootGSM();
  pinMode(53, OUTPUT);
  digitalWrite(53, HIGH);
  Serial.begin(9600);
  Serial.println(F("Serial monitor functional.  Sketch loaded: NEW_new_squawk_v3.0"));
  Serial.println(F("Booting SIM."));
  Serial1.begin(19200);



  // Give time to your GSM shield log on to network
  Serial.println(F("Sim started."));
  delay(5000);
  Serial.println(F("Bootup time complete.  Setting pinModes."));
  pinMode(low1, INPUT);
  pinMode(low2, INPUT);
  pinMode(alarmPin, INPUT);
  pinMode(hplcIN, INPUT);
  pinMode(hplcOUT, INPUT);
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  Serial.println(F("pinModes set.  Prepping Modbus module"));
  //putting these pins to zero sets it in receive mode.
  //does it need to be in receive mode?
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  node.begin(1, Serial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  Serial.println(F("Modbus module prepped.  Initializing SD and pulling variables"));

  Initialize_SDcard();
  loadContacts();

  delay(100);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  Serial.println("RTC setting");
  // following line sets the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  //rtc.adjust(DateTime(2020, 4, 21, 9, 53, 0));
  Serial.println("this is test");
  DateTime now = rtc.now();
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.println("");
  sendSMS(F("SMS systems booted successfully.  Contacts loaded successfully."));

  //this next bit sets up the cycle count on the honeywell.


  uint32_t cycleSetup; //for the honeywell 7800 series, two registers must be read for U32 format

  delay(300);
  cycleSetup = node.readHoldingRegisters (0x0006, 2);
  delay(300);
  if (cycleSetup == node.ku8MBSuccess)
  {
    delay(50);

    oldcycles = node.getResponseBuffer(cycleSetup);

    Serial.print("Cycle count is...");
    Serial.println(oldcycles);
  }
}

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
  HPLC();
  Honeywell_alarm();
  SMSRequest();
  //dailyReport();
}


void SMSRequest()
{
  if (Serial1.available() > 0)
  {
    Serial.println("Something on Serial1...");
    incomingChar = Serial1.read();
    switch (incomingChar)
    {
      case 'C':
        delay(10);
        Serial.print(incomingChar);
        incomingChar = Serial1.read();
        if (incomingChar == 'H') {
          delay(10);
          Serial.print(incomingChar);
          incomingChar = Serial1.read();
          if (incomingChar == 'E') {
            delay(10);
            Serial.print(incomingChar);
            incomingChar = Serial1.read();
            if (incomingChar == 'C') {
              delay(10);
              Serial.print(incomingChar);
              incomingChar = Serial1.read();
              if (incomingChar == 'K') {
                delay(10);
                Serial.print(incomingChar);
                incomingChar = "";
                Serial.println(F("GOOD CHECK. SMS SYSTEMS ONLINE"));
                Serial.println(F("SENDING CHECK VERIFICATION MESSAGE")) ;
                sendSMS2(F("SMS systems are online"));
                Serial.println("verification message sent");
                break;
              }
            }
          }
        }


      case 'B':
        delay(10);
        Serial.print(incomingChar);
        incomingChar = Serial1.read();
        if (incomingChar == 'E') {
          delay(10);
          Serial.print(incomingChar);
          incomingChar = Serial1.read();
          if (incomingChar == 'A') {
            delay(10);
            Serial.print(incomingChar);
            incomingChar = Serial1.read();
            if (incomingChar == 'N') {
              delay(10);
              Serial.print(incomingChar);
              incomingChar = Serial1.read();
              if (incomingChar == 'S') {
                delay(10);
                Serial.print(incomingChar);
                incomingChar = "";
                sendSMS2("this is a beans message");
                break;
              }
            }
          }
        }

      case 'S':
        delay(10);
        Serial.print(incomingChar);
        incomingChar = Serial1.read();
        if (incomingChar == 'T') {
          delay(10);
          Serial.print(incomingChar);
          incomingChar = Serial1.read();
          if (incomingChar == 'A') {
            delay(10);
            Serial.print(incomingChar);
            incomingChar = Serial1.read();
            if (incomingChar == 'T') {
              delay(10);
              Serial.print(incomingChar);
              incomingChar = Serial1.read();
              if (incomingChar == 'E') {
                delay(10);
                Serial.print(incomingChar);
                incomingChar = "";
                statusReport();
                break;
              }
            }
          }
        }

      case 'F':
        delay(10);
        Serial.print(incomingChar);
        incomingChar = Serial1.read();
        if (incomingChar == 'L') {
          delay(10);
          Serial.print(incomingChar);
          incomingChar = Serial1.read();
          if (incomingChar == 'A') {
            delay(10);
            Serial.print(incomingChar);
            incomingChar = Serial1.read();
            if (incomingChar == 'M') {
              delay(10);
              Serial.print(incomingChar);
              incomingChar = Serial1.read();
              if (incomingChar == 'E') {
                delay(10);
                Serial.print(incomingChar);
                incomingChar = "";
                sendSMSreport();
                break;
              }
            }
          }
        }

      case 'A':
        delay(10);
        Serial.print(incomingChar);
        incomingChar = Serial1.read();
        if (incomingChar == 'B') {
          delay(10);
          Serial.print(incomingChar);
          incomingChar = Serial1.read();
          if (incomingChar == 'O') {
            delay(10);
            Serial.print(incomingChar);
            incomingChar = Serial1.read();
            if (incomingChar == 'R') {
              delay(10);
              Serial.print(incomingChar);
              incomingChar = Serial1.read();
              if (incomingChar == 'T') {
                delay(10);
                Serial.print(incomingChar);
                incomingChar = "";
                sendSMS2(F("Shutting down SMS systems now.  Manual reset of SIM module required."));
                abortSMS();
                return;

              }
            }
          }
        }
    }
  }
}


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

void sendSMSreport()
{
  delay(100);
  uint16_t result;
  delay(100);
  result = node.readHoldingRegisters (0x000A , 1);
  if (result == node.ku8MBSuccess)
  {
    float fSignal = ((node.getResponseBuffer(result)) * .048);

    delay(1000);
    // REPLACE THE X's WITH THE RECIPIENT'S MOBILE NUMBER
    // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
    Serial1.println(pnumber1);
    delay(100);

    // Send the SMS text message
    Serial1.print(fSignal);
    delay(100);
    // End AT command with a ^Z, ASCII code 26
    Serial1.println((char)26);
    delay(100);
    Serial1.println();
    // Give module time to send SMS
    delay(5000);
  }
  else
  {
    sendSMS(F("request received but no data"));
  }
}

void statusReport()
{
  delay(100);
  uint16_t result2;
  delay(100);
  uint16_t checkAlarm;
  delay(100);
  uint16_t alarmNum;

  checkAlarm = node.readHoldingRegisters (0x0000, 1);
  if (checkAlarm == node.ku8MBSuccess)
  {
    alarmNum = node.getResponseBuffer(checkAlarm);

    if (alarmNum != 0)
    {
      Serial.print("The alarmNum value is: ");
      Serial.println(alarmNum);
      Serial.println("STATE request received but boiler is in alarm state");
      Serial.println("running readModbus() function");
      delay(100);
      readModbus();
      Serial.println("readModbus() function complete.  Text should have been sent");
    }

    else if (alarmNum == 0)
    {
      delay(100);
      result2 = node.readHoldingRegisters (0x0002 , 1);
      if (result2 == node.ku8MBSuccess)
      {
        int Sreport = ((node.getResponseBuffer(result2)));



        switch (Sreport)
        {

          case 6:
            sendSMS(F("Standby"));
            break;
          case 8:
            sendSMS(F("Standby Hold: Start Switch"));
            break;
          case 9:
            sendSMS(F("Standby Hold: Flame Detected"));
            break;
          case 12:
            sendSMS(F("Standby Hold: T7 Running Interlock "));
            break;
          case 13:
            sendSMS(F("Standby Hold: Airflow Switch"));
            break;
          case 14:
            sendSMS(F("Purge Hold: T19 High Fire Switch"));
            break;
          case 15:
            sendSMS(F("Purge Delay: T19 High Fire Jumpered"));
            break;
          case 16:
            sendSMS(F("Purge Hold: Test Switch"));
            break;
          case 17:
            sendSMS(F("Purge Delay: Low Fire Jumpered"));
            break;
          case 18:
            sendSMS(F("Purge Hold: Flame Detected"));
            break;
          case 19:
            sendSMS(F("Purge"));
            break;
          case 20:
            sendSMS(F("Low Fire Switch"));
            break;
          case 22:
            sendSMS(F("Purge Hold: Lockout Interlock"));
            break;
          case 25:
            sendSMS(F("Pilot Ignition"));
            break;
          case 28:
            sendSMS(F("Main Ignition"));
            break;
          case 30:
            sendSMS(F("Run"));
            break;
          case 35:
            sendSMS(F("Postpurge"));
            break;
          case 45:
            sendSMS(F("Pre-ignition"));
            break;
          default:
            sendSMS(F("Request received, but no data available"));
            break;

        }
      }
    }
  }
  else
  {
    sendSMS(F("request received but no data"));
  }
}

void sendSMS(String message1) {
  // AT command to set Serial1 to SMS mode
  Serial1.print("AT+CMGF=1\r");
  delay(100);

  // REPLACE THE X's WITH THE RECIPIENT'S MOBILE NUMBER
  // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
  Serial1.println(pnumber1);
  delay(100);

  // REPLACE WITH YOUR OWN SMS MESSAGE CONTENT
  Serial1.println(message1);
  delay(100);

  // End AT command with a ^Z, ASCII code 26
  Serial1.println((char)26);
  delay(100);
  Serial1.println();
  // Give module time to send SMS
  delay(5000);

}



void readModbus()
{
  Serial.println("In the readModbus() function now");
  delay(500);
  uint16_t result;

  result = node.readHoldingRegisters (0x0000, 1);
  if (result == node.ku8MBSuccess)
  {

    int alarmRegister = node.getResponseBuffer(result);


    switch (alarmRegister)
    {

      case 1:
        sendSMS(F("Fault Code 1: No Purge Card"));
        break;
      case 14:
        sendSMS(F("Fault code 14\r\nHigh Fire Interlock Switch failure to close during Prepurge"));
        break;
      case 15:
        sendSMS(F("Fault code 15\r\nFlame present when no flame expected"));
        break;
      case 17:
        sendSMS(F("Fault code 17\r\nMain flame failure\r\nFLame failure during RUN"));
        break;
      case 19:
        sendSMS(F("Fault code 19\r\nMain Flame Failure"));
        break;
      case 20:
        sendSMS(F("Fault code 20\r\nLow Fire Interlock switch failure"));
        break;
      case 28:
        sendSMS(F("Fault code 28\r\nPilot Flame Failure"));
        break;
      case 29:
        sendSMS(F("Fault code 29\r\nLockout Interlock\r\nCheck blower rotation.\r\nCheck air switch."));
        break;
      case 46:
        sendSMS(F("Fault code 46\r\nFlame Amplifier\r\nEnsure flame amplifier is installed properly"));
        break;
      case 47:
        sendSMS(F("Fault code 47\r\nJumpers Changed\r\nJumper configuration has been changed since startup"));
        break;
      default:
        sendSMS(F("boiler in alarm state but no Modbus data available"));
        break;
        Serial.println("End of readModbus() function");
    }
  }
  else
  {
    sendSMS(F("Boiler in alarm state but communication with boiler controller has failed"));
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

void loadContacts()
{

  if (!SD.begin(10)) {
    Serial.println(F("initialization failed!"));
    while (1);
  }
  Serial.println(F("initialization done."));

  myFile = SD.open("line1.txt");
  if (myFile) {
    Serial.println(F("email address command"));

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      char c = myFile.read();  //gets one byte from serial buffer and erases previous byte


      line1 += c; //makes the string readString
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println(F("error opening line1.txt"));
  }
  Serial.println(line1);

  myFile = SD.open("line2.txt");
  if (myFile) {
    Serial.println(F("email command 2"));

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      char c = myFile.read();  //gets one byte from serial buffer


      line2 += c;
    }

    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println(F("error opening line2.txt"));
  }
  Serial.println(line2);

  myFile = SD.open("phone1.txt");
  if (myFile) {
    Serial.println("phone number command 1");
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      char c = myFile.read();  //gets one byte from serial buffer
      pnumber1 += c;
    }
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening phonenum1.txt");
  }
  Serial.println(pnumber1);

  myFile = SD.open("phone2.txt");
  if (myFile) {
    Serial.println("phone number command 2");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      char c = myFile.read();  //gets one byte from serial buffer

      pnumber2 += c;
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening phonenum2.txt");
  }
  Serial.println(pnumber2);
  delay(100);
}

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
  }
  //this next line may not be necessary, but I think it will help prevent against false alarms on HPLC
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
      sendSMS2(F("primary low water alarm"));

      Serial.println(F("message sent or simulated"));
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
      sendSMS2("Secondary low water alarm");

      Serial.println("message sent or simulated");
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
      sendSMS2(F("High pressure alarm"));
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
      Serial.println("alarmSwitch is true");
    }
    difference3 = currentMillis - alarmTime3;

    if ( difference3 >= debounceInterval)
    {
      Serial.println("sending alarm message");
      sendSMS2(F("Boiler down on alarm."));
      delay(100);
      readModbus();
      Serial.println("message sent or simulated");
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

void sendSMS2(String message2)
{
  // AT command to set Serial1 to SMS mode
  Serial1.print("AT+CMGF=1\r");
  delay(100);

  // REPLACE THE X's WITH THE RECIPIENT'S MOBILE NUMBER
  // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
  Serial1.println("AT+CMGS=\"+17065755866\"");
  delay(100);

  // REPLACE WITH YOUR OWN SMS MESSAGE CONTENT
  Serial1.println(message2);
  delay(100);

  // End AT command with a ^Z, ASCII code 26
  Serial1.println((char)26);
  delay(100);
  Serial1.println();
  // Give module time to send SMS
  delay(5000);
}
void abortSMS()
{
  digitalWrite (gsmRebootPin, HIGH);
  delay(3000);
  digitalWrite (gsmRebootPin, LOW);
}
//  This is the function for reading run hours and run cycles.
//  The goal of this function is to be able to look for
//  short cycling of the boiler.  Short cycling will be defined as more than six cycles per hour.

void dailyReport()
{
  //  save the current time
  DateTime now = rtc.now();
  if (now.second() == 0)
    //if ((now.second() == 0) || (now.second() == 15)|| (now.second() == 30)||(now.second() == 45))
  {
    uint32_t cycleCheck; //for the honeywell 7800 series, two registers must be read for U32 format
    uint32_t runHoursCheck;
    delay(500);
    cycleCheck = node.readHoldingRegisters (0x0006, 2);
    delay(500);
    if (cycleCheck == node.ku8MBSuccess)
    {
      delay(500);

      cycleslong = node.getResponseBuffer(cycleCheck);
      Serial.println(cycleslong);
      cyclediff = cycleslong - oldcycles;

      if (cyclediff != 0)
    {
      // open the file. note that only one file can be open at a time,
      // so you have to close this one before opening another.
      bLog = SD.open("BLOG.txt", FILE_WRITE);

        // if the file is available, write to it:
        if (bLog) {
          bLog.print(now.month());
          bLog.print(',');
          bLog.print(now.day());
          bLog.print(',');
          bLog.print(now.hour()); //Store date on SD card
          bLog.print(':');
          bLog.print(now.minute());
          bLog.print(',');
          bLog.print(cyclediff);
          bLog.println(); //End of Row move to next row
          bLog.close(); //Close the file
          delay(1000);
          Serial.println("data was written to card");

          //  read Holding register for cycle count
          //  save to New Cycle Count variable
          //
          //  read Holding register for run hours
          //  save to New run hours variable
          //
          //  new cycle count - old cycle count = the cycles over the last 24 hours daily_count
          //  new run hours - old run hours = the run hours over the last 24 hours daily_hours
          //  old cycle count = new cycle count
          //  old run hours = new run hours
          //
          //
          //
          //  open the file to write things to  //File myFile = SD.open(cycleLog.txt, FILE_WRITE);
          //
          //  print the time, cycle count, and run hours into the file onto a new, comma delineated line
          //
          //
          //  num = daily_count / daily_hours
          //  if (num > 6)
          //  {
          //   you're short cycling
          //   }
        }

        else
          Serial.println("SD card writing failed");
      }
      else return;
    }
    else
      Serial.println("no Modbus data available");

  }
  else return;
}


void Initialize_SDcard()
{
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
}
