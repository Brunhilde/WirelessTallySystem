// ClientManager.h

#ifndef _CLIENTMANAGER_h
#define _CLIENTMANAGER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <XBee.h>

#include "TallyClient.h"

#ifndef MAX_CLIENTS
  #define MAX_CLIENTS 16
#endif


//! XBee instance
extern XBee xbee;


class cClientManager
{
public:
  static cClientManager *GetInstance(void)
  {
    return &m_Instance;
  }

  void Init(void);
  
  // callbacks
  void OnClientConnected(XBeeAddress64& arg_Addr);
  void OnClientDisconnected(const uint8_t arg_ID);

  // get
  inline cTallyClient *GetClientByID(const uint8_t arg_ID)
  {
    return &m_Clients[arg_ID];
  }

  // other stuff
  inline uint8_t GetMaxClients(void) const
  {
    return static_cast<uint8_t>(MAX_CLIENTS);
  }

  template<uint8_t T_PAYLOAD_SIZE>
  void Broadcast(uint8_t arg_Payload[T_PAYLOAD_SIZE])
  {
    for( uint8_t i = 0u; i < GetMaxClients(); ++i )
    {
      // send
      cTallyClient *loc_ptr_Client = GetClientByID(i);

      if( loc_ptr_Client->IsAvailable() )
      {
        ZBTxRequest& loc_TX_Frame = loc_ptr_Client->GetTXFrame();

        loc_TX_Frame.setPayload(arg_Payload, sizeof(arg_Payload));

        xbee.send(loc_TX_Frame);
      }
    }
  }

private:
  cClientManager(void);

  // management
  cTallyClient *GetNextFreeClientSlot(void);

  //! list of all client objects
  cTallyClient m_Clients[MAX_CLIENTS];
  //! client counter
  uint8_t m_ClientCount;

  //! singleton instance
  static cClientManager m_Instance;
};

#endif

