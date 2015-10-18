/*
 Name:		WirelessTallySystem_Client.ino
 Created:	14.10.2015 11:59:04
 Author:	Tobias Dieterich
*/

#include <SoftwareSerial.h>
#include <Streaming.h>
#include <TimerOne.h>

const bool G_DEBUG = false;

const int C_PIN_SS_RX = 2;
const int C_PIN_SS_TX = 3;

const int C_PIN_DIP1 = 4;
const int C_PIN_DIP2 = 5;
const int C_PIN_DIP3 = 6;
const int C_PIN_DIP4 = 7;

const int C_PIN_TALLY_PGM = 8;
const int C_PIN_TALLY_PRV = 9;

const int C_PIN_STS_LED = 13;

int G_ADDR = 0;
int G_ADDR_LAST = 0;

bool g_Tally_PGM = false;
bool g_Tally_PRV = false;

typedef enum
{
  E_STATE_IDLE,
  E_STATE_RX_STATE,
  E_STATE_ACTION
} eState;

eState g_State = E_STATE_IDLE;

char g_Cmd = ' ';
int  g_ID = 0;

SoftwareSerial XBee(C_PIN_SS_RX, C_PIN_SS_TX);

// prototypes
void checkAddress(void);

void setup()
{
  // put your setup code here, to run once:
  pinMode(C_PIN_DIP1, INPUT);
  pinMode(C_PIN_DIP2, INPUT);
  pinMode(C_PIN_DIP3, INPUT);
  pinMode(C_PIN_DIP4, INPUT);

  pinMode(C_PIN_TALLY_PGM, OUTPUT);
  pinMode(C_PIN_TALLY_PRV, OUTPUT);

  pinMode(C_PIN_STS_LED, OUTPUT);

  Serial.begin(9600);
  XBee.begin(9600);

  Timer1.initialize(100000);
  Timer1.attachInterrupt(statusLED);
}

void loop()
{
  // put your main code here, to run repeatedly:
  checkAddress();

  if (G_ADDR_LAST != G_ADDR)
  {
    Serial << F("Addr = ") << G_ADDR << F("\n");

    G_ADDR_LAST = G_ADDR;

    // inform coordinator
    XBee << 'S' << G_ADDR_LAST;
  }

  // tally
  switch (g_State)
  {
  case E_STATE_IDLE:
  {
    if (XBee.available())
    {
      if (((char)XBee.peek() == 'P') || ((char)XBee.peek() == 'V'))
      {
        g_Cmd = (char)XBee.read();
        g_State = E_STATE_RX_STATE;

        if (G_DEBUG)
        {
          Serial << F("-> E_STATE_RX_STATE\n");
        }
      }
      else if( (char)XBee.peek() == 'S' )
      {
        g_Cmd = (char)XBee.read();

        XBee << 'S' << G_ADDR_LAST;

        if( G_DEBUG )
        {
          Serial << F("Received source index request\n");
        }
      }
      else
      {
        if (G_DEBUG)
        {
          Serial << F("unrecognized '") << (char)XBee.peek() << F("'\n");
        }

        static_cast<void>(XBee.read());
      }
    }
  }
  break;

  case E_STATE_RX_STATE:
  {
    if (XBee.available())
    {
      if (isDigit((char)XBee.peek()))
      {
        g_ID = (char)XBee.read() - 0x30;

        g_State = E_STATE_ACTION;

        if (G_DEBUG)
        {
          Serial << F("-> E_STATE_RX_ACTION\n");
        }
      }
      else
      {
        g_State = E_STATE_IDLE;

        if (G_DEBUG)
        {
          Serial << F("-> E_STATE_RX_STATE back to E_STATE_DILE (") << (char)XBee.peek() << F(")\n");
        }
      }
    }
  }
  break;

  case E_STATE_ACTION:
  {
    switch (g_Cmd)
    {
    case 'P':
    {
      if (g_ID == 1)
      {
        digitalWrite(C_PIN_TALLY_PGM, 1);

        g_Tally_PGM = true;

        Serial << F("PGM tally 'ON'\n");
        Serial.flush();
      }
      else
      {
        digitalWrite(C_PIN_TALLY_PGM, 0);

        g_Tally_PGM = false;

        Serial << F("PGM tally 'OFF'\n");
        Serial.flush();
      }
    }
    break;

    case 'V':
    {
      if (g_ID == 1)
      {
        digitalWrite(C_PIN_TALLY_PRV, 1);

        g_Tally_PRV = true;

        Serial << F("PRV tally 'ON'\n");
        Serial.flush();
      }
      else
      {
        digitalWrite(C_PIN_TALLY_PRV, 0);

        g_Tally_PRV = false;

        Serial << F("PRV tally 'OFF'\n");
        Serial.flush();
      }
    }
    break;
    }

    g_State = E_STATE_IDLE;

    if (G_DEBUG)
    {
      Serial << F("-> E_STATE_IDLE\n");
    }
  }
  break;
  }
}

void statusLED(void)
{
  if( g_Tally_PGM )
  {
    digitalWrite(C_PIN_STS_LED, 1);
  }
  else if( g_Tally_PRV )
  {
    digitalWrite(C_PIN_STS_LED, digitalRead(C_PIN_STS_LED) ^ 1);
  }
  else
  {
    digitalWrite(C_PIN_STS_LED, 0);
  }
}

void checkAddress(void)
{
  G_ADDR = (digitalRead(C_PIN_DIP1))
    + (digitalRead(C_PIN_DIP2) << 1)
    + (digitalRead(C_PIN_DIP3) << 2)
    + (digitalRead(C_PIN_DIP4) << 3);
}
