/*
 Name:		WirelessTallySystem_Coordinator.ino
 Created:	14.10.2015 11:58:21
 Author:	Tobias Dieterich
*/

#include "ClientManager.h"
#include "TallyClient.h"
#include <SPI.h>
#include <Ethernet.h>
#include <Streaming.h>
#include <SoftwareSerial.h>
#include <ATEMbase.h>
#include <ATEMstd.h>
#include <TimerOne.h>
#include <XBee.h>

#include "ClientManager.h"


// ----------------------------------------------
//  S E T U P
// ----------------------------------------------
const bool G_DEBUG = 0;

const int C_PIN_SS_RX = 2;
const int C_PIN_SS_TX = 3;

const int C_PIN_STS_LED = 9;
const int C_PIN_LCL_TALLY = 8;

byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0x25, 0x4E }; // <= SETUP!  MAC address of the Arduino
IPAddress clientIp(172, 31, 8, 100);                 // <= SETUP!  IP address of the Arduino
IPAddress switcherIp(172, 31, 8, 1);                 // <= SETUP!  IP address of the ATEM Switcher

long g_t_Sync = 1000; // [ms] (re-)sync timer period


// ----------------------------------------------
// I N S T A N C E S
// ----------------------------------------------
ATEMstd AtemSwitcher;
SoftwareSerial XBeeSS(C_PIN_SS_RX, C_PIN_SS_TX);
XBee xbee;


// ----------------------------------------------
// G L O B A L S
// ----------------------------------------------
bool g_b_ATEM_connected = false;
uint16_t g_cnt_TallySources = 0U;


void setup()
{
  // status LED
  pinMode(C_PIN_STS_LED, OUTPUT);

  // local tally output
  pinMode(C_PIN_LCL_TALLY, OUTPUT);
  digitalWrite(C_PIN_LCL_TALLY, LOW);

  // serial port for status messages
  Serial.begin(9600);
  Serial << F("\n- - - - - - - -\nWirelessTallySystem Coordinator started\n");

  // XBee initialization
  XBeeSS.begin(9600);
  xbee.setSerial(XBeeSS);

  // ATEM connection
  Ethernet.begin(mac, clientIp);
  AtemSwitcher.begin(switcherIp);
  AtemSwitcher.serialOutput(0x80);
  AtemSwitcher.connect();

  // (re-)sync timer
  Timer1.initialize(g_t_Sync*1000);
  Timer1.attachInterrupt(sync);

  // client handling init
  cClientManager::GetInstance()->Init();
}

void loop()
{
  AtemSwitcher.runLoop();

  if( AtemSwitcher.isConnected() )
  {
    if( !g_b_ATEM_connected )
    {
      Timer1.start();
    }
    g_b_ATEM_connected = true;

    // determine tally sources
    if( g_cnt_TallySources == 0u )
    {
      g_cnt_TallySources = AtemSwitcher.getTallyByIndexSources();
    }

    // determine tally states
    for( uint16_t i = 0u; i < g_cnt_TallySources; ++i )
    {
      uint8_t loc_State = AtemSwitcher.getTallyByIndexTallyFlags(i);

      cClientManager::GetInstance()->SetClientTallyState(i+1u, loc_State);

      // local tally output
      if( (i+1u) == 7u )
      {
        if( loc_State & 0x01 )
        {
          digitalWrite(C_PIN_LCL_TALLY, HIGH);
        }
        else if( !(loc_State & 0x01) )
        {
          digitalWrite(C_PIN_LCL_TALLY, LOW);
        }
        else
        {
          // NOP
        }
      }
    }
  }
  else
  {
    g_b_ATEM_connected = false;

    digitalWrite(C_PIN_LCL_TALLY, LOW);

    Timer1.stop();
  }

  // XBee status handling
  xbee.readPacket();

  if( xbee.getResponse().isAvailable() )
  {
    if( xbee.getResponse().getApiId() == ZB_RX_RESPONSE )
    {
      ZBRxResponse loc_RX;

      xbee.getResponse().getZBRxResponse(loc_RX);
      
      cClientManager::GetInstance()->OnClientDataReceived(loc_RX.getRemoteAddress64(), loc_RX.getData(), loc_RX.getDataLength());
    }
    else if( xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE )
    {
      ZBTxStatusResponse loc_sts_TX;
      
      xbee.getResponse().getZBTxStatusResponse(loc_sts_TX);
      if( loc_sts_TX.getDeliveryStatus() != SUCCESS )
      {
        // unsuccessful TX, client is lost
        Timer1.stop(); //noInterrupts();
        cClientManager::GetInstance()->OnClientDisconnected(loc_sts_TX.getFrameId() - 1u);
        Timer1.start(); // interrupts();
      }
    }
    else if( xbee.getResponse().getApiId() == ZB_IO_NODE_IDENTIFIER_RESPONSE )
    {
      ZBRxResponse loc_sts_RX;
      xbee.getResponse().getZBRxResponse(loc_sts_RX);
      
      Timer1.stop(); //noInterrupts();
      cClientManager::GetInstance()->OnClientConnected(loc_sts_RX.getRemoteAddress64());
      Timer1.start(); //interrupts();
    }
    else
    {
      // NOP
    }
  }
}

// periodic (re-)sync
void sync(void)
{
  if( g_b_ATEM_connected )
  {
    cClientManager::GetInstance()->InformClientsSync();

    digitalWrite(C_PIN_STS_LED, digitalRead(C_PIN_STS_LED) ^ 1);
  }
}
