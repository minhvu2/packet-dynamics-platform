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

#ifndef SYMBOLIC_PACKET_LIST_H
#define SYMBOLIC_PACKET_LIST_H

#include <list>
#include <utility>
#include <stdint.h>

#include "object.h"

/**
 * \file
 * \ingroup 
 * Declaration of ns3::SymbolicPacketList class.
 */

namespace ns3 {

/**
 * \ingroup 
 * \brief a std::list symbolic packet
 *
 * This class implements 
 */
class SymbolicPacketList : public Object
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  
  struct symbolicPacket
  {
    uint32_t m_uid;
    uint64_t m_delay;
  };

  /** Constructor. */
  SymbolicPacketList ();
  /** Destructor. */
  virtual ~SymbolicPacketList ();

  void SetList (std::list<uint32_t> packetUidList, std::list<uint64_t> packetDelayList);
//  void SetList (const std::list<uint32_t> &packetList);
  bool IsSymPacket (uint32_t packetUid);
  uint64_t GetSymPacketDelay (uint32_t packetUid);


private:
  /** Pakcet list type: a simple list of Packet Number. */
  typedef std::list<symbolicPacket> PacketList;
  /** Packets iterator. */
  typedef std::list<symbolicPacket>::iterator PacketListI;

  /** The packet list. */
  PacketList m_packetList;
};

} // namespace ns3

#endif /* SYMBOLIC_PACKET_LIST_H */
