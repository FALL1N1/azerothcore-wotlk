#ifndef _OBJECTMGR_H
#define _OBJECTMGR_H

#include "Player.h"

enum TypeID
{
    TYPEID_OBJECT        = 0,
    TYPEID_ITEM          = 1,
    TYPEID_CONTAINER     = 2,
    TYPEID_UNIT          = 3,
    TYPEID_PLAYER        = 4,
    TYPEID_GAMEOBJECT    = 5,
    TYPEID_DYNAMICOBJECT = 6,
    TYPEID_CORPSE        = 7
};

#define NUM_CLIENT_OBJECT_TYPES             8

#define MAX_PLAYER_NAME          12                         // max allowed by client name length
#define MAX_INTERNAL_PLAYER_NAME 15                         // max server internal player name length (> MAX_PLAYER_NAME for support declined names)
#define MAX_PET_NAME             12                         // max allowed by client name length
#define MAX_CHARTER_NAME         24                         // max allowed by client name length

bool normalizePlayerName(std::string& name);
 
struct TrinityStringLocale
{
    StringVector Content;
};


enum ScriptsType
{
    SCRIPTS_FIRST = 1,

    SCRIPTS_SPELL = SCRIPTS_FIRST,
    SCRIPTS_EVENT,
    SCRIPTS_WAYPOINT,

    SCRIPTS_LAST
};

enum eScriptFlags
{
    // Talk Flags
    SF_TALK_USE_PLAYER = 0x1,

    // Emote flags
    SF_EMOTE_USE_STATE = 0x1,

    // TeleportTo flags
    SF_TELEPORT_USE_CREATURE = 0x1,

    // KillCredit flags
    SF_KILLCREDIT_REWARD_GROUP = 0x1,

    // RemoveAura flags
    SF_REMOVEAURA_REVERSE = 0x1,

    // CastSpell flags
    SF_CASTSPELL_SOURCE_TO_TARGET = 0,
    SF_CASTSPELL_SOURCE_TO_SOURCE = 1,
    SF_CASTSPELL_TARGET_TO_TARGET = 2,
    SF_CASTSPELL_TARGET_TO_SOURCE = 3,
    SF_CASTSPELL_SEARCH_CREATURE = 4,
    SF_CASTSPELL_TRIGGERED = 0x1,

    // PlaySound flags
    SF_PLAYSOUND_TARGET_PLAYER = 0x1,
    SF_PLAYSOUND_DISTANCE_SOUND = 0x2,

    // Orientation flags
    SF_ORIENTATION_FACE_TARGET = 0x1,
};


typedef std::unordered_map<int32, TrinityStringLocale> TrinityStringLocaleContainer;

// Trinity string ranges
#define MIN_TRINITY_STRING_ID           1                    // 'trinity_string'
#define MAX_TRINITY_STRING_ID           2000000000

typedef std::unordered_map<uint64, Player*> PlayerMap;

class ObjectMgr
{
    public:
        void Player_Add(Player* player);
        void Player_Remove(Player* player);

        typedef std::vector<std::string> ScriptNameContainer;
        Player* GetPlayer(uint64 GUID);
        Player* FindPlayer(uint64 GUID);
        Player* FindPlayerInOrOutOfWorld(uint64 GUID);
        Player* FindPlayerByName(const char* name);
        Player* FindPlayerByName(std::string name);
        bool GetPlayerNameByGUID(uint64 guid, std::string &name) const;
        uint64 GetPlayerGUIDByName(std::string name) const;
        uint32 GetPlayerAccountIdByGUID(uint64 guid) const;
        uint32 GetPlayerAccountIdByPlayerName(const std::string& name) const;
  
        void Update();

        //GUID_Handlings
        void SetHighestGuids();
        uint32 IncreaseGroupId();
        
        void LoadScriptNames();
        ScriptNameContainer &GetScriptNames() { return _scriptNamesStore; }
        const char * GetScriptName(uint32 id) const { return id < _scriptNamesStore.size() ? _scriptNamesStore[id].c_str() : ""; }
        uint32 GetScriptId(const char *name);

        // first free id for selected id type
        uint32 m_auctionid;
        uint64 m_equipmentSetGuid;
        uint32 m_ItemTextId;
        uint32 m_mailid;
        uint32 m_hiPetNumber;

        // first free low guid for seelcted guid type
        uint32 m_hiCharGuid;
        uint32 m_hiCreatureGuid;
        uint32 m_hiPetGuid;
        uint32 m_hiVehicleGuid;
        uint32 m_hiItemGuid;
        uint32 m_hiGoGuid;
        uint32 m_hiDoGuid;
        uint32 m_hiCorpseGuid;
        uint32 m_hiMoTransGuid;
        uint32 m_hiGroupGuid;

    private:
        ACE_Thread_Mutex                    i_objectLock;
        ACE_Based::LockedQueue<Player*, ACE_RW_Mutex> _NewPlayer;
        ACE_Based::LockedQueue<Player*, ACE_RW_Mutex> _DelPlayer;
        PlayerMap           m_PlayerMap;

        LocaleConstant DBCLocaleIndex;
        TrinityStringLocaleContainer _trinityStringLocaleStore;
        ScriptNameContainer _scriptNamesStore;
        
        
        void LoadScripts(ScriptsType type);
        void CheckScripts(ScriptsType type, std::set<int32>& ids);

        //PlayerZeugs 
};
#define sObjectMgr ACE_Singleton<ObjectMgr, ACE_Null_Mutex>::instance()
#endif
