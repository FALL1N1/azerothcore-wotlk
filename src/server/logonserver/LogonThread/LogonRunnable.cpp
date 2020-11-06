/*
 * Copyright (C) 2005-2011 MaNGOS <http://www.getmangos.com/>
 *
 * Copyright (C) 2008-2011 Trinity <http://www.trinitycore.org/>
 *
 * Copyright (C) 2010-2011 Project SkyFire <http://www.projectskyfire.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/** \file
    \ingroup Trinityd
*/

#include "Common.h"
//#include "ObjectAccessor.h"
#include "Logon.h"
#include "Database/DatabaseEnv.h"
#include "Timer.h"
#include "LogonRunnable.h"

#include "ClientSocket.h"
#include "ControlSocket.h"
#include "NodeSocket.h"

#define LOGON_SLEEP_CONST 50

/// Heartbeat for the World
void LogonRunnable::run()
{
    uint32 realCurrTime = 0;
    uint32 realPrevTime = getMSTime();

    uint32 prevSleepTime = 0;                               // used for balanced full tick time length near WORLD_SLEEP_CONST

    ///- While we have not World::m_stopEvent, update the world
    while (!Logon::IsStopped())
    {
        ++Logon::m_logonLoopCounter;
        realCurrTime = getMSTime();

        uint32 diff = getMSTimeDiff(realPrevTime,realCurrTime);

        sLogon->Update( diff );
        realPrevTime = realCurrTime;

        // diff (D0) include time of previous sleep (d0) + tick time (t0)
        // we want that next d1 + t1 == WORLD_SLEEP_CONST
        // we can't know next t1 and then can use (t0 + d1) == WORLD_SLEEP_CONST requirement
        // d1 = WORLD_SLEEP_CONST - t0 = WORLD_SLEEP_CONST - (D0 - d0) = WORLD_SLEEP_CONST + d0 - D0
        if (diff <= LOGON_SLEEP_CONST+prevSleepTime)
        {
            prevSleepTime = LOGON_SLEEP_CONST+prevSleepTime-diff;
            acore::Thread::Sleep(prevSleepTime);
        }
        else
            prevSleepTime = 0;

    }

    //sWorld->KickAll();                                       // save and kick all players
    //sWorld->UpdateSessions( 1 );                             // real players unload required UpdateSessions call

    sClientSocketAcceptor->close();
    sControlSocketConnector->close();
    sNodeSocketConnector->close();

    sNetworkMgr->close();
}
