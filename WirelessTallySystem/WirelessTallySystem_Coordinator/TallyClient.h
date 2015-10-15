// TallyClient.h

#ifndef _TALLYCLIENT_h
#define _TALLYCLIENT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <XBee.h>

class cTallyClient
{
public:
  cTallyClient(void)
    : m_ID(0u)
    , m_Addr()
    , m_TX_Frame()
    , m_bAvailable(false)
  {

  }

  cTallyClient(const uint8_t arg_ID, uint32_t arg_msb, uint32_t arg_lsb)
    : m_ID(arg_ID)
    , m_Addr(arg_msb, arg_lsb)
    , m_TX_Frame()
    , m_bAvailable(true)
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

  // other stuff
  bool IsAvailable(void) const
  {
    return m_bAvailable;
  }

private:
  //! client ID
  uint8_t m_ID;
  //! client 64bit XBee address
  XBeeAddress64 m_Addr;
  //! frame for TX messages
  ZBTxRequest   m_TX_Frame;
  //! available flag
  bool m_bAvailable;
};

#endif

