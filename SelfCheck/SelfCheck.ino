// squawkbox_v1.0.2 26 Sept 2022 @ 1600

#include <SD.h>
#include <ModbusMaster.h>
#include <MemoryFree.h>

File myFile;
//The following const int pins are all pre-run in the PCB:
const int low1 = 5;
const int low2 = 6;
const int alarmPin = 8;
const int hlpcIN = 14;
const int hlpcOUT = 15;
const int gasINpin = 16;
const int gasOUTpin = 17;
const int MAX485_DE = 3;//to modbus module
const int MAX485_RE_NEG = 2;//to modbus module
const int SIMpin = A3;// this pin is routed to SIM pin 12 for boot (DF Robot SIM7000A module)

const int debounceInterval = 3000;//to prevent false alarms from electrical noise.
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
int gasIN;
int gasOUT;
int gasCounter;

//example char urlHeaderArray[] = "AT+HTTPPARA="URL","http://relay-post-8447.twil.io/recipient_loop?";
//example char contactFromArray[] = "From=%2b15034516078&";
//example char conToTotalArray[] = "To=%2b17065755866&";

char SetCombody[] = "Body=SquawkBox%20Setup%20Complete\"\r";
char LWbody[] = "Body=Primary%20Low%20Water\"\r";
char LW2body[] = "Body=Secondary%20Low%20Water\"\r";
char REPbody[] = "Body=CaseCart%20Routine%20Timer\"\r";
char HLPCbody[] = "Body=High%20Pressure%20Alarm\"\r";
//char CHECKbody[] = "Body=Good%20Check\"\r";
char BCbody[] = "Body=Boiler%20Down\"\r";

char contactFromArray[25];
char conToTotalArray[60];
char urlHeaderArray[100];

unsigned long currentMillis = 0;
unsigned long difference = 0;
unsigned long difference2 = 0;
unsigned long difference3 = 0;
unsigned long difference4 = 0;
unsigned long difference5 = 0;
unsigned long difference6 = 0;
unsigned long difference7 = 0;
unsigned long difference8 = 0;
unsigned long dailytimer = 43200000;
unsigned long msgtimer1 = 0;
unsigned long msgtimer2 = 0;
unsigned long msgtimer3 = 0;
unsigned long alarmTime = 0;
unsigned long alarmTime2 = 0;
unsigned long alarmTime3 = 0;
unsigned long alarmTime4 = 0;
unsigned long alarmTime5 = 0;

bool alarmSwitch = false;
bool alarmSwitch2 = false;
bool alarmSwitch3 = false;
bool alarmSwitch4 = false;
bool alarmSwitch5 = false;
bool msgswitch = false;
bool msgswitch2 = false;
bool msgswitch3 = false;
bool selfCheck = false;

ModbusMaster node;

void setup() 
{
  Serial.begin(9600);
  Serial1.begin(19200);
  Serial.println(F("This is Case_Cart_Boiler_1.v0 sketch."));

  pinMode(low1, INPUT);
  pinMode(low2, INPUT);
  pinMode(alarmPin, INPUT);
  pinMode(hlpcIN, INPUT);
  pinMode(hlpcOUT, INPUT);
  pinMode(gasINpin, INPUT);
  pinMode(gasOUTpin, INPUT);
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  pinMode(SIMpin, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  node.begin(1, Serial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission); 
  SIMboot();
  initiateSim();
  boot_SD();
  loadContacts();
  sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, SetCombody);
  Serial.println(F("Setup complete. Entering main loop"));
//  Serial.print(F("setup() Free RAM = "));
//  Serial.print(freeMemory());
//  Serial.println(F(" bytes."));
  memoryTest();
}

void loop()
{
  primaryCutoff = digitalRead(low1);
  secondaryCutoff = digitalRead(low2);
  alarm = digitalRead(alarmPin);
  hlpcCOMMON = digitalRead(hlpcIN);
  hlpcNC = digitalRead(hlpcOUT);
  gasIN = digitalRead(gasINpin);
  gasOUT = digitalRead(gasOUTpin);
  currentMillis = millis();

  primary_LW();
  secondary_LW();
  Honeywell_alarm();
  HLPC();
  gasPressure();
  timedmsg();
  SMSRequest();
  self_Check();
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
      sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, LWbody);
      Serial.println(F("message sent or simulated"));
      memoryTest();
