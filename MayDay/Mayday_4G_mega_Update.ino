#include <ModbusMaster.h>

const int low1 = 4;
const int low2 = 6;
const int alarmPin = A5;
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
char URLhead[] = "AT+HTTPPARA=\"URL\",\"http://relay-post-8447.twil.io/secondary-low-water?";
char URLfrom[] = "From=%2b19049808059&";
char URLtoMe[] = "To=%2b17065755866&";
char URLto2[] = "To=%2b17065755866&";
char URLto3[] = "To=%2b17065755866&";
char URLbody[] = "Body=Setup%20Complete\"\r";
char LWbody[] = "Body=Low%20Water\"\r";
char LW2body[] = "Body=Low%20Water2\"\r";
char REPbody[] = "Body=routine%20timer\"\r";
char CHECKbody[] = "Body=good%20check\"\r";
char HLPCbody[] = "Body=High%20Pressure%20Alarm\"\r";
char BCbody[] = "Body=Boiler%20down%20BC%20alarm\"\r";
unsigned char data = 0;
char incomingChar;

unsigned long currentMillis = 0;
unsigned long difference = 0;
unsigned long difference2 = 0;
unsigned long difference3 = 0;
unsigned long difference4 = 0;
unsigned long difference5 = 0;
unsigned long fifmintimer = 900000;
unsigned long fivmintimer = 300000;
unsigned long dailytimer = 86400000;
unsigned long msgtimer1 = 0;

unsigned long alarmTime = 0;
unsigned long alarmTime2 = 0;
unsigned long alarmTime3 = 0;
unsigned long alarmTime4 = 0;

bool alarmSwitch = false;
bool alarmSwitch2 = false;
bool alarmSwitch3 = false;
bool alarmSwitch4 = false;
bool msgswitch = false;
#define MAX485_DE 3
#define MAX485_RE_NEG 2
ModbusMaster node;


void setup() {


  Serial.begin(9600);
  Serial1.begin(19200);
  // Give time to your GSM shield log on to network
  delay(10000);
  pinMode(low1, INPUT);
  pinMode(low2, INPUT);
  pinMode(alarmPin, INPUT);
  pinMode(hplcIN, INPUT);
  pinMode(hplcOUT, INPUT);

  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);

  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  node.begin(1, Serial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);


  //PUT SIM MODULE WAKEUP HERE
  Serial1.print("AT\r"); //Manufacturer identification
  getResponse();
  Serial1.print("AT\r"); //Manufacturer identification
  getResponse();
  Serial1.print("AT\r"); //Manufacturer identification
  getResponse();
  Serial1.print("AT\r"); //Manufacturer identification
  getResponse();
  Serial1.print("AT\r"); //Manufacturer identification
  getResponse();
  //SIM MODULE SETUP---
  Serial1.print("AT+CGDCONT=1\"IP\",\"super\"\r");
  delay(100);
  getResponse();
  Serial1.print("AT+COPS=1,2,\"310410\"\r");
  delay(5000);
  getResponse();
  Serial1.print("AT+SAPBR=3,1,\"APN\",\"super\"\r");
  delay(300);
  getResponse();
  Serial1.print("AT+SAPBR=1,1\r");
  delay(2000);
  getResponse();
  Serial1.print("AT+CMGD=0,4\r");
  delay(100);
  getResponse();
  Serial1.print("AT+CMGF=1\r");
  delay(100);
  getResponse();
  Serial1.print("AT+CNMI=2,2,0,0,0\r");
  getResponse();
  //PUT TEST MESSAGE HERE
  sendSMS(URLhead, URLtoMe, URLfrom, URLbody);
  sendSMS(URLhead, URLto2, URLfrom, URLbody);
  sendSMS(URLhead, URLto3, URLfrom, URLbody);
  Serial.println(F("Setup complete. Entering main loop"));
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
  Honeywell_alarm();
  //HPLC();
  timedmsg();
  SMSRequest();
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
      sendSMS(URLhead, URLtoMe, URLfrom, LWbody);
      sendSMS(URLhead, URLto2, URLfrom, LWbody);
      sendSMS(URLhead, URLto3, URLfrom, LWbody);
      //sendSMS("AT+HTTPPARA=\"URL\",\"http://relay-post-8447.twil.io/secondary-low-water?From=%2b19049808059&To=%2b17065755866&Body=Primary%20Low%20Water\"\r");
      //sendSMS("AT+HTTPPARA=\"URL\",\"http://relay-post-8447.twil.io/secondary-low-water?From=%2b19049808059&To=%2b16158122833&Body=Primary%20Low%20Water\"\r");

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
      sendSMS(URLhead, URLtoMe, URLfrom, LW2body);
      sendSMS(URLhead, URLto2, URLfrom, LW2body);
      sendSMS(URLhead, URLto3, URLfrom, LW2body);
      //sendSMS("AT+HTTPPARA=\"URL\",\"http://relay-post-8447.twil.io/secondary-low-water?From=%2b19049808059&To=%2b17065755866&Body=Secondary%20Low%20Water\"\r");
      //sendSMS("AT+HTTPPARA=\"URL\",\"http://relay-post-8447.twil.io/secondary-low-water?From=%2b19049808059&To=%2b16158122833&Body=Secondary%20Low%20Water\"\r");
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
      sendSMS(URLhead, URLtoMe, URLfrom, BCbody);
      sendSMS(URLhead, URLto2, URLfrom, BCbody);
      sendSMS(URLhead, URLto3, URLfrom, BCbody);

      delay(100);

