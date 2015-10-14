/*
 Name:		WirelessTallySystem_Coordinator.ino
 Created:	14.10.2015 11:58:21
 Author:	Tobias Dieterich
*/

#include <SPI.h>
#include <Ethernet.h>
#include <Streaming.h>
#include <SoftwareSerial.h>
#include <ATEMbase.h>
#include <ATEMstd.h>
#include <TimerOne.h>
#include <XBee.h>

const bool G_DEBUG = 0;

const int C_PIN_SS_RX = 2;
const int C_PIN_SS_TX = 3;

const int C_PIN_STS_LED = 9;

ATEMstd AtemSwitcher;

SoftwareSerial XBeeSS(C_PIN_SS_RX, C_PIN_SS_TX);

uint16_t g_idx_PGM_last = 0U;
uint16_t g_idx_PRV_last = 0U;

byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0x25, 0x4E }; // <= SETUP!  MAC address of the Arduino
IPAddress clientIp(172, 31, 8, 100);                 // <= SETUP!  IP address of the Arduino
IPAddress switcherIp(172, 31, 8, 1);                 // <= SETUP!  IP address of the ATEM Switche

XBeeAddress64 g_Addresses[] =
{
  XBeeAddress64(0x0013A200, 0x40DC25E2),
  XBeeAddress64(0x0013A200, 0x40DC25F9)
};
uint8_t g_cnt_Addresses = 2;

XBee xbee;

void setup()
{
  pinMode(C_PIN_STS_LED, OUTPUT);

  Ethernet.begin(mac, clientIp);
  Serial.begin(9600);
  Serial << F("\n- - - - - - - -\nSerial Started\n");

  XBeeSS.begin(9600);

  xbee.setSerial(XBeeSS);

  AtemSwitcher.begin(switcherIp);
  AtemSwitcher.serialOutput(0x80);
  AtemSwitcher.connect();

  Timer1.initialize(500000);
  Timer1.attachInterrupt(sync);
}

void loop()
{
  AtemSwitcher.runLoop();

  // PGM
  uint16_t loc_idx_PGM = AtemSwitcher.getProgramInput();

  if (loc_idx_PGM != g_idx_PGM_last)
  {
    Serial << "PGM: " << loc_idx_PGM << "\n";

    Timer1.stop();
    uint8_t loc_payload[] = { 'P', (uint8_t)loc_idx_PGM + 0x30 };
    broadcast(loc_payload);
    Timer1.start();
  }

  g_idx_PGM_last = loc_idx_PGM;

  // PRV
  uint16_t loc_idx_PRV = AtemSwitcher.getPreviewInput();

  if (loc_idx_PRV != g_idx_PRV_last)
  {
    Serial << "PRV: " << loc_idx_PRV << "\n";

    Timer1.stop();
    uint8_t loc_payload[] = { 'V', (uint8_t)loc_idx_PRV + 0x30 };
    broadcast(loc_payload);
    Timer1.start();
  }

  g_idx_PRV_last = loc_idx_PRV;
}

void sync(void)
{
  uint8_t loc_payload1[] = { 'P', g_idx_PGM_last + 0x30 };
  broadcast(loc_payload1);
  uint8_t loc_payload2[] = { 'V', g_idx_PRV_last + 0x30 };
  broadcast(loc_payload2);

  digitalWrite(C_PIN_STS_LED, digitalRead(C_PIN_STS_LED) ^ 1);
}

void broadcast(uint8_t arg_payload[2])
{
  for (uint8_t i = 0U; i < g_cnt_Addresses; ++i)
  {
    ZBTxRequest loc_TX(g_Addresses[i], arg_payload, sizeof(arg_payload));
    xbee.send(loc_TX);
  }
}
