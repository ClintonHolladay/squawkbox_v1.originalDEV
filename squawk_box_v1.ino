#include <SD.h>
#include <ModbusMaster.h>
File myFile;
//The following const int pins are all pre-run in the PCB:
const int low1 = 4;
const int low2 = 5;
const int alarmPin = 6;   //honeywell alarm terminal (terminal 3 on honeywell)
const int hplcIN = 14;
const int hplcOUT = 15;
const int MAX485_DE = 3;  //to modbus module
const int MAX485_RE_NEG = 2;   //to modbus module
const int SIMpin = A3;  // this pin is routed to SIM pin 12 for boot (DF Robot SIM7000A module)


const int debounceInterval = 3000;  //to prevent false alarms from electrical noise.
//Setting this debounce too high will prevent the annunciation of instantaneous alarms like a bouncing LWCO.
//NEEDS TO BE MADE CHANGEABLE ON SD CARD

//declare state-reading variables
int primaryCutoff;
int plwcCounter;
int secondaryCutoff;
int slwcCounter;
int alarm;
int alarmCounter;
int hlpcCOMMON;
int hlpcNC;
int hlpcCounter;

char SetCombody[] = "Body=Setup%20Complete\"\r";
char LWbody[] = "Body=Low%20Water\"\r";
char LW2body[] = "Body=Low%20Water2\"\r";
char REPbody[] = "Body=routine%20timer\"\r";
char HLPCbody[] = "Body=High%20Pressure%20Alarm\"\r";
char CHECKbody[] = "Body=good%20check\"\r";
char BCbody[] = "Body=Boiler%20Down\"\r";
char fault1[] = "Body=Fault%20Code1%20No%20Purge%20Card\"\r";

String URLheader = "";
String conFrom1 = "";
String conFrom2 = "";
String conTo1 = "";
String conTo2 = "";
String conTo3 = "";

char contactFromArray1[25];
char contactFromArray2[25];
char contactToArray1[25];
char contactToArray2[25];
char contactToArray3[25];
char urlHeaderArray[100];

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

ModbusMaster node;


void setup() {

  Serial.begin(9600);
  Serial1.begin(19200);
  Serial.println(F("This is squawkbox v1.0 sketch."));

  pinMode(low1, INPUT);
  pinMode(low2, INPUT);
  pinMode(alarmPin, INPUT);
  pinMode(hplcIN, INPUT);
  pinMode(hplcOUT, INPUT);
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  pinMode(SIMpin, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  node.begin(1, Serial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  SIMboot();

  // Give time for the SIM module to turn on and wake up

  loadContacts(); //run the SD card function.

  Serial.println(F("Contacts Loaded.  Booting SIM module.  Initiating wakeup sequence..."));
  delay(2000);
  initiateSim();
}
void loop()
{
  primaryCutoff = digitalRead(low1);
  secondaryCutoff = digitalRead(low2);
  alarm = digitalRead(alarmPin);
  hlpcCOMMON = digitalRead(hplcIN);
  hlpcNC = digitalRead(hplcOUT);
  currentMillis = millis();

  primary_LW();
  secondary_LW();
  Honeywell_alarm();
  HPLC();
  timedmsg();
  SMSRequest();
  //zreadModbus();
  //delay(300);
}


void primary_LW()
{
  if ((primaryCutoff == HIGH) && (plwcCounter == 0))
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
      sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, LWbody);
      //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, LWbody);
      //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, LWbody);

      Serial.println(F("message sent or simulated"));
      delay(10);
      plwcCounter = 1;
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

    if (primaryCutoff == LOW)
    {
      alarmSwitch = false;
      difference = 0;
      alarmTime = 0;
      plwcCounter = 0;
      return;
    }
  }
}

void secondary_LW()
{
  if ((secondaryCutoff == HIGH) && (slwcCounter == 0))
  {
    if (alarmSwitch2 == false)
    {
      alarmTime2 = currentMillis;
      alarmSwitch2 = true;
      Serial.println(F("alarmSwitch2 is true"));
    }
    difference2 = currentMillis - alarmTime2;

    if ( difference2 >= debounceInterval)
    {
      Serial.println(F("Secondary low water.  Sending message."));
      sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, LW2body);
      //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, LW2body);
      //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, LW2body);
      Serial.println(F("message sent or simulated"));
      delay(10);
      slwcCounter = 1;
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

    if (secondaryCutoff == LOW)
    {
      alarmSwitch2 = false;
      difference2 = 0;
      alarmTime2 = 0;
      slwcCounter = 0;
      return;
    }
  }
}


