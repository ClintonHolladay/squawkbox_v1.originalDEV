

unsigned char data = 0;


void setup()
{

  Serial.begin(115200);
  Serial1.begin(115200);

}


void loop()
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

  if (Serial.available())
  {
    data = Serial.read();
  }

  if (data == '1')
  {
    Serial1.print("AT+CGMI\r"); //Manufacturer identification
  }
  if (data == '2')
  {
    Serial1.print("AT+SAPBR=3,1,\"APN\",\"super\"\r"); //Software version
  }

  if (data == '3')
  {
    Serial1.print("AT+SAPBR=1,1\r");
  }

  if (data == '4')
  {
    Serial1.print("AT+HTTPINIT\r");
  }
  if (data == '5')
  {
    Serial1.print("AT+HTTPPARA=\"CID\",1\r");
  }
  if (data == '6')
  {
    //Serial1.print("AT+HTTPPARA=\"URL\",\"http://enqmw2xe0xnp.x.pipedream.net/\"\r");
    Serial1.print("AT+HTTPPARA=\"URL\",\"http://relay-post-8447.twil.io/secondary-low-water?From=%2b19049808059&To=%2b17065755866&Body=Low%20Water\"\r");
  }
  if (data == '7')
  {
    Serial1.print("AT+HTTPACTION=1\r");
  }
  if (data == '8')
  {
    Serial1.print("AT+HTTPTERM\r");
  }
  if (data == '9')
  {
    Serial1.print("AT+SAPBR=0,1\r");
  }
  if (data == 'a')
  {
    Serial1.print("AT+CMGD=0,4\r");
  }

  if (data == 'b')
  {
    Serial1.print("AT+CGDCONT=1,\"IP\",\"super\"\r");
  }

  if (data == 'c')
  {
    Serial1.print("AT+COPS=1,2,\"310410\",7\r");
  }
  if (data == 'd')
  {
    Serial1.print("AT+COPS?\r");
  }
  if (data == 'e')
  {
    Serial1.print("AT+CPOWD=1\r");
  }
  if (data == 'f')
  {
    Serial1.print("AT+COPS=?\r");
  }
  if (data == 'g')
  {
    Serial1.print("AT+HTTPDATA=100,5000\r\n");
  }
  if (data == 'h')
  {
    Serial1.print("Content-Length:100\r\n{\"Body=body\"}(char)26\r\n");

    delay(3000);

  }
  if (data == 'i')
  {
    Serial1.print("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n");
  }

  if (data == 'j')
  {
    Serial1.print("AT+HTTPPARA=\"USERDATA\",\"To=\'+17065755866\'\"\r\n");
  }
  if (data == 'k')
  {
    Serial1.print("\r\n");
  }
  if (data == 'l')
  {
    Serial1.print("Body=spam\r\n");
  }
  if (data == 'm')
  {
    Serial1.print("AT+IPR=0\r");
  }
  if (data == 'o')
  {
    Serial1.print("AT+CMGF=1\r");
  }
  if (data == 'n')
  {
    Serial1.print("AT+CMGR=1,0\r");
  }
  if (data == 'p')
  {
    Serial1.print("AT+CMGL\r");
  }
  if (data == 'q')
  {
    Serial1.print("AT+CGMR\r");
  }
  if (data == 'r')
  {
    Serial1.print("AT+CSQ\r");
  }
}