//      readModbus();
      Serial.println("messages sent or simulated");
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
      sendSMS(URLhead, URLtoMe, URLfrom, HLPCbody);
      sendSMS(URLhead, URLto2, URLfrom, HLPCbody);
      sendSMS(URLhead, URLto3, URLfrom, HLPCbody);

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

void sendSMS(char pt1[], char pt2[], char pt3[], char pt4[])
{

  char finalURL[150] = "";

  strcpy(finalURL, pt1);
  strcat(finalURL, pt2);
  strcat(finalURL, pt3);
  strcat(finalURL, pt4);
  Serial1.print("AT+SAPBR=3,1,\"APN\",\"super\"\r");
  delay(300);
  getResponse();
  Serial1.print("AT+SAPBR=1,1\r");
  delay(1000);
  getResponse();

  Serial1.print("AT+HTTPINIT\r");
  delay(100);
  getResponse();
  Serial1.print("AT+HTTPPARA=\"CID\",1\r");
  delay(100);
  getResponse();
  Serial1.print(finalURL);
  delay(100);
  getResponse();
  Serial1.print("AT+HTTPACTION=1\r");
  delay(4000);
  getResponse();
  Serial1.print("AT+HTTPTERM\r");
  delay(100);
  getResponse();
}
void getResponse()
{
  if (Serial1.available())
  {
    while (Serial1.available())
    {
      data = Serial1.read();
      Serial.write(data);
    }
    data = 0;
  }
}

void timedmsg()
{

  if (msgswitch == false)
  {
    msgtimer1 = currentMillis;
    msgswitch = true;

  }
  difference5 = currentMillis - msgtimer1;

  if (difference5 >= dailytimer)
  {
    sendSMS(URLhead, URLtoMe, URLfrom, REPbody);
    difference5 = 0;
    msgswitch = false;
    msgtimer1 = 0;
  }
}


void SMSRequest()
{

  delay(100);
  if (Serial1.available() > 0) {

    incomingChar = Serial1.read();
    Serial.print(incomingChar);
    if (incomingChar == 'C') {
      delay(100);
      Serial.print(incomingChar);
      incomingChar = Serial1.read();
      if (incomingChar == 'H') {
        delay(100);
        Serial.print(incomingChar);
        incomingChar = Serial1.read();
        if (incomingChar == 'E') {
          delay(100);
          Serial.print(incomingChar);
          incomingChar = Serial1.read();
          if (incomingChar == 'C') {
            delay(100);
            Serial.print(incomingChar);
            incomingChar = Serial1.read();
            if (incomingChar == 'K') {
              delay(100);
              Serial.print(incomingChar);
              incomingChar = "";
              Serial.println(F("GOOD CHECK. SMS SYSTEMS ONLINE"));
              Serial.println(F("SENDING CHECK VERIFICATION MESSAGE")) ;
              sendSMS(URLhead, URLtoMe, URLfrom, CHECKbody);
              Serial.println("verification message sent");
              Serial1.print("AT+CMGD=0,4\r");
              delay(100);
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
  Serial.println("In the readModbus() function now");
  delay(300);
  uint16_t result;

  result = node.readHoldingRegisters (0x0000, 1);
  if (result == node.ku8MBSuccess)
  {

    int alarmRegister = node.getResponseBuffer(result);
    Serial.print("Register response:  ");
    Serial.println(alarmRegister);

    switch (alarmRegister)
    {

      case 1:
        sendSMS(URLhead, URLtoMe, URLfrom, "Body=Code1%20No%20Purge%20Card\"\r" );
        break;
      case 15:
        sendSMS(URLhead, URLtoMe, URLfrom, "Body=Code15%20Unexpected%20Flame\"\r" );
        break;
      case 17:
        sendSMS(URLhead, URLtoMe, URLfrom, "Body=Code17%20Main%20Flame\"\r" );
        break;
      case 19:
        sendSMS(URLhead, URLtoMe, URLfrom, "Body=Code19%20Main%20Ign\"\r" );
        break;
      case 28:
        sendSMS(URLhead, URLtoMe, URLfrom, "Body=Code28%20Pilot%20Failure\"\r" );
        break;
      case 29:
        sendSMS(URLhead, URLtoMe, URLfrom, "Body=Code29%20Interlock\"\r" );

      default:
        sendSMS(URLhead, URLtoMe, URLfrom, "Body=Check%20fault%20code\"\r" );
        break;
    }
  }
  else
  {
    sendSMS(URLhead, URLtoMe, URLfrom, "Body=Modbus%20Com%20Fail\"\r");
  }
}

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