void Honeywell_alarm()
{
  if ((alarm == HIGH) && (alarmCounter == 0))
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
      Serial.println(F("sending alarm message"));
      sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, BCbody);
      //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, BCbody);
      //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, BCbody);
      delay(100);
      Serial.println(F("about to enter modbus reading function..."));
      readModbus();
      Serial.println(F("message sent or simulated"));
      alarmCounter = 1;
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

    if (alarm == LOW)
    {
      alarmSwitch3 = false;
      difference3 = 0;
      alarmTime3 = 0;
      alarmCounter = 0;
      return;
    }
  }
}

void HPLC()
{

  if ((hlpcCOMMON == HIGH) && (hlpcNC == LOW) && (hlpcCounter == 0))
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
      sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, HLPCbody);
      //sendSMS(urlHeaderArray, contactToArray2, contactFromArray1, HLPCbody);
      //sendSMS(urlHeaderArray, contactToArray3, contactFromArray1, HLPCbody);
      delay(100);
      Serial.println(F("message sent or simulated"));
      hlpcCounter = 1;
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
    if (hlpcNC == (HIGH))
    {
      alarmSwitch4 = false;
      difference4 = 0;
      alarmTime4 = 0;
      hlpcCounter = 0;
      return;
    }
  }
}

void sendSMS(char pt1[], char pt2[], char pt3[], char pt4[])
{

  char finalURL[250] = "";

  strcpy(finalURL, pt1);
  strcat(finalURL, pt2);
  strcat(finalURL, pt3);
  strcat(finalURL, pt4);
  delay(500);
  Serial.println(finalURL);
  delay(20);
  Serial1.print("AT+HTTPTERM\r");
  delay(1000);
  getResponse();
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
  Serial1.println(finalURL);
  delay(100);
  getResponse();
  Serial1.print("AT+HTTPACTION=1\r");
  delay(5000);
  getResponse();
}
void getResponse()
{
unsigned char response_buffer = 0;
  if (Serial1.available())
  {
    while (Serial1.available())
    {
      response_buffer = Serial1.read();
      Serial.write(response_buffer);
    }
    response_buffer = 0;
  }
  delay(500);
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
    sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, REPbody);
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
              //sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, CHECKbody);
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



void loadContacts()
{

  if (!SD.begin(10)) {
    Serial.println(F("initialization failed!"));
    while (1);
  }
  Serial.println(F("initialization done."));

  //---------------------------------------------//
  //------------------load "from" number.  This is the number alert messages will apear to be from-------------//

  myFile = SD.open("from1.txt");
  if (myFile) {
    Serial.println("pull phone number 1 from SD");
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      char c = myFile.read();  //gets one byte from serial buffer
      conFrom1 += c;
    }
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening from1.txt");
  }
  //convert the String into a character array
  conFrom1.toCharArray(contactFromArray1, 25);
  Serial.print(F("The first phone number FROM String is "));
  Serial.println(conFrom1);
  Serial.print(F("The first phone number FROM char array is "));
  Serial.println(contactFromArray1);

  //---------------------------------------------//
  //------------------load "from2" number.  This is the number alert messages will apear to be from-------------//

  myFile = SD.open("from2.txt");
  if (myFile) {
    Serial.println("loading second from number");
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      char c = myFile.read();  //gets one byte from serial buffer
      conFrom2 += c;
    }
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening from2.txt");
  }
  //convert the String into a character array
  conFrom2.toCharArray(contactFromArray2, 25);
  Serial.print(F("The first phone number FROM String is "));
  Serial.println(conFrom2);
  Serial.print(F("The second phone number FROM char array is "));
  Serial.println(contactFromArray2);

  //---------------------------------------------//
  //------------------load first contact number-------------//

  myFile = SD.open("to1.txt");
  if (myFile) {
    Serial.println("phone number 1 command");
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      char c = myFile.read();  //gets one byte from serial buffer
      conTo1 += c;
    }
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening toContact1.txt");
  }

  conTo1.toCharArray(contactToArray1, 25);
  Serial.print(F("The first phone number TO String is "));
  Serial.println(conTo1);
  Serial.print(F("The first phone number TO char array is "));
  Serial.println(contactToArray1);

  //---------------------------------------------//
  //------------------load second contact number-------------//

  myFile = SD.open("to2.txt");
  if (myFile) {
    Serial.println(F("phone number 2 command"));
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      char c = myFile.read();  //gets one byte from serial buffer
      conTo2 += c;
    }
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println(F("error opening to2.txt"));
  }

  conTo1.toCharArray(contactToArray2, 25);
  Serial.print(F("The second phone number TO String is "));
  Serial.println(conTo2);
  Serial.print(F("The second phone number TO char array is "));
  Serial.println(contactToArray2);

  //---------------------------------------------//
  //------------------load third contact number-------------//

  myFile = SD.open("to3.txt");
  if (myFile) {
    Serial.println(F("loading third contact number"));
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      char c = myFile.read();  //gets one byte from serial buffer
      conTo3 += c;
    }
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println(F("error opening to3.txt"));
  }

  conTo3.toCharArray(contactToArray3, 25);
  Serial.print(F("The third phone number TO String is "));
  Serial.println(conTo3);
  Serial.print(F("The third phone number TO char array is "));
  Serial.println(contactToArray3);

  //---------------------------------------------//
  //------------------load URL header-------------//

  myFile = SD.open("URL.txt");
  if (myFile) {
    Serial.println(F("loading URL header"));
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      char c = myFile.read();  //gets one byte from serial buffer
      URLheader += c;
    }
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println(F("error opening URL.txt"));
  }

  URLheader.toCharArray(urlHeaderArray, 100);
  Serial.print(F("The URL header is "));
  Serial.println(URLheader);
  Serial.print(F("The URL header array is  "));
  Serial.println(urlHeaderArray);
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

