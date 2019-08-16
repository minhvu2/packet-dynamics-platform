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
 * Author: 
 */

#include "symbolic-packet-list.h"
#include "log.h"
#include <utility>
#include <string>
#include "assert.h"

/**
 * \file
 * \ingroup 
 * Implementation of ns3::SymbolicPacketList class.
 */

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SymbolicPacketList");

NS_OBJECT_ENSURE_REGISTERED (SymbolicPacketList);

TypeId
SymbolicPacketList::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SymbolicPacketList")
    .SetParent<Object> ()
    .SetGroupName ("Core")
    .AddConstructor<SymbolicPacketList> ()
  ;
  return tid;
}

SymbolicPacketList::SymbolicPacketList ()
{
  NS_LOG_FUNCTION (this);
}
SymbolicPacketList::~SymbolicPacketList ()
{
}

void
SymbolicPacketList::SetList (std::list<uint32_t> packetUidList, std::list<uint64_t> packetDelayList)
{
  NS_LOG_FUNCTION (this);
  SymbolicPacketList::symbolicPacket packet;
  while (!packetUidList.empty ()  && !packetDelayList.empty ())
    {
      packet.m_uid = packetUidList.front ();
      packet.m_delay = packetDelayList.front ();
      m_packetList.push_back (packet);
      packetUidList.pop_front ();
      packetDelayList.pop_front ();
    }
}

bool
SymbolicPacketList::IsSymPacket (uint32_t packetUid)
{
  NS_LOG_FUNCTION (this << packetUid);
  if (m_packetList.empty())
    {
      return false;
    }
  for (PacketListI i = m_packetList.begin (); i != m_packetList.end (); i++)
    {
      if (packetUid == i->m_uid)
        {
          return true;
        }
    }
  return false;  
}

uint64_t
SymbolicPacketList::GetSymPacketDelay (uint32_t packetUid)
{
  NS_LOG_FUNCTION (this << packetUid);
  if (m_packetList.empty())
    {
      return 0;
    }
  for (PacketListI i = m_packetList.begin (); i != m_packetList.end (); i++)
    {
      if (packetUid == i->m_uid)
        {
          return i->m_delay;
        }
    }
  return 0;  
}
/*void
SymbolicPacketList::SetList (const std::list<uint32_t> &packetList)
{
  NS_LOG_FUNCTION (this << &packetList);
  m_packetList = packetList;
}

bool
SymbolicPacketList::IsSymPacket (uint32_t packetUid)
{
  NS_LOG_FUNCTION (this << packetUid);
  for (PacketListI i = m_packetList.begin (); i != m_packetList.end (); i++)
    {
      if (packetUid == *i)
        {
          return true;
        }
    }
  return false;  
}*/


} // namespace ns3
