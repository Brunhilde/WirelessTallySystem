// TallyClient.h

#ifndef _TALLYCLIENT_h
#define _TALLYCLIENT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <XBee.h>
#include <Streaming.h>


class cTallyClient
{
public:
  cTallyClient(void)
    : m_ID(0u)
    , m_Addr()
    , m_TX_Frame()
    , m_bAvailable(false)
    , m_idx_Source(0u)
    , m_b_TallyPGM(false)
    , m_b_TallyPRV(false)
  {

  }

  cTallyClient(const uint8_t arg_ID, uint32_t arg_msb, uint32_t arg_lsb)
    : m_ID(arg_ID)
    , m_Addr(arg_msb, arg_lsb)
    , m_TX_Frame()
    , m_bAvailable(true)
    , m_idx_Source(0u)
    , m_b_TallyPGM(false)
    , m_b_TallyPRV(false)
  {
    m_TX_Frame.setAddress64(m_Addr);
    m_TX_Frame.setFrameId(arg_ID+1u);
  }


  // set
  void SetID(const uint8_t arg_ID)
  {
    m_ID = arg_ID;

    m_TX_Frame.setFrameId(arg_ID + 1u);
  }

  void SetAddress(const XBeeAddress64& arg_Addr)
  {
    m_Addr = arg_Addr;

    m_TX_Frame.setAddress64(arg_Addr);
  }

  void SetAvailable(const bool arg_bAvailable)
  {
    m_bAvailable = arg_bAvailable;
  }

  void SetSourceIndex(const uint16_t arg_idx_Source)
  {
    m_idx_Source = arg_idx_Source;
  }

  void SetTallyProgram(const bool arg_TallyState)
  {
    if( arg_TallyState != m_b_TallyPGM )
    {
      m_b_TallyPGM = arg_TallyState;

      if( m_b_TallyPGM )
      {
        Serial << F("Client :: ID=") << GetID() << F(", source index=") << GetSourceIndex() << F(", now on PGM!\n");
      }

      InformTallyProgramChange();
    }
  }

  void SetTallyPreview(const bool arg_TallyState)
  {
    if( arg_TallyState != m_b_TallyPRV )
    {
      m_b_TallyPRV = arg_TallyState;

      if( m_b_TallyPRV )
      {
        Serial << F("Client :: ID=") << GetID() << F(", source index=") << GetSourceIndex() << F(", now on PRV!\n");
      }

      InformTallyPreviewChange();
    }
  }

  // get
  uint8_t GetID(void) const
  {
    return m_ID;
  }

  XBeeAddress64& GetAddress(void)
  {
    return m_Addr;
  }

  ZBTxRequest& GetTXFrame(void)
  {
    return m_TX_Frame;
  }

  uint16_t GetSourceIndex(void) const
  {
    return m_idx_Source;
  }

  bool GetTallyProgram(void) const
  {
    return m_b_TallyPGM;
  }

  bool GetTallyPreview(void) const
  {
    return m_b_TallyPRV;
  }

  // other stuff
  void Send(uint8_t *arg_ptr_Payload, uint8_t arg_PayloadSize);

  bool IsAvailable(void) const
  {
    return m_bAvailable;
  }

  void InformTallyProgramChange(void);
  void InformTallyPreviewChange(void);

private:
  //! client ID
  uint8_t m_ID;
  //! client 64bit XBee address
  XBeeAddress64 m_Addr;
  //! frame for TX messages
  ZBTxRequest   m_TX_Frame;
  //! available flag
  bool m_bAvailable;
  //! source index
  uint16_t m_idx_Source;
  //! tally PGM state
  bool m_b_TallyPGM;
  //! tally PRV state
  bool m_b_TallyPRV;
};

#endif