//      Serial.print(F("primary_LW() Free RAM = "));
//      Serial.print(freeMemory());
//      Serial.println(F(" bytes."));
      plwcCounter = 1;
    }
    if (difference < debounceInterval)
    {
      Serial.println(difference);
      return;
    }
  }
  else
  {
    if (primaryCutoff == LOW && alarmSwitch)
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
      sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, LW2body);
      Serial.println(F("message sent or simulated"));
      memoryTest();
//      Serial.print(F("secondary_LW() Free RAM = "));
//      Serial.print(freeMemory());
//      Serial.println(F(" bytes."));
      slwcCounter = 1;
    }
    if (difference2 < debounceInterval)
    {
      Serial.println(difference2);
      return;
    }
  }
  else
  {
    if (secondaryCutoff == LOW && alarmSwitch2)
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
      Serial.println(F("sending FSG alarm message"));
      sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, BCbody);
      Serial.println(F("Entering modbus reading function..."));
      readModbus();
      Serial.println(F("message sent or simulated"));
      memoryTest();
//      Serial.print(F("Honeywell_alarm() Free RAM = "));
//      Serial.print(freeMemory());
//      Serial.println(F(" bytes."));
      alarmCounter = 1;
    }
    if (difference3 < debounceInterval)
    {
      Serial.println(difference3);
      return;
    }
  }
  else
  {
    if (alarm == LOW && alarmSwitch3)
    {
      alarmSwitch3 = false;
      difference3 = 0;
      alarmTime3 = 0;
      alarmCounter = 0;
      return;
    }
  }
}

void HLPC()
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
      Serial.println("Sending HLPC alarm message");
      sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, HLPCbody);
      Serial.println(F("message sent or simulated"));
      memoryTest();
//      Serial.print(F("HLPC() Free RAM = "));
//      Serial.print(freeMemory());
//      Serial.println(F(" bytes."));
      hlpcCounter = 1;
    }
    if (difference4 < debounceInterval)
    {
      Serial.println(difference4);
      return;
    }
  }
  else
  {
    if (hlpcNC && alarmSwitch4)
    {
      alarmSwitch4 = false;
      difference4 = 0;
      alarmTime4 = 0;
      hlpcCounter = 0;
      return;
    }
  }
}

