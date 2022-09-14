//SEND SMS MESSAGE WITH SIM900


//set variables.  We're using a primary low water, secondary low water, and an alarm circuit from the controller
String pnumber1;
String pnumber2;
const int low1 = 4; // primary low water input
const int low2 = 12; // secondary low water input
const int alarmPin = 6; // alarm circuit input(terminal 3 on Honeywell 7800 series)
int primaryCutoff;  // this is the variable used for the digitalRead of the primary cutoff pin
int counter1; // the counters are used to keep the loop from running continuously while the pin status is high.
int secondaryCutoff;
int counter2;
int alarm;
int counter3;
//int hplcIN;
//int hplcOUT;
int counter4;
String message;
String message2;
String message3;
const int ledPin1 = 9; //this was used for troubleshooting.  I'll leave it here for future troubleshooting

#include <SoftwareSerial.h>
#include <ModbusMaster.h>

#include <SPI.h>


//the following lines set up the MAX485 board that is used for ttl to rs485 conversion
#define MAX485_DE 3
#define MAX485_RE_NEG 2

//initiate modbus master object
ModbusMaster node;


void preTransmission() // not sure what the purpose of this is
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission() // also not sure what this does
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}


// Configure software serial port for using the SIM900
SoftwareSerial SIM900(7, 8);

