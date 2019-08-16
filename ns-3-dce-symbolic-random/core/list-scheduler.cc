/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "list-scheduler.h"
#include "event-impl.h"
#include "log.h"
#include <utility>
#include <string>
#include "assert.h"

#include "s2e.h"
#include <cstdlib>
#include <cstdio>
//#include <ctime>
#include <sys/time.h>

/**
 * \file
 * \ingroup scheduler
 * ns3::ListScheduler implementation.
 */

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ListScheduler");

NS_OBJECT_ENSURE_REGISTERED (ListScheduler);

TypeId
ListScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ListScheduler")
    .SetParent<Scheduler> ()
    .SetGroupName ("Core")
    .AddConstructor<ListScheduler> ()
  ;
  return tid;
}

ListScheduler::ListScheduler ()
{
  NS_LOG_FUNCTION (this);
}
ListScheduler::~ListScheduler ()
{
}

// <M>
uint64_t ListScheduler::m_originalDelay = 1;
bool ListScheduler::m_useRandomDelay = true;
uint32_t ListScheduler::m_packetUid = 0;
uint32_t ListScheduler::m_numSymPackets = 100;
uint64_t ListScheduler::m_interval = 1024000000;
uint32_t ListScheduler::m_firstSymPacket = 0;
bool ListScheduler::m_usePathReduction = true;

uint32_t ListScheduler::m_packetNumber = 0;  
bool ListScheduler::m_isTransmission = false;
uint32_t ListScheduler::m_packetSize = 0;
ListScheduler::Packets ListScheduler::m_packets;

void
ListScheduler::SetOriginalDelay (uint64_t originalDelay)
{
  m_originalDelay = originalDelay;
  
}

void
ListScheduler::EnableRandomDelay (bool useRandomDelay)
{
  m_useRandomDelay = useRandomDelay;
  
}

void
ListScheduler::SetNumberSymPackets (uint32_t numSymPackets)
{
  m_numSymPackets = numSymPackets;
  
}

void
ListScheduler::SetInterval (uint64_t interval)
{
  m_interval = interval;
}

void
ListScheduler::SetFirstSymPacket (uint32_t firstSymPacket)
{
  m_firstSymPacket = firstSymPacket;
}

void
ListScheduler::SetPacketUid (uint32_t packetUid)
{
  m_packetUid = packetUid;
}

void
ListScheduler::SetTransmission (bool value)
{
  m_isTransmission = value;
}

void
ListScheduler::SetPacketSize (uint32_t packetSize)
{
  m_packetSize = packetSize;
}

void
ListScheduler::PrintPacketInfo (void)
{
  char buf[64];
  memset (buf, 0, sizeof(buf));
  
  for (PacketsI i = m_packets.begin(); i != m_packets.end(); i++)
    {
      if (i->m_isSymbolic)
        {
          snprintf (buf, sizeof(buf), "Packetuid %u  Delay 0", i->m_uid);
        }
      else
        {
          snprintf (buf, sizeof(buf), "Packetuid %u  Delay %lu", i->m_uid, i->m_delay);
        }
        
      s2e_warning (buf);
      memset (buf, 0, sizeof(buf));
    }

  snprintf (buf, sizeof(buf), "Packetuid 123456789  Delay 123456789");
  s2e_warning (buf);
  memset (buf, 0, sizeof(buf));
}


void
ListScheduler::Insert (const Event &ev)
{
  NS_LOG_FUNCTION (this << &ev);

  // <M>
  if (m_isTransmission)
    {
      m_packetNumber++;
      const_cast<Event&>(ev).key.m_isTransmission = true;
      const_cast<Event&>(ev).key.m_packetSize = m_packetSize;

      // save packet info to print out later
      PacketInfo pInfo;
      pInfo.m_uid = m_packetUid;

      if (m_packetNumber >= m_firstSymPacket && m_numSymPackets > 0 && ev.key.m_packetSize > 60)
        {
          m_numSymPackets--;

//          uint64_t sym_ts;
//          s2e_make_symbolic (&sym_ts, sizeof (uint64_t), "Symbolic Delay");
//          if (sym_ts > m_interval)
//            {
//              s2e_kill_state (0, "Out of Range");
//            }
//          const_cast<Event&>(ev).key.m_ts += sym_ts;
          s2e_make_symbolic (&pInfo.m_delay, sizeof (uint64_t), "Symbolic Delay");
          pInfo.m_isSymbolic = true;
          if (pInfo.m_delay > m_interval)
            {
              s2e_kill_state (0, "Out of Range");
            }
          const_cast<Event&>(ev).key.m_ts += pInfo.m_delay + 1 - m_originalDelay;  
        }
      else  // not a symbolic packet
        {
          pInfo.m_isSymbolic = false;
          if (m_useRandomDelay)  // random delay
            {
              struct timeval time;
              gettimeofday (&time, NULL);
              srand ((time.tv_sec*1000) + (time.tv_usec/1000));
              pInfo.m_delay = rand () % m_interval; // random delay
              const_cast<Event&>(ev).key.m_ts += pInfo.m_delay + 1 - m_originalDelay;
            }
          else // original delay
            {
              pInfo.m_delay = m_originalDelay;
            }
        }

      m_packets.push_back (pInfo);
      SetTransmission (false);
    }
  else
    {
      const_cast<Event&>(ev).key.m_isTransmission = false;
      const_cast<Event&>(ev).key.m_packetSize = 0;
    }  
  
  if (m_usePathReduction)
  {
     for (EventsI i = m_events.begin (); i != m_events.end (); i++)
      {
        if (ev.key.m_uid < i->key.m_uid)
          {
            if (ev.key.m_ts <= i->key.m_ts)
              {
                m_events.insert (i, ev);
                return;
              }
          }
          else // ev.id > i.id
            {
              if (ev.key.m_ts < i->key.m_ts)
                {
                  m_events.insert (i, ev);
                  return;
                }
            }
        }
 
  }
  else
  {
    for (EventsI i = m_events.begin (); i != m_events.end (); i++)
      {
        if (ev.key < i->key)
          {
            m_events.insert (i, ev);
            return;
          }
      }
  }

  m_events.push_back (ev);
}

bool
ListScheduler::IsEmpty (void) const
{
  NS_LOG_FUNCTION (this);
  return m_events.empty ();
}

Scheduler::Event
ListScheduler::PeekNext (void) const
{
  NS_LOG_FUNCTION (this);
  return m_events.front ();
}

Scheduler::Event
ListScheduler::RemoveNext (void)
{
  NS_LOG_FUNCTION (this);
  Event next = m_events.front ();
  m_events.pop_front ();
  return next;
}

void
ListScheduler::Remove (const Event &ev)
{
  NS_LOG_FUNCTION (this << &ev);
  for (EventsI i = m_events.begin (); i != m_events.end (); i++)
    {
      if (i->key.m_uid == ev.key.m_uid)
        {
          NS_ASSERT (ev.impl == i->impl);
          m_events.erase (i);
          return;
        }
    }
  NS_ASSERT (false);
}

} // namespace ns3