void readModbus()
{
  Serial.println("In the readModbus() function now");
  delay(300);
  uint16_t result = node.readHoldingRegisters (0x0000, 1);
  Serial.print("The alarm register value is: ");
  Serial.println(result);


  if (result == node.ku8MBSuccess)
  {
    Serial.println("Alarm register result was success");
    int alarmRegister = node.getResponseBuffer(result);
    Serial.print("Register response:  ");
    Serial.println(alarmRegister);

    switch (alarmRegister)
    {

      case 1:
        sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, fault1 );
        break;
      case 15:
        sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Code15%20Unexpected%20Flame\"\r" );
        break;
      case 17:
        sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Code17%20Main%20Failure\"\r" );
        break;
      case 19:
        sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Code19%20MainIgn%20Failure\"\r" );
        break;
      case 28:
        sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Code28%20Pilot%20Failure\"\r" );
        break;
      case 29:
        sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Code29%20Interlock\"\r" );
        break;
      default:
        sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Check%20fault%20code\"\r" );
        break;
    }
  }
  else
  {
    sendSMS(urlHeaderArray, contactToArray1, contactFromArray1, "Body=Modbus%20Com%20Fail\"\r");
  }
}

void SIMboot()
{
//This function only boots the SIM module if it needs to be booted
//This prevents nuisance power-downs upon startup

unsigned char sim_buffer = 0;

  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      Serial1.print("AT\r"); //blank AT command elicits a response of OK
      delay(50);
    }

    if (Serial1.available())
    {
      while (Serial1.available())
      {
        sim_buffer = Serial1.read();
        Serial.write(sim_buffer);
      }
      sim_buffer = 0;
      Serial.println("SIM module appears to be on.  No need to boot");
      return;
    }


    else {
      Serial.println("SIM module appears to be off.  Attempting boot...");
      digitalWrite(SIMpin, HIGH);
      delay(3000);
      digitalWrite(SIMpin, LOW);
      Serial.println("boot attempted.  wait 10 seconds");
      delay(10000);
    }

  }
}



void initiateSim()
{
  //The remainder of the setup is for waking the
  //SIM module, logging on, and sending the first
  //test message to verify proper booting
  Serial.println("Hey!  Wake up!");
  Serial1.print("AT\r"); //Toggling this blank AT command elicits a mirror response from the
  //SIM module and helps to activate it.
  getResponse();
  Serial1.print("AT\r");
  getResponse();
  Serial1.print("AT\r");
  getResponse();
  Serial1.print("AT\r");
  getResponse();
  Serial1.print("AT\r");
  getResponse();
  //SIM MODULE SETUP---
  Serial1.print("AT+CGDCONT = 1, \"IP\",\"super\"\r"); //"super" is the key required to log onto the network using Twilio SuperSIM
  delay(500);
  getResponse();
  Serial1.print("AT+COPS=1,2,\"310410\"\r"); //310410 is AT&T's network code https://www.msisdn.net/mccmnc/310410/
  delay(5000);
  getResponse();
  Serial1.print("AT+SAPBR=3,1,\"APN\",\"super\"\r"); //establish SAPBR profile.  APN = "super"
  delay(3000);
  getResponse();
  Serial1.print("AT+SAPBR=1,1\r");
  delay(2000);
  getResponse();
  Serial1.print("AT+CMGD=0,4\r"); //this line deletes any existing text messages to ensure
  //that the message space is empy and able to accept new messages
  delay(100);
  getResponse();
  Serial1.print("AT+CMGF=1\r");
  delay(100);
  getResponse();
  Serial1.print("AT+CNMI=2,2,0,0,0\r"); //
  delay(100);
  getResponse();
  //the sendSMS function takes four parameters.
  sendSMS(urlHeaderArray, contactFromArray1, contactToArray1, SetCombody);
  //sendSMS(urlHeaderArray, contactFromArray1, contactToArray2, SetCombody);
  //sendSMS(urlHeaderArray, contactFromArray1, contactToArray3, SetCombody);
  delay(2000);

  Serial.println(F("Setup complete. Entering main loop"));

}
