/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/** \addtogroup u2w User to World Communication
 *  @{
 *  \file PoolSocketMgr.h
 *  \author Derex <derex101@gmail.com>
 */

#ifndef __POOLSOCKETMGR_H
#define __POOLSOCKETMGR_H

#include <ace/Basic_Types.h>
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>

class PoolSocket;
class ReactorRunnable;
class ACE_Event_Handler;

/// Manages all sockets connected to peers and network threads
class PoolSocketMgr
{
public:
    friend class PoolSocket;
    friend class ACE_Singleton<PoolSocketMgr, ACE_Thread_Mutex>;

    /// Start network, listen at address:port .
    int StartNetwork(ACE_UINT16 port, const char* address);

    /// Stops all network threads, It will wait for all running threads .
    void StopNetwork();

    /// Wait untill all network threads have "joined" .
    void Wait();

private:
    int OnSocketOpen(PoolSocket* sock);

    int StartReactiveIO(ACE_UINT16 port, const char* address);

private:
    PoolSocketMgr();
    virtual ~PoolSocketMgr();

    ReactorRunnable* m_NetThreads;
    size_t m_NetThreadsCount;

    int m_SockOutKBuff;
    int m_SockOutUBuff;
    bool m_UseNoDelay;

    class PoolSocketAcceptor* m_Acceptor;
};

#define sPoolSocketMgr ACE_Singleton<PoolSocketMgr, ACE_Thread_Mutex>::instance()

#endif
/// @}