void gasPressure()
{
  if ((gasIN == HIGH) && (gasOUT == LOW) && (gasCounter == 0))
  {
    if (alarmSwitch5 == false)
    {
      alarmTime5 = currentMillis;
      alarmSwitch5 = true;
      Serial.println("alarmSwitch5 is true");
    }
    difference5 = currentMillis - alarmTime5;

    if ( difference5 >= debounceInterval)
    {
      Serial.println("Sending Gas Pressure alarm message");
      sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, "Body=Gas%20Pressure%20Alarm\"\r");
      Serial.println(F("message sent or simulated"));
      memoryTest();
//      Serial.print(F("gasPressure() Free RAM = "));
//      Serial.print(freeMemory());
//      Serial.println(F(" bytes."));
      gasCounter = 1;
    }
    if (difference5 < debounceInterval)
    {
      Serial.println(difference5);
      return;
    }
  }
  else
  {
    if (gasOUT && alarmSwitch5)
    {
      alarmSwitch5 = false;
      difference5 = 0;
      alarmTime5 = 0;
      gasCounter = 0;
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
  Serial.print("finalURL is: ");
  Serial.println(finalURL);
  delay(20);
  Serial1.print("AT+HTTPTERM\r");
  delay(1000);
  Serial1.print("AT+SAPBR=3,1,\"APN\",\"super\"\r");
  delay(300);
  Serial1.print("AT+SAPBR=1,1\r");
  delay(1000);
  Serial1.print("AT+HTTPINIT\r");
  delay(100);
  Serial1.print("AT+HTTPPARA=\"CID\",1\r");
  delay(100);
  Serial1.println(finalURL);
  delay(100);
  Serial1.print("AT+HTTPACTION=1\r");
  delay(5000);
  memoryTest();
//  Serial.print(F("Send SMS Free RAM = "));
//  Serial.print(freeMemory());
//  Serial.println(F(" bytes."));
}

// void getResponse()
// {
//   if (Serial1.available())
//   {
//     while (Serial1.available())
//     {
//       data = Serial1.read();
//       Serial.write(data);
//     }
//     data = 0;
//   }
//   delay(500);
// }

void timedmsg()
{
  if (msgswitch == false)
  {
    msgtimer1 = currentMillis;
    msgswitch = true;
  }
  difference6 = currentMillis - msgtimer1;

  if (difference6 >= dailytimer)
  {
    sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, REPbody);
    difference6 = 0;
    msgswitch = false;
    msgtimer1 = 0;
  }
}
void self_Check()
{
  if (msgswitch2 == false)
  {
    msgtimer2 = currentMillis;
    msgswitch2 = true;
  }
  difference7 = currentMillis - msgtimer2;
  //Serial.print("Self check timer 300000 = ");
  //Serial.println(difference7);
  if (difference7 >= 300000)
  {
    sendSMS(urlHeaderArray,"To=%2b15034516078&","Fom=%2b19063393615&","Body=start\"\r");
    selfCheck = true;
    difference7 = 0;
    msgswitch2 = false;
    msgtimer2 = 0;
  }
  if(selfCheck)
  {
    if (msgswitch3 == false)
    {
      msgtimer3 = millis();
      msgswitch3 = true;
    }
    difference8 = millis() - msgtimer3;
    //Serial.print("Self check inside timer 180000 = ");
    //Serial.println(difference8);
    if (difference8 >= 180000)
    {
      msgswitch3 = false;
      selfCheck = false;
      digitalWrite(SIMpin, HIGH);
      delay(3000);
      digitalWrite(SIMpin, LOW);
      delay(5000);
      digitalWrite(SIMpin, HIGH);
      delay(3000);
      digitalWrite(SIMpin, LOW);
      delay(5000);
      SIMboot();
      delay(3000);
      sendSMS(urlHeaderArray, "To=%2b16158122833&", contactFromArray, "Body=SelfCheck%20Reboot\"\r");
    }
  }
}
void SMSRequest()
{
  if (Serial1.available() > 0) 
  {
    char SelfCheck[6];
    char incomingChar = Serial1.read();
    //Serial.print(incomingChar);
    if (incomingChar == 'C' || incomingChar == 'c' || incomingChar == 's') 
    {
      delay(100);
      //Serial.print(incomingChar);
      SelfCheck[0] = incomingChar;
      incomingChar = Serial1.read();
      if (incomingChar == 'H' || incomingChar == 'h' || incomingChar == 't') 
      {
        delay(100);
        //Serial.print(incomingChar);
        SelfCheck[1] = incomingChar;
        incomingChar = Serial1.read();
        if (incomingChar == 'E' || incomingChar == 'e' || incomingChar == 'a') 
        {
          delay(100);
          //Serial.print(incomingChar);
          SelfCheck[2] = incomingChar;
          incomingChar = Serial1.read();
          if (incomingChar == 'C' || incomingChar == 'c' || incomingChar == 'r') 
          {
            delay(100);
            //Serial.print(incomingChar);
            SelfCheck[3] = incomingChar;
            incomingChar = Serial1.read();
            if (incomingChar == 'K' || incomingChar == 'k' || incomingChar == 't') 
            {
              SelfCheck[4] = incomingChar;
              Serial.print("SelfCheck = ");
              Serial.println(SelfCheck);
              if(strcmp(SelfCheck,"start") == 0)
              {
                sendSMS(urlHeaderArray, conToTotalArray, contactFromArray,"Body=Self%20Check%20Confirmed\"\r");
                selfCheck = false;
                msgswitch3 = false;
              }
              else
              {
                Serial.println("CHECK recieved!!!");
                Serial.println(F("GOOD CHECK. SMS SYSTEMS ONLINE"));
                Serial.println(F("SENDING CHECK VERIFICATION MESSAGE")) ;
                sendSMS(urlHeaderArray, conToTotalArray, contactFromArray,"Body=Communication%20verification%20confirmed\"\r");
                Serial.println("verification message sent");
                Serial1.print("AT+CMGD=0,4\r");
                memoryTest();
  //              Serial.print(F("SMS REQUEST Free RAM = "));
  //              Serial.print(freeMemory());
  //              Serial.println(F(" bytes."));
                return;
              }
            }
          }
        }
      }
    }
  }
  return;
}

