#ifndef __CLIENTSESSION_H
#define __CLIENTSESSION_H

#include <atomic>

#include "Logon.h" 
#include "DatabaseEnv.h"
#include "ByteBuffer.h"
#include "Logon.h"

//DEPS
#include "Groups.h"

class WorldPacket;
class ByteBuffer;
class ClientSocket;
class NodeSocket;
class Player;
class LoginQueryHolder;
class Warden;

enum AccountDataType
{
    GLOBAL_CONFIG_CACHE             = 0,                    // 0x01 g
    PER_CHARACTER_CONFIG_CACHE      = 1,                    // 0x02 p
    GLOBAL_BINDINGS_CACHE           = 2,                    // 0x04 g
    PER_CHARACTER_BINDINGS_CACHE    = 3,                    // 0x08 p
    GLOBAL_MACROS_CACHE             = 4,                    // 0x10 g
    PER_CHARACTER_MACROS_CACHE      = 5,                    // 0x20 p
    PER_CHARACTER_LAYOUT_CACHE      = 6,                    // 0x40 p
    PER_CHARACTER_CHAT_CACHE        = 7,                    // 0x80 p
};

#define NUM_ACCOUNT_DATA_TYPES        8

#define GLOBAL_CACHE_MASK           0x15
#define PER_CHARACTER_CACHE_MASK    0xEA

struct AccountData
{
    AccountData() : Time(0), Data("") {}

    time_t Time;
    std::string Data;
};

enum SessionFlags
{
    FLAG_ACCOUNT_RECONNECT      = 0x00000001,
    FLAG_ACCOUNT_WAIT_FOR_OK    = 0x00000002,
    FLAG_LOGOUT_REQUEST         = 0x00000004,
    FLAG_LOGOUT                 = 0x00000008,
    FLAG_TRANSFER_PENDING       = 0x00000010,
    FLAG_FORCE_TELEPORT         = 0x00000040,
    FLAG_INFORMED_SERVER_DOWN   = 0x00000080,
    FLAG_ACCOUNT_KICKED         = 0x00000100,
};

//Temp use
struct ChannelT
{
    uint32 Channel_ID;
    std::string ChannelName;
    std::string ChannelPass;
};
typedef std::unordered_map<uint32, ChannelT> ChannelList;

class ClientSession
{
public:
    ClientSession(uint32 id, ClientSocket *sock, AccountTypes sec, uint8 expansion, time_t mute_time, LocaleConstant locale, uint32 recruiter, bool isRecruiter);
    ~ClientSession();

    ACE_Event_Handler::Reference_Count add_reference();
    ACE_Event_Handler::Reference_Count remove_reference();

    /*********************************************************/
    /***                  ACCOUNT SETTINGS                 ***/
    /*********************************************************/
    void LoadGlobalAccountData();
    void LoadAccountData(PreparedQueryResult result, uint32 mask);
    void SendAccountDataTimes(uint32 mask);
    void SetAccountData(AccountDataType type, time_t tm, std::string data);
    uint8 Expansion() const { return _expansion; }
    AccountData* GetAccountData(AccountDataType type) { return &_accountData[type]; }
    uint32 GetAccountId() const { return _accountId; }
    std::string const& GetRemoteAddress() { return _clientaddress; }
    AccountTypes GetSecurity() const { return _security; }
    bool IsAuthed() { return _authed; }
    void SetAuthed(bool sw) { _authed = sw; }
    void SetInQueue(bool state) { _inQueue = state; }
    time_t                                                  _muteTime;

    void SetBanned(bool yes)
    {
        KickPlayer();
        _banned = true;
    } 
    /*********************************************************/
    /***                  I/O HANDLING                     ***/
    /*********************************************************/

    enum PacketProcedure
    {
        PACKET_SEND_CLIENT = 0x1,
        PACKET_SEND_NODE = 0x2,
        PACKET_PROCESS = 0x4
    };

