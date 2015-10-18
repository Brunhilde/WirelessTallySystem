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
  void OnClientDataReceived(XBeeAddress64& arg_Addr, uint8_t *arg_ptr_Data, uint8_t arg_DataLength);

  // get
  inline cTallyClient *GetClientByID(const uint8_t arg_ID)
  {
    return &m_Clients[arg_ID];
  }

  cTallyClient *GetClientByAddress(XBeeAddress64& arg_Addr);
  cTallyClient *GetClientBySourceIndex(uint16_t arg_idx_Source);

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
        loc_ptr_Client->Send(arg_Payload, sizeof(arg_Payload));
      }
    }
  }

  void SetClientTallyState(uint16_t arg_idx_Source, uint8_t arg_TallyState);
  void InformClientsSync(void);

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