void loadContacts()
{
  String URLheader = "";
  String conFrom1 = "";
  String conTo1 = "";
  String conTo2 = "";
  String conTo3 = "";
  String conTo4 = "";
  String conToTotal = "To=%2b1";
  memoryTest();
//  Serial.print(F("loadContacts() BEGENNING Free RAM = "));
//  Serial.print(freeMemory());
//  Serial.println(F(" bytes."));

//------load "from" number.  This is the number alert messages will apear to be from------//

  conFrom1 = fill_from_SD("from1.txt");
  conFrom1.toCharArray(contactFromArray, 25);
  Serial.print("From number is: ");
  Serial.print(contactFromArray);

//------load "to" numbers.  These are the numbers alert messages will be sent to------//

  conTo1 = fill_from_SD("To1.txt");
  if (conTo1[0] > 0) 
  {
    conToTotal += conTo1;
  }

  conTo2 = fill_from_SD("To2.txt");
  if (conTo2[0] > 0) 
  {
    conToTotal += "," + conTo2;
  }

  conTo3 = fill_from_SD("To3.txt");
  if (conTo3[0] > 0) 
  {
    conToTotal += "," + conTo3;
  }

  conTo4 = fill_from_SD("To4.txt");
  if (conTo4[0] > 0) 
  {
    conToTotal += "," + conTo3;
  }

  conToTotal += "&";//format the "to" list of numbers for being in the URL by ending it with '&' so that next parameter can come after

  Serial.print(F(" The total contact list is: "));
  Serial.println(conToTotal);
  Serial.print("eighth position character: ");
  Serial.println(conToTotal[7]);

  if (conToTotal[7] == ',')
  {
    Serial.print(F("Oops. Found a ',' where it soundnt be... Fixing NOW."));
    conToTotal.remove(3, 1);
    Serial.print(F("Corrected contact list: "));
    Serial.println(conToTotal);
  }

  conToTotal.toCharArray(conToTotalArray, 60);

  URLheader = fill_from_SD("URL.txt");
  URLheader.toCharArray(urlHeaderArray, 100);
  Serial.print("URL header is: ");
  Serial.println(urlHeaderArray);
//  Serial.print(F("loadContacts() ENDING Free RAM = "));
//  Serial.print(freeMemory());
//  Serial.println(F(" bytes."));
  Serial.println(F("Contacts should be Loaded."));
}

String fill_from_SD(String file_name)
{
  String temporary_string = "";
  String info_from_SD = "";
  myFile = SD.open(file_name);
  if (myFile) 
  { // read from the file until there's nothing else in it:
    while (myFile.available())
    {
      char c = myFile.read();  //gets one byte from serial buffer
      info_from_SD += c;
    }
    myFile.close();
    return info_from_SD;
  }
  else
  {
    // if the file didn't open, print an error:
    Serial.println("error opening SD file to load in contact #s.");
    return "fial";
  } 
}