    void HandlePacket(WorldPacket *packet, uint16 opcode, uint8 procedure);

    void UpdateSingleNodeIO(uint32 diff);
    void UpdateSingleClientIO(uint32 diff);

    bool UpdateSingle(uint32 diff);

    void UpdateMultiNodeIO(uint32 diff);
    void UpdateMultiClientIO(uint32 diff);

    bool UpdateMulti(uint32 diff);

    void CloseSocket();
    void CloseSubSocket();

    /*********************************************************/
    /***                 SEND HANDLING                     ***/
    /*********************************************************/
    void SendPacket(WorldPacket const* packet);
    void SendPacketToNode(WorldPacket const* packet);

    void SendAuthWaitQue(uint32 position);
    void SendCommandStringToNode(const char *args, ...) ATTR_PRINTF(2, 3);
    void SendTutorialsData();
    void SendAuthResponse(uint8 code, bool shortForm, uint32 queuePos = 0);
    void SendClientCacheVersion(uint32 version);
    void SendPlayerNotFoundNotice(std::string name);
    void SendWrongFactionNotice(); 
    void SendPartyResult(PartyOperation operation, const std::string& member, PartyResult res, uint32 val = 0);
    void SendPlayerLogin();

    /*********************************************************/
    /***                  PLAYER STUFF                     ***/
    /*********************************************************/
    void KickPlayer();
    void PlayerLogout();
    void SetPlayer(Player *plr);
    Player* GetPlayer() const { return _player; }
    uint64 GetGuid() { return _GUID; }

    /*********************************************************/
    /***                      FLAGS                        ***/
    /*********************************************************/
    void SetFlag(uint32 newFlag);
    void RemoveFlag(uint32 oldFlag);

    void ToggleFlag(uint32 flag)
    {
        if (HasFlag(flag))
            RemoveFlag(flag);
        else
            SetFlag(flag);
    }

    bool HasFlag(uint32 flag) const { return (_sessionflags & flag) != 0; }

    /*********************************************************/
    /***                    LOCALES                        ***/
    /*********************************************************/
    LocaleConstant GetSessionDbcLocale() const { return _sessionDbcLocale; }
    LocaleConstant GetSessionDbLocaleIndex() const { return _sessionDbLocaleIndex; }

    /*********************************************************/
    /***                   TUTORIALS                       ***/
    /*********************************************************/
    void LoadTutorialsData();
    uint32 GetTutorialInt(uint32 intId)
    {
        return _Tutorials[intId];
    }

    void SetTutorialInt(uint32 intId, uint32 value)
    {
        if (_Tutorials[intId] != value)
        {
            _Tutorials[intId] = value;
            _TutorialsChanged = true;
        }
    }

    void SaveTutorialsData(SQLTransaction& trans);
     

    void SetStunned(bool apply);


    //The OpcodeHandling
public:
    void Handle_NULL(WorldPacket& recvPacket);
    void Handle_ServerSide(WorldPacket& recvPacket);
    void HandleCharEnumOpcode(WorldPacket& recvPacket); //Dummy
    void HandleCharEnum(PreparedQueryResult result);
    void HandlePlayerLoginOpcode(WorldPacket& recvPacket);
    void HandlePlayerLogin(LoginQueryHolder * holder);
    void HandlePlayerLogoutRequest(WorldPacket& recv_data);
    void HandleLogoutCancelOpcode(WorldPacket& recv_data); 
    void HandleWorldStateUITimerUpdate(WorldPacket& /*recv_data*/);
 
    //Groups
    void HandleGroupInviteOpcode(WorldPacket& recvPacket);
 
