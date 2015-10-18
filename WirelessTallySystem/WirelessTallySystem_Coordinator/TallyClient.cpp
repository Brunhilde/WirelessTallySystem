// 
// 
// 

#include "TallyClient.h"

//! XBee instance
extern XBee xbee;

void cTallyClient::Send(uint8_t *arg_ptr_Payload, uint8_t arg_PayloadSize)
{
  ZBTxRequest& loc_TX_Frame = GetTXFrame();

  loc_TX_Frame.setPayload(arg_ptr_Payload, arg_PayloadSize);

  xbee.send(loc_TX_Frame);
}

void cTallyClient::InformTallyProgramChange(void)
{
  uint8_t loc_Payload[] = { 'P', (m_b_TallyPGM) ? '1' : '0' };

  Send(loc_Payload, sizeof(loc_Payload));
}

void cTallyClient::InformTallyPreviewChange(void)
{
  uint8_t loc_Payload[] = { 'V', (m_b_TallyPRV) ? '1' : '0' };

  Send(loc_Payload, sizeof(loc_Payload));
}
