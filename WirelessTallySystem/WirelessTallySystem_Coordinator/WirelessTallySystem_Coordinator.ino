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

byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0x25, 0x4E }; // <= SETUP!  MAC address of the Arduino
IPAddress clientIp(172, 31, 8, 100);                 // <= SETUP!  IP address of the Arduino
IPAddress switcherIp(172, 31, 8, 1);                 // <= SETUP!  IP address of the ATEM Switcher

long g_t_Sync = 500; // [ms] (re-)sync timer period


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
uint16_t g_idx_PGM_last = 0U;
uint16_t g_idx_PRV_last = 0U;


void setup()
{
  // status LED
  pinMode(C_PIN_STS_LED, OUTPUT);

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

    // PGM
    uint16_t loc_idx_PGM = AtemSwitcher.getProgramInput();

    if( loc_idx_PGM != g_idx_PGM_last )
    {
      Serial << F("PGM: ") << loc_idx_PGM << F("\n");

      Timer1.stop();

      uint8_t loc_Payload[] = { 'P', (uint8_t)loc_idx_PGM + 0x30 };
      cClientManager::GetInstance()->Broadcast<2u>(loc_Payload);

      Timer1.start();
    }

    g_idx_PGM_last = loc_idx_PGM;

    // PRV
    uint16_t loc_idx_PRV = AtemSwitcher.getPreviewInput();

    if( loc_idx_PRV != g_idx_PRV_last )
    {
      Serial << F("PRV: ") << loc_idx_PRV << F("\n");

      Timer1.stop();

      uint8_t loc_Payload[] = { 'V', (uint8_t)loc_idx_PRV + 0x30 };
      cClientManager::GetInstance()->Broadcast<2u>(loc_Payload);

      Timer1.start();
    }

    g_idx_PRV_last = loc_idx_PRV;
  }
  else
  {
    g_b_ATEM_connected = false;

    Timer1.stop();
  }

  // XBee status handling
  xbee.readPacket();

  if( xbee.getResponse().isAvailable() )
  {
    if( xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE )
    {
      ZBTxStatusResponse loc_sts_TX;
      
      xbee.getResponse().getZBTxStatusResponse(loc_sts_TX);
      if( loc_sts_TX.getDeliveryStatus() != SUCCESS )
      {
        // unsuccessful TX, client is lost
        cClientManager::GetInstance()->OnClientDisconnected(loc_sts_TX.getFrameId() - 1u);
      }
    }
    else if( xbee.getResponse().getApiId() == ZB_IO_NODE_IDENTIFIER_RESPONSE )
    {
      ZBRxResponse loc_sts_RX;
      xbee.getResponse().getZBRxResponse(loc_sts_RX);
      
      cClientManager::GetInstance()->OnClientConnected(loc_sts_RX.getRemoteAddress64());
    }
  }
}

// periodic (re-)sync
void sync(void)
{
  if( g_b_ATEM_connected )
  {
    uint8_t loc_Payload1[] = { 'P', g_idx_PGM_last + 0x30 };
    cClientManager::GetInstance()->Broadcast<2u>(loc_Payload1);

    uint8_t loc_Payload2[] = { 'V', g_idx_PRV_last + 0x30 };
    cClientManager::GetInstance()->Broadcast<2u>(loc_Payload2);

    digitalWrite(C_PIN_STS_LED, digitalRead(C_PIN_STS_LED) ^ 1);
  }
}
