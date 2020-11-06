/*
 * Copyright (C) 2016+     AzerothCore <www.azerothcore.org>
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 */

#ifndef _LOGONDATABASE_H
#define _LOGONDATABASE_H

#include "DatabaseWorkerPool.h"
#include "MySQLConnection.h"

class LogonDatabaseConnection : public MySQLConnection
{
    public:
        //- Constructors for sync and async connections
        LogonDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo) {}
        LogonDatabaseConnection(ACE_Activation_Queue* q, MySQLConnectionInfo& connInfo) : MySQLConnection(q, connInfo) {}

        //- Loads database type specific prepared statements
        void DoPrepareStatements();
};

typedef DatabaseWorkerPool<LogonDatabaseConnection> LogonDatabaseWorkerPool;

enum LogonDatabaseStatements
{
    /*  Naming standard for defines:
        {DB}_{SET/DEL/ADD/REP}_{Summary of data changed}
        When updating more than one field, consider looking at the calling function
        name for a suiting suffix.
    */

    LOGON_DEL_REALM_CHARACTERS,
    LOGON_INS_REALM_CHARACTERS,
    LOGON_UPD_LOGON_STATE,
    LOGON_INS_LOGON_STATE,
    LOGON_SEL_COMMANDS,
    LOGON_SEL_NODE_SECURITY_LEVEL,
    MAX_LOGONDATABASE_STATEMENTS,
};

#endif