    /************************************************************\
    |******************** SocialHandler stuff *******************|
    \************************************************************/
    void HandleContactListOpcode(WorldPacket& recvPacket);
    void HandleAddFriendOpcode(WorldPacket& recvPacket);
    void HandleAddFriendOpcodeCallBack(PreparedQueryResult result, std::string friendNote);
    void HandleDelFriendOpcode(WorldPacket& recvPacket);
    void HandleAddIgnoreOpcode(WorldPacket& recvPacket);
    void HandleAddIgnoreOpcodeCallBack(PreparedQueryResult result);
    void HandleDelIgnoreOpcode(WorldPacket& recvPacket);
    void HandleSetContactNotesOpcode(WorldPacket& recvPacket);

    //ClientHandlings
public: 
    void HandleWhoisOpcode(WorldPacket& recv_data);
    void HandleSendMail(WorldPacket & recv_data); 

    //NodeHandlings
public:
    void Handle_SMSG_LOGOUT_RESPONSE(WorldPacket& recvPacket);
    void Handle_SMSG_LOGOUT_COMPLETE(WorldPacket& recvPacket);
    void Handle_SMSG_AUTH_RESPONSE(WorldPacket& recvPacket);
    void Handle_SMSG_LEVELUP_INFO(WorldPacket& recvPacket);
    void Handle_SMSG_AUTH_CHALLENGE(WorldPacket& recvPacket);

    //Caching
    void HandleSMSG_GAMEOBJECT_QUERY(WorldPacket & recv_data);
    void HandleSMSG_CREATURE_QUERY(WorldPacket & recv_data);
    void HandleSMSG_ITEM_QUERY_SINGLE(WorldPacket & recv_data);

    void Handle_NODE_PLAYER_DATA(WorldPacket& recvPacket);
    void Handle_NODE_PLAYER_CHANGED_ZONE(WorldPacket& recvPacket);

    void Handle_NODE_TRANSFER_ACK(WorldPacket& recvPacket);

    void Handle_NODE_MISC_DATA(WorldPacket& recvPacket);

    //Characterthings
public:
    void ProcessQueryCallbacks();
    PreparedQueryResultFuture _charEnumCallback;
    QueryResultHolderFuture _charLoginCallback;
    QueryCallback<PreparedQueryResult, std::string> _addFriendCallback;
    PreparedQueryResultFuture _addIgnoreCallback;

    bool CharCanLogin(uint32 lowGUID) { return _allowedCharsToLogin.find(lowGUID) != _allowedCharsToLogin.end(); }
    std::set<uint32> _allowedCharsToLogin;

private:

    ///- Sockets
    ClientSocket *_ClientSocket;
    NodeSocket   *_NodeSocket;

    uint32 _reconnect_timer;
    uint32 _callback_timer;

    ///- ACCOUNT SETTINGS
    std::atomic<uint32>     _sessionflags;
    uint32                  _accountId;
    std::string             _clientaddress;
    uint32                  _nodeId;
    bool                    _inQueue;
    bool                    _banned;
    bool                    _authed;
    uint8                   _expansion;
    AccountTypes            _security;
    AccountData             _accountData[NUM_ACCOUNT_DATA_TYPES]; 

    ///- TUTORIAL SETTINGS
    bool                    _TutorialsChanged;
    uint32                  _Tutorials[MAX_ACCOUNT_TUTORIAL_VALUES];

    ///- LOCALES SETTINGS
    LocaleConstant          _sessionDbcLocale;
    LocaleConstant          _sessionDbLocaleIndex;
     
    //Temp
    ChannelList m_chlist;

    //Player
    uint64 _GUID;
    uint32 _GUIDLow;
    Player *_player;
    bool m_playerLoading;
    uint32 m_loginDelay;
    uint32 m_DoSStrikes;

public:
    void IncAntiSpamCounter() { _antiSpam++; }
    uint8 GetAntiSpamCounter() { return _antiSpam; }

    void SendFakeChatNotification(uint8 type, std::string msg, uint32 lang);
    void SendFakeChannelNotification(std::string channel, std::string msg, uint32 lang);

private:
    uint8 _antiSpam;
    ACE_Event_Handler::Reference_Count _referenceCount;
};

#endif
