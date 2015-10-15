// 
// 
// 

#include <Streaming.h>

#include "ClientManager.h"

cClientManager cClientManager::m_Instance;

cClientManager::cClientManager(void)
  : m_ClientCount(0u)
{
  
}

void cClientManager::Init(void)
{
  for( uint8_t i = 0u; i < GetMaxClients(); ++i )
  {
    GetClientByID(i)->SetID(i);
  }
}

void cClientManager::OnClientConnected(XBeeAddress64& arg_Addr)
{
  cTallyClient *loc_ptr_Client = GetNextFreeClientSlot();

  if( loc_ptr_Client != NULL )
  {
    loc_ptr_Client->SetAddress(arg_Addr);
    loc_ptr_Client->SetAvailable(true);

    ++m_ClientCount;

    Serial << F("ClientManager :: client connected, addr msb=0x") << _HEX(arg_Addr.getMsb()) << F(" lsb=0x") << _HEX(arg_Addr.getLsb())
      << F(", assigned ID=") << loc_ptr_Client->GetID() << F("\n")
      << F("ClientManager :: ") << m_ClientCount << F(" clients connected\n");
  }
  else
  {
    Serial << F("ClientManager :: client refused, slots full\n");
  }
}

void cClientManager::OnClientDisconnected(const uint8_t arg_ID)
{
  cTallyClient *loc_ptr_Client = GetClientByID(arg_ID);

  if( loc_ptr_Client->IsAvailable() )
  {
    XBeeAddress64 loc_Addr = loc_ptr_Client->GetAddress();

    loc_ptr_Client->SetAvailable(false);
    loc_ptr_Client->SetAddress(XBeeAddress64());

    --m_ClientCount;

    Serial << F("ClientManager :: client disconnected, addr msb=0x") << _HEX(loc_Addr.getMsb()) << F(" lsb=0x") << _HEX(loc_Addr.getLsb())
      << F(", ID=") << loc_ptr_Client->GetID() << F("\n")
      << F("ClientManager :: ") << m_ClientCount << F(" clients connected\n");
  }
}

cTallyClient *cClientManager::GetNextFreeClientSlot(void)
{
  for( uint8_t i = 0u; i < GetMaxClients(); ++i )
  {
    if( !GetClientByID(i)->IsAvailable() )
    {
      return GetClientByID(i);
    }
  }

  return NULL; // all slots in use
}
