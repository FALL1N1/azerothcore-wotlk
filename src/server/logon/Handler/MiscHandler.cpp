#include "Common.h"
#include "Language.h"
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "Logon.h"
#include "BigNumber.h"
#include "SHA1.h"
#include "zlib.h"
#include "Player.h" 
#include "AccountMgr.h"
#include "ObjectMgr.h"
#include "ClusterDefines.h"
#include "RoutingHelper.h"
#include "ClientSession.h"
#include "SocialMgr.h"
#include "Player.h"

void ClientSession::HandleWorldStateUITimerUpdate(WorldPacket& /*recv_data*/)
{
    // empty opcode
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_WORLD_STATE_UI_TIMER_UPDATE");

    WorldPacket data(SMSG_WORLD_STATE_UI_TIMER_UPDATE, 4);
    data << uint32(time(NULL));
    SendPacket(&data);
}

void ClientSession::HandleWhoisOpcode(WorldPacket& recv_data)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "Received opcode CMSG_WHOIS");
    std::string charname;
    recv_data >> charname;

    if (!AccountMgr::IsAdminAccount(GetSecurity()))
    {
        //SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);
        return;
    }

    if (charname.empty() || !normalizePlayerName (charname))
    {
        //SendNotification(LANG_NEED_CHARACTER_NAME);
        return;
    }

    Player* player = sObjectMgr->FindPlayerByName(charname.c_str());

    if (!player)
    {
        //SendNotification(LANG_PLAYER_NOT_EXIST_OR_OFFLINE, charname.c_str());
        return;
    }

    uint32 accid = player->GetSession()->GetAccountId();

    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_WHOIS);

    stmt->setUInt32(0, accid);

    PreparedQueryResult result = LoginDatabase.Query(stmt);

    if (!result)
    {
        //SendNotification(LANG_ACCOUNT_FOR_PLAYER_NOT_FOUND, charname.c_str());
        return;
    }

    Field* fields = result->Fetch();
    std::string acc = fields[0].GetString();
    if (acc.empty())
        acc = "Unknown";
    std::string email = fields[1].GetString();
    if (email.empty())
        email = "Unknown";
    std::string lastip = fields[2].GetString();
    if (lastip.empty())
        lastip = "Unknown";

    std::string msg = charname + "'s " + "account is " + acc + ", e-mail: " + email + ", last ip: " + lastip;

    WorldPacket data(SMSG_WHOIS, msg.size()+1);
    data << msg;
    SendPacket(&data);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "Received whois command from player %s for character %s", GetPlayer()->GetPlayerName().c_str(), charname.c_str());
}


void ClientSession::Handle_NODE_PLAYER_DATA(WorldPacket& data)
{
    uint32 action;
    data >> action;

    /*switch (action)
    {
        case CL_DEF_COMPLETED_ACHIEVEMENT:
            uint32 id;
            data >> id;
            if(AchievementEntry const *achieve = GetAchievementStore()->LookupEntry(id))
                if (Player *player = GetPlayer())
                    player->CompletedAchievement(achieve);
            break;
        case CL_DEF_RESET_ACHIEVEMENT_CRITERIA:
            uint32 type, miscvalue1, miscvalue2;
            bool evenIfCriteriaComplete;
            data >> type;
            data >> miscvalue1;
            data >> miscvalue2;
            data >> evenIfCriteriaComplete;
            if (Player *player = GetPlayer())
                player->ResetAchievementCriteria(AchievementCriteriaTypes(type), miscvalue1, miscvalue2, evenIfCriteriaComplete);
            break;
        case CL_DEF_RESET_ACHIEVEMENTS:
            if (Player *player = GetPlayer())
                player->ResetAchievement();
            break;
        case CL_DEF_CRITERIA_UPDATE:
            uint32 ID, changeValue;
            uint8 ptype;
            data >> ID;
            data >> changeValue;
            data >> ptype;
            AchievementCriteriaEntry *t_entry = new AchievementCriteriaEntry;
            t_entry->ID = ID;
            if (Player *player = GetPlayer())
                player->SetCriteriaProgress(t_entry, changeValue, ptype);
            delete t_entry;
            break;
    }*/
}

void ClientSession::Handle_NODE_MISC_DATA(WorldPacket& recvPacket)
{
    uint32 cmd, node;
    recvPacket >> cmd;

    if (cmd == CL_DEF_TRANSFER_TO_NODE)
    {
        sLog->outDetail("CL_DEF_TRANSFER_TO_NODE");
        recvPacket >> node;

        if (_nodeId == node)
            return;

        if (!sRoutingHelper->CheckNodeID(node))
            return;

        /*if (sRoutingHelper->GetNodeConnectionData(node).type == NODE_MASTER)
            SetFlag(FLAG_ACCOUNT_WAIT_FOR_OK, true);
        else
            SetFlag(FLAG_TRANSFER_PENDING,true);*/
        
        _nodeId = node;     //Set our NodeID
        CloseSubSocket(); //Close NodeSocket

        _reconnect_timer = 500; //Set the reconnect Timer
        SetFlag(FLAG_ACCOUNT_RECONNECT); //Die Verbindung baut nun der Reconnect auf
    }
}

void ClientSession::HandleSendMail(WorldPacket & recv_data)
{
    std::string receiver, subject, body;

    recv_data.read_skip<uint64>();
    recv_data >> receiver;
    recv_data >> subject;
    recv_data >> body;
    recv_data.rfinish();

    if (GetAntiSpamCounter() > LOGON_DOS_STRIKE_LIMIT || sLogon->CheckForSpam(subject) || sLogon->CheckForSpam(body))
    {
        IncAntiSpamCounter();

        WorldPacket data(SMSG_SEND_MAIL_RESULT, 4+4+4);
        data << (uint32) 0;
        data << (uint32) MAIL_SEND;
        data << (uint32) MAIL_OK;
        SendPacket(&data);
        return;
    }

    SendPacketToNode(&recv_data);
}
