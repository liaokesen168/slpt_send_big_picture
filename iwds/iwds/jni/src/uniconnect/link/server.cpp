/*
 *  Copyright (C) 2014 Ingenic Semiconductor
 *
 *  SunWenZhong(Fighter) <wenzhong.sun@ingenic.com, wanmyqawdr@126.com>
 *
 *  Elf/IDWS Project
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#include <uniconnect/drivers/datachannel.h>
#include <uniconnect/link.h>

#include "./server.h"


using std::string;

using Iwds::Mutex;
using Iwds::Thread;
using Iwds::Condition;
using Iwds::Log;
using Iwds::Assert;

#define LOG_TAG                                                 \
            (string("Uniconnect: Link: Server: ") +             \
                                m_link->trait()->tag())

class WaitConnectWatchDog : public Thread
{
public:
    WaitConnectWatchDog(Link *link) :
        m_link(link),
        m_counter(0)
    {
        start();
    }

    ~WaitConnectWatchDog()
    {
        requestExitAndWait();
    }

protected:
    bool run()
    {
        sleep(1);

        if (m_counter++ == 20) {
            Log::e(LOG_TAG, "Wait for connect watch dog reset for 20s timeout.");

            m_link->serverSideDisconnect();

            return false;
        }

        return true;
    }

private:
    Link *m_link;
    int m_counter;
};

Server::Server(Link *link) :
    m_link(link),
    m_wait(),
    m_lock(),
    m_startupFlag(false)
{

}

Server::~Server()
{

}

bool Server::start()
{
    Mutex::Autolock l(&m_lock);

    if (!Thread::start())
        return false;

    while (!m_startupFlag) {
        if (!m_wait.wait(&m_lock, 5000)) {
            setErrorString("startup thread timeout for 5s");

            return false;
        }
    }

    m_startupFlag = false;

    return true;
}

bool Server::readyToRun()
{
    if (!m_link->listenerThreadReadyToRun()) {
        Log::e(LOG_TAG, string("failed to start thread: ") +
                                            m_link->errorString());
        return false;
    }

    {
        Mutex::Autolock l(&m_lock);
        m_startupFlag = true;
        m_wait.signal();
    }

    return true;
}

void Server::atExit()
{
    m_link->listenerThreadAtExit();
}

bool Server::run()
{
    switch (m_link->getState()) {
    case LinkState::STATE_DISCONNECTED:
        if (!m_link->readyForOperation()) {
            Log::d(LOG_TAG, string("Not ready for operation: ") +
                                                m_link->errorString());
            Thread::sleep(1);

            break;
        }

        m_link->setState(LinkState::STATE_CONNECTING);

        break;

    case LinkState::STATE_CONNECTING:
    {
        WaitConnectWatchDog dog(m_link);

        if (!m_link->waitForConnect()) {
            /*
             * Connecting failed
             */
            Log::e(LOG_TAG, string("Wait for connect failed: ") +
                                                m_link->errorString());
            Thread::sleep(1);

            m_link->setState(LinkState::STATE_DISCONNECTED);

            break;
        }

        /*
         * start RX/TX
         */
        bool ok = m_link->startReader();

        Assert::dieIf(!ok, m_link->errorString());

        m_link->setState(LinkState::STATE_CONNECTED);

        break;
    }

    case LinkState::STATE_CONNECTED:
        /*
         * Notify link connected
         */
        m_link->notifyLinkConnected();

        /*
         * Wait RX/TX stop
         */
        m_link->waitReaderStop();

        /*
         * Force server side disconnect
         */
        m_link->serverSideDisconnect();

        /*
         * Notify link disconnected
         */
        m_link->notifyLinkDisconnected();

        break;

    default:
        Assert::dieIf(true, "Implement me.");

        break;
    }

    return true;
}

