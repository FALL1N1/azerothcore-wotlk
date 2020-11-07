#include "Creatures.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "ClientSession.h"
#include "LocalesMgr.h"

void Creatures::WarmingCache()
{
 
}

CreatureTemplate const* Creatures::GetCreatureTemplate(uint32 entry)
{
    CreatureTemplateContainer::const_iterator itr = _creatureTemplateStore.find(entry);
    if (itr != _creatureTemplateStore.end())
        return &(itr->second);

    return NULL;
}

void Creatures::CacheCreatureTemplate(CreatureTemplate* creature)
{
    //To prevent a crash we should set an Mutex here
    ACORE_WRITE_GUARD(ACE_RW_Thread_Mutex, rwMutex_);

    CreatureTemplate& cre = _creatureTemplateStore[creature->entry];
    cre = *creature;
    return;
}

/// Only _static_ data is sent in this packet !!!
void ClientSession::HandleCreatureQueryOpcode(WorldPacket & recv_data)
{
    uint32 entry;
    recv_data >> entry;
    uint64 guid;
    recv_data >> guid;

    CreatureTemplate const* ci = sCreatures->GetCreatureTemplate(entry);
    if (ci)
    {

        std::string Name, SubName;
        Name = ci->Name;
        SubName = ci->SubName;

        int loc_idx = GetSessionDbLocaleIndex();
        if (loc_idx >= 0)
        {
            if (CreatureLocale const* cl = sLocalesMgr->GetCreatureLocale(entry))
            {
                LocalesMgr::GetLocaleString(cl->Name, loc_idx, Name);
                LocalesMgr::GetLocaleString(cl->SubName, loc_idx, SubName);
            }
        }
        sLog->outDetail("WORLD: CMSG_CREATURE_QUERY '%s' - Entry: %u.", ci->Name.c_str(), entry);
                                                            // guess size
        WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 100);
        data << uint32(entry);                              // creature entry
        data << Name;
        data << uint8(0) << uint8(0) << uint8(0);           // name2, name3, name4, always empty
        data << SubName;
        data << ci->IconName;                               // "Directions" for guard, string for Icons 2.3.0
        data << uint32(ci->type_flags);                     // flags
        data << uint32(ci->type);                           // CreatureType.dbc
        data << uint32(ci->family);                         // CreatureFamily.dbc
        data << uint32(ci->rank);                           // Creature Rank (elite, boss, etc)
        data << uint32(ci->KillCredit[0]);                  // new in 3.1, kill credit
        data << uint32(ci->KillCredit[1]);                  // new in 3.1, kill credit
        data << uint32(ci->Modelid1);                       // Modelid1
        data << uint32(ci->Modelid2);                       // Modelid2
        data << uint32(ci->Modelid3);                       // Modelid3
        data << uint32(ci->Modelid4);                       // Modelid4
        data << float(ci->ModHealth);                       // dmg/hp modifier
        data << float(ci->ModMana);                         // dmg/mana modifier
        data << uint8(ci->RacialLeader);
        for (uint32 i = 0; i < MAX_CREATURE_QUEST_ITEMS; ++i)
            data << uint32(ci->questItems[i]);              // itemId[6], quest drop
        data << uint32(ci->movementId);                     // CreatureMovementInfo.dbc
        SendPacket(&data);
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent SMSG_CREATURE_QUERY_RESPONSE");
    }
    else
        SendPacketToNode(&recv_data);
}

/// Only _static_ data is sent in this packet !!!
void ClientSession::HandleSMSG_CREATURE_QUERY(WorldPacket & recv_data)
{
    uint8 trash;
    uint32 entry;

    recv_data >> entry;

    //Sollte als nicht existent interpretiert werden, denk ich mal
    if (entry >= 0x80000000)
        return;

    CreatureTemplate *creature = new CreatureTemplate;

    creature->entry = entry;                            // creature entry
    recv_data >> creature->Name;
    recv_data >> trash >> trash >> trash;               // name2, name3, name4, always empty
    recv_data >> creature->SubName;
    recv_data >> creature->IconName;                    // "Directions" for guard, string for Icons 2.3.0
    recv_data >> creature->type_flags;                  // flags
    recv_data >> creature->type;                        // CreatureType.dbc
    recv_data >> creature->family;                      // CreatureFamily.dbc
    recv_data >> creature->rank;                        // Creature Rank (elite, boss, etc)
    recv_data >> creature->KillCredit[0];               // new in 3.1, kill credit
    recv_data >> creature->KillCredit[1];               // new in 3.1, kill credit
    recv_data >> creature->Modelid1;                    // Modelid1
    recv_data >> creature->Modelid2;                    // Modelid2
    recv_data >> creature->Modelid3;                    // Modelid3
    recv_data >> creature->Modelid4;                    // Modelid4
    recv_data >> creature->ModHealth;                   // dmg/hp modifier
    recv_data >> creature->ModMana;                     // dmg/mana modifier
    recv_data >> creature->RacialLeader;
    for (uint32 i = 0; i < MAX_CREATURE_QUEST_ITEMS; ++i)
        recv_data >> creature->questItems[i];           // itemId[6], quest drop
    recv_data >> creature->movementId;                  // CreatureMovementInfo.dbc
    
    sCreatures->CacheCreatureTemplate(creature);

}