void preTransmission() // user designated action required by the MODBUS library
{                      // writing these terminals to HIGH / LOW tells the RS-485 board to send or recieve
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}
void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void readModbus()
{
  Serial.println("In the readModbus() function now");
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
      case  1: sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, "Body=Fault%20Code1%20No%20Purge%20Card\"\r");
               break;
      case 10: sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, "Body=Fault%20Code10%20PreIgnition%20Interlock%20Standby\"\r");
               break;
      case 14: sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, "Body=Fault%20Code14%20High%20Fire%20Interlock%20Switch\"\r");
               break;
      case 15: sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, "Body=Fault%20Code15%20Unexpected%20Flame\"\r");
               break;
      case 17: sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, "Body=Fault%20Code17%20Main%20Flame%20Failure%20RUN\"\r");
               break;
      case 19: sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, "Body=Fault%20Code19%20Main%20Flame%20Ignition%20Failure\"\r");
               break;
      case 20: sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, "Body=Fault%20Code20%20Low%20Fire%20Interlock%20Switch\"\r");
               break;
      case 28: sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, "Body=Fault%20Code28%20Pilot%20Flame%20Failure\"\r");
               break;
      case 29: sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, "Body=Fault%20Code29%20Lockout%20Interlock\"\r");
               break;
      case 33: sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, "Body=Fault%20Code33%20PreIgnition%20Interlock\"\r");
               break;
      case 47: sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, "Body=Fault%20Code47%20Jumpers%20Changed\"\r");
               break;
      default: sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, "Body=Fault%20Check%20Fault%20Code\"\r");
               break;
    }
  }
  else
  {
    sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, "Body=Modbus%20Com%20Fail\"\r");
  }
}

void SIMboot()
{
  Serial.println("Booting SIM module...");
//This function only boots the SIM module if it needs to be booted
//This prevents nuisance power-downs upon startup
  unsigned char sim_buffer {};
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
      Serial.println("SIM module appears to be on.  No need to boot");
      return;
    }
    else 
    {
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
  Serial.println("Initiating wakeup sequence...");
  Serial.println("Hey!  Wake up!");
  Serial1.print("AT\r");
  delay(50);
  Serial1.print("AT\r");
  delay(50);
  Serial1.print("AT\r"); 
  delay(50);
  Serial1.print("AT\r"); 
  delay(50);
  Serial1.print("AT\r"); 
  delay(50);
  //SIM MODULE SETUP
  Serial1.print("AT+CGDCONT=1,\"IP\",\"super\"\r");//"super" is the key required to log onto the network using Twilio SuperSIM
  delay(500);
  Serial1.print("AT+COPS=1,2,\"310410\"\r");//310410 is AT&T's network code https://www.msisdn.net/mccmnc/310410/
  delay(5000);
  Serial1.print("AT+SAPBR=3,1,\"APN\",\"super\"\r");//establish SAPBR profile.  APN = "super"
  delay(3000);
  Serial1.print("AT+SAPBR=1,1\r");
  delay(2000);
  Serial1.print("AT+CMGD=0,4\r");/*this line deletes any existing text messages to ensure
                                  that the message space is empy and able to accept new messages*/
  delay(100);
  Serial1.print("AT+CMGF=1\r");
  delay(100);
  Serial1.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);
  //sendSMS(urlHeaderArray, conToTotalArray, contactFromArray, SetCombody);
  delay(2000);
}

void boot_SD()
{
  Serial.println("Booting SD module...");
  if (!SD.begin(10)) 
  {
    Serial.println(F("SD Module initialization failed!"));
    delay(5000);
    for(int i = 0; i < 4; ++i)
    {
      if (!SD.begin(10)) 
      {
        Serial.println(F("SD Module initialization failed!"));
        sendSMS("AT+HTTPPARA=\"URL\",\"http://relay-post-8447.twil.io/recipient_loop?", "To=%2b16158122833&", "From=%2b15034516078&", "Body=SD%20Module%20Initialization%20Fail\"\r");
        delay(3000);
      }
      else
      {
        Serial.println(F("SD Module initialization done."));
        break;
      }
    }
  }
  else
  {
    Serial.println(F("SD Module initialization done."));
  }
}

void memoryTest()
{
  int threshold = 3000;
  int memory = freeMemory();
  Serial.print(F("memoryTest() Free RAM = "));
  Serial.print(memory);
  Serial.println(F(" bytes "));
  if (memory < threshold)
  {
    sendSMS(urlHeaderArray, "To=%2b16158122833&", contactFromArray, "Body=Memory%20Alert%20Possible%20Leak\"\r");
  }
}