void setup() {
  SIM900.begin(19200);
  // Give time to your GSM shield log on to network
  delay(10000);
  pinMode(low1, INPUT);
  pinMode(low2, INPUT);
  pinMode(alarmPin, INPUT);
  //pinMode(hplcIN, INPUT);
  // pinMode(hplcOUT, INPUT);
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  //putting these pins to zero sets it in receive mode.
  //does it need to be in receive mode?
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  Serial.begin(9600 );
  node.begin(1, Serial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
 
  sendSMS(F("Systems booted successfully.  Contacts loaded successfully."));
}

void loop() {


  int primaryCutoff = digitalRead(low1);
  int secondaryCutoff = digitalRead(low2);
  int alarm = digitalRead(alarmPin);
  //int upstream = digitalRead(hplcIN);
  // int  downstream = digitalRead(hplcOUT);

  //read the inputs and reset counters to 0 if LOW
  //if HIGH, counters remain at 1, and functions will not run

  if (primaryCutoff == LOW)
  {
    counter1 = 0;
  }
  if (secondaryCutoff == LOW)
  {
    counter2 = 0;
  }
  if (alarm == LOW)
  {
    counter3 = 0;
  }
  //  if ((upstream == HIGH) && (downstream == HIGH))
  // {
  //  counter4 = 0;
  // }
  //this next line may not be necessary, but I think it will help prevent against false alarms on HPLC
  //if (upstream == LOW)
  //{
  // counter4 = 1;
  // }
  if ((primaryCutoff == HIGH) && (counter1 == 0))
  {
    delay(4000);
    if ((primaryCutoff == HIGH) && (counter1 == 0))
    {
      sendSMS(F("primary low water alarm"));
      delay(3000);
      sendSMS2(F("primary low water alarm"));
      delay(3000);
      sendSMS3(F("primary low water alarm"));
      counter1 = 1;
    }
    else if ((primaryCutoff == LOW) || (counter1 == 1))
    {
      return;
    }
  }
  if ((secondaryCutoff == HIGH) && (counter2 == 0))
  {
    delay(4000);
    if ((secondaryCutoff == HIGH) && (counter2 == 0))
    {
      sendSMS(F("secondary low water alarm"));
      delay(3000);
      sendSMS2(F("secondary low water alarm"));
      delay(3000);
      sendSMS3(F("secondary low water alarm"));
      counter2 = 1;
    }
    else if ((secondaryCutoff == LOW) || (counter2 == 1))
    {
      return;
    }
  }
  /*if ((upstream == HIGH) && (downstream == LOW) && (counter4 = 0))
    {
    delay(3000);
    if ((upstream == HIGH) && (downstream == LOW) && (counter4 = 0))
    {
      sendSMS("HIGH PRESSURE ALARM");
      counter4 = 1;
    }
    }*/
  if ((alarm == HIGH) && (counter3 == 0))
  {
    delay(1000);
    if ((alarm == HIGH) && (counter3 == 0))
    {
      sendSMS(F("Boiler down on alarm."));
      readModbus();
      counter3 = 1;
    }
    else if ((alarm == LOW) || (counter3 == 1))
    {
      return;
    }
  }
}

void sendSMS(String message) {
  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r");
  delay(100);

  // REPLACE THE X's WITH THE RECIPIENT'S MOBILE NUMBER
  // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
  SIM900.println("AT + CMGS = \"+13365081995\"");
  delay(100);

  // REPLACE WITH YOUR OWN SMS MESSAGE CONTENT
  SIM900.println(message);
  delay(100);

  // End AT command with a ^Z, ASCII code 26
  SIM900.println((char)26);
  delay(100);
  SIM900.println();
  // Give module time to send SMS
  delay(5000);
}
void sendSMS2(String message2) {
  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r");
  delay(100);

  // REPLACE THE X's WITH THE RECIPIENT'S MOBILE NUMBER
  // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
  SIM900.println("AT + CMGS = \"+17065755866\"");
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
void sendSMS3(String message3) {
  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r");
  delay(100);

  // REPLACE THE X's WITH THE RECIPIENT'S MOBILE NUMBER
  // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
  SIM900.println("AT + CMGS = \"+16152953975\"");
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




void readModbus()
{
  delay(500);
  uint16_t result;

  result = node.readHoldingRegisters (0x0000, 1);
  if (result == node.ku8MBSuccess)
  {
    digitalWrite(ledPin1, HIGH);
    node.getResponseBuffer(result);

    if (node.getResponseBuffer(result)  == 1)
    {

      sendSMS(F("Fault code 1\r\nNo Timer Card\r\nCheck timer card is seated properly.\r\nIf fault persists, replace timer card."));

      delay(3000);
      sendSMS2(F("Fault code 1\r\nNo Timer Card\r\nCheck timer card is seated properly.\r\nIf fault persists, replace timer card."));
      delay(3000);
      sendSMS3(F("Fault code 1\r\nNo Timer Card\r\nCheck timer card is seated properly.\r\nIf fault persists, replace timer card."));

      delay(100);

    }

    if (node.getResponseBuffer(result) == 15)
    {
      sendSMS(F("Fault code 15\r\nFlame Present When No Flame Is Expected\r\nCheck flame\r\nCheck flame sensor and amplifier"));
      delay(3000);
      sendSMS2(F("Fault code 15\r\nFlame Present When No Flame Is Expected\r\nCheck flame\r\nCheck flame sensor and amplifier"));
      delay(3000);
      sendSMS3(F("Fault code 15\r\nFlame Present When No Flame Is Expected\r\nCheck flame\r\nCheck flame sensor and amplifier"));
      delay(100);
    }
    if (node.getResponseBuffer(result) == 19)
    {
      sendSMS(F("Fault code 19\r\nMain flame ignition failure"));
      delay(3000);
      sendSMS2(F("Fault code 19\r\nMain flame ignition failure"));
      delay(3000);
      sendSMS3(F("Fault code 19\r\nMain flame ignition failure"));
      delay(100);
    }

    if (node.getResponseBuffer(result)  == 28)
    {

      sendSMS(F("Fault code 28\r\nPilot Flame Failure"));

      delay(3000);
      sendSMS2(F("Fault code 28\r\nPilot Flame Failure"));
      delay(3000);
      sendSMS3(F("Fault code 28\r\nPilot Flame Failure"));

      delay(100);
    }

    if (node.getResponseBuffer(result)  == 29)
    {

      sendSMS(F("Fault code 29\r\nLockout Interlock\r\nCheck blower rotation.\r\nCheck air switch."));

      delay(3000);
      sendSMS2(F("Fault code 29\r\nLockout Interlock\r\nCheck blower rotation.\r\nCheck air switch."));
      delay(3000);
      sendSMS3(F("Fault code 29\r\nLockout Interlock\r\nCheck blower rotation.\r\nCheck air switch."));

      delay(100);

    }

    if (node.getResponseBuffer(result)  == 46)
    {

      sendSMS(F("Fault code 46\r\nFlame Amplifier\r\nEnsure flame amplifier is installed properly"));

      delay(3000 );
      sendSMS2(F("Fault code 46\r\nFlame Amplifier\r\nEnsure flame amplifier is installed properly"));
      delay(3000 );
      sendSMS3(F("Fault code 46\r\nFlame Amplifier\r\nEnsure flame amplifier is installed properly"));

      delay(100);
    }

  }
}
