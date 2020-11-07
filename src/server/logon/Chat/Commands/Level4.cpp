#include "Chat.h"
#include "Logon.h"
#include "ClientSession.h"
#include "ClientSessionMgr.h" 
#include "revision.h"
#include "Language.h"
#include "RoutingHelper.h"
#include "AccountMgr.h"


bool ChatHandler::HandleServerInfoCommand(const char *args)
{
    uint32 playersNum = sClientSessionMgr->GetPlayerCount();
    uint32 maxPlayersNum = sClientSessionMgr->GetMaxPlayerCount();
    uint32 activeClientsNum = sClientSessionMgr->GetActiveSessionCount();
    uint32 queuedClientsNum = sClientSessionMgr->GetQueuedSessionCount();
    std::string uptime = secsToTimeString(sLogon->GetUptime());
    uint32 updateTime = sLogon->GetUpdateTime();
    uint32 nodecount = sRoutingHelper->GetNodeCount();

    PSendSysMessage(LANG_CONNECTED_PLAYERS, playersNum, maxPlayersNum);
    PSendSysMessage(LANG_UPDATE_DIFF, updateTime);
    PSendSysMessage("***********************");
    PSendSysMessage("Nodes: %u", nodecount);
    PSendSysMessage("***********************");
    for (uint32 i = 1; i != nodecount + 1; i++)
    {
        std::stringstream ss;
        bool online = sRoutingHelper->CheckNodeID(i);
        ss << "Node:  " << i;
        ss << " is ";
        if (online)
            ss << "online";
        else
            ss << "offline";
        PSendSysMessage("%s", ss.str().c_str());
    }
    PSendSysMessage("***********************");

    return true;
}

bool ChatHandler::HandleHaltServerCommand(const char *args)
{
    PSendSysMessage("Command");

    return true;
}

bool ChatHandler::HandleAnnounceCommand(const char *args)
{
    if (!*args)
        return false;

    std::stringstream ss;
    ss << "[CAROLINE] " << args;
    sClientSessionMgr->SendServerMessage(SERVER_MSG_STRING, ss.str().c_str());
    return true;
}

bool ChatHandler::HandleServerShutDownCommand(const char *args)
{
    if (!*args)
        return false;

    char* time_str = strtok ((char*) args, " ");
    char* exitcode_str = strtok (NULL, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if ((time == 0 && (time_str[0] != '0' || time_str[1] != '\0')) || time < 0)
        return false;

    if (exitcode_str)
    {
        int32 exitcode = atoi (exitcode_str);

        // Handle atoi() errors
        if (exitcode == 0 && (exitcode_str[0] != '0' || exitcode_str[1] != '\0'))
            return false;

        // Exit code should be in range of 0-125, 126-255 is used
        // in many shells for their own return codes and code > 255
        // is not supported in many others
        if (exitcode < 0 || exitcode > 125)
            return false;

        sLogon->ShutdownServ(time, 0, exitcode);
    }
    else
        sLogon->ShutdownServ(time, 0, SHUTDOWN_EXIT_CODE);
    return true;
}

bool ChatHandler::HandleServerShutDownCancelCommand(const char *args)
{
    sLogon->ShutdownCancel();
    return true;
}
