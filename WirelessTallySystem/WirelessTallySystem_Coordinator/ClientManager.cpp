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

    // ask for source index
    uint8_t loc_Buf[] = { 'S' };
    loc_ptr_Client->Send(loc_Buf, sizeof(loc_Buf));

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
    loc_ptr_Client->SetSourceIndex(0u);

    --m_ClientCount;

    Serial << F("ClientManager :: client disconnected, addr msb=0x") << _HEX(loc_Addr.getMsb()) << F(" lsb=0x") << _HEX(loc_Addr.getLsb())
      << F(", ID=") << loc_ptr_Client->GetID() << F("\n")
      << F("ClientManager :: ") << m_ClientCount << F(" clients connected\n");
  }
}

void cClientManager::OnClientDataReceived(XBeeAddress64& arg_Addr, uint8_t *arg_ptr_Data, uint8_t arg_DataLength)
{
  if( arg_DataLength >= 2u )
  {
    uint8_t loc_cmd   = arg_ptr_Data[0];
    uint8_t loc_value = arg_ptr_Data[1];

    cTallyClient *loc_ptr_Client = GetClientByAddress(arg_Addr);

    if( loc_ptr_Client != NULL )
    {
      switch( loc_cmd )
      {
      case 'S':
        {
          // handle source index
          loc_ptr_Client->SetSourceIndex(static_cast<uint16_t>(loc_value - 0x30));

          Serial << F("ClientManager :: client transmitted source index=") << (loc_value - 0x30) << F(", addr msb=0x") << _HEX(loc_ptr_Client->GetAddress().getMsb()) << F(" lsb=0x") << _HEX(loc_ptr_Client->GetAddress().getLsb())
            << F(", ID=") << loc_ptr_Client->GetID() << F("\n");
        }
      break;
      }
    }
  }
}

cTallyClient *cClientManager::GetClientByAddress(XBeeAddress64& arg_Addr)
{
  for( uint8_t i = 0u; i < GetMaxClients(); ++i )
  {
    if( GetClientByID(i)->IsAvailable() )
    {
      if( GetClientByID(i)->GetAddress() == arg_Addr )
      {
        return GetClientByID(i);
      }
    }
  }

  return NULL;
}

cTallyClient *cClientManager::GetClientBySourceIndex(uint16_t arg_idx_Source)
{
  for( uint8_t i = 0u; i < GetMaxClients(); ++i )
  {
    if( GetClientByID(i)->IsAvailable() )
    {
      if( GetClientByID(i)->GetSourceIndex() == arg_idx_Source )
      {
        return GetClientByID(i);
      }
    }
  }

  return NULL;
}

void cClientManager::SetClientTallyState(uint16_t arg_idx_Source, uint8_t arg_TallyState)
{
  cTallyClient *loc_ptr_Client = GetClientBySourceIndex(arg_idx_Source);

  if( loc_ptr_Client != NULL )
  {
    if( arg_TallyState & 0x01 )
    {
      loc_ptr_Client->SetTallyProgram(true);
    }
    else if( !(arg_TallyState & 0x01) )
    {
      loc_ptr_Client->SetTallyProgram(false);
    }
    else
    {
      // NOP
    }
    
    if( arg_TallyState & 0x02 )
    {
      loc_ptr_Client->SetTallyPreview(true);
    }
    else if( !(arg_TallyState & 0x02) )
    {
      loc_ptr_Client->SetTallyPreview(false);
    }
    else
    {
      // NOP
    }
  }
}

void cClientManager::InformClientsSync(void)
{
  for( uint8_t i = 0u; i < GetMaxClients(); ++i )
  {
    if( GetClientByID(i)->IsAvailable() )
    {
      uint8_t loc_Payload1[] = { 'P', (GetClientByID(i)->GetTallyProgram() ? '1' : '0') };
      GetClientByID(i)->Send(loc_Payload1, sizeof(loc_Payload1));

      uint8_t loc_Payload2[] = { 'V', (GetClientByID(i)->GetTallyPreview() ? '1' : '0') };
      GetClientByID(i)->Send(loc_Payload2, sizeof(loc_Payload2));
    }
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
