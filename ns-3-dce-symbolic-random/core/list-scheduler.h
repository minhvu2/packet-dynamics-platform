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

#ifndef LIST_SCHEDULER_H
#define LIST_SCHEDULER_H

#include "scheduler.h"
#include <list>
#include <utility>
#include <stdint.h>

/**
 * \file
 * \ingroup scheduler
 * ns3::ListScheduler declaration.
 */

namespace ns3 {

class EventImpl;

/**
 * \ingroup scheduler
 * \brief a std::list event scheduler
 *
 * This class implements an event scheduler using an std::list
 * data structure, that is, a double linked-list.
 */
class ListScheduler : public Scheduler
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  /** Constructor. */
  ListScheduler ();
  /** Destructor. */
  virtual ~ListScheduler ();
  
  // <M>
  
  struct PacketInfo
  {
    uint32_t m_uid;
    uint64_t m_delay;
    bool m_isSymbolic;
  };
  
  static void SetOriginalDelay (uint64_t originalDelay);
  static void EnableRandomDelay (bool useRandomDelay);
  static void PrintPacketInfo (void);
  static void SetNumberSymPackets (uint32_t numSymPackets);
  static void SetInterval (uint64_t interval);
  static void SetFirstSymPacket (uint32_t firstSymPacket);
  
  static void SetPacketUid (uint32_t packetId);
  static void SetTransmission (bool value);
  static void SetPacketSize (uint32_t packetSize);

  // Inherited
  virtual void Insert (const Scheduler::Event &ev);
  virtual bool IsEmpty (void) const;
  virtual Scheduler::Event PeekNext (void) const;
  virtual Scheduler::Event RemoveNext (void);
  virtual void Remove (const Scheduler::Event &ev);

private:
  /** Event list type: a simple list of Events. */
  typedef std::list<Scheduler::Event> Events;
  /** Events iterator. */
  typedef std::list<Scheduler::Event>::iterator EventsI;

  /** The event list. */
  Events m_events;
  
  // <M>
  /** Packet list type: to print out packet info */
  typedef std::list<PacketInfo> Packets;
  typedef std::list<PacketInfo>::iterator PacketsI;
  static Packets m_packets;
  static uint64_t m_originalDelay;
  static uint32_t m_packetUid; // packet uid from channel
  static bool m_useRandomDelay;
  
  static uint32_t m_numSymPackets;
  static uint64_t m_interval;
  static uint32_t m_firstSymPacket;
  static bool m_usePathReduction;
  
  static uint32_t m_packetNumber; // use to set symbolic packet
  static bool m_isTransmission;
  static uint32_t m_packetSize;
};

} // namespace ns3

#endif /* LIST_SCHEDULER_H */
