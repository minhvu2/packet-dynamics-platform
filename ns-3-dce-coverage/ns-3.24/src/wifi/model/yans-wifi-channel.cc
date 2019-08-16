/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
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
 * Author: Mathieu Lacage, <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/object-factory.h"
#include "yans-wifi-channel.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"

#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include <cstdlib>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("YansWifiChannel");

NS_OBJECT_ENSURE_REGISTERED (YansWifiChannel);

TypeId
YansWifiChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::YansWifiChannel")
    .SetParent<WifiChannel> ()
    .SetGroupName ("Wifi")
    .AddConstructor<YansWifiChannel> ()
    .AddAttribute ("PropagationLossModel", "A pointer to the propagation loss model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&YansWifiChannel::m_loss),
                   MakePointerChecker<PropagationLossModel> ())
    .AddAttribute ("PropagationDelayModel", "A pointer to the propagation delay model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&YansWifiChannel::m_delay),
                   MakePointerChecker<PropagationDelayModel> ())
    .AddAttribute ("SymbolicPacketList", "List of symbolic packets",
                   PointerValue (),
                   MakePointerAccessor (&YansWifiChannel::m_symPacketList),
                   MakePointerChecker<SymbolicPacketList> ())
    .AddAttribute ("RandomDelay", "Whether random delay is used or not",
                   BooleanValue (false),
                   MakeBooleanAccessor (&YansWifiChannel::m_randomDelay),
                   MakeBooleanChecker ())
    .AddAttribute ("MaxDelay", "Maximum value for the propagation delay",
                   UintegerValue (0),
                   MakeUintegerAccessor (&YansWifiChannel::m_maxDelay),
                   MakeUintegerChecker <uint64_t> ())
  ;
  return tid;
}

YansWifiChannel::YansWifiChannel ()
{
}

YansWifiChannel::~YansWifiChannel ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_phyList.clear ();
}

void
YansWifiChannel::SetPropagationLossModel (Ptr<PropagationLossModel> loss)
{
  m_loss = loss;
}

void
YansWifiChannel::SetPropagationDelayModel (Ptr<PropagationDelayModel> delay)
{
  m_delay = delay;
}

void
YansWifiChannel::Send (Ptr<YansWifiPhy> sender, Ptr<const Packet> packet, double txPowerDbm,
                       WifiTxVector txVector, WifiPreamble preamble, struct mpduInfo aMpdu, Time duration) const
{
  Ptr<MobilityModel> senderMobility = sender->GetMobility ()->GetObject<MobilityModel> ();
  NS_ASSERT (senderMobility != 0);
  uint32_t j = 0;
  for (PhyList::const_iterator i = m_phyList.begin (); i != m_phyList.end (); i++, j++)
    {
      if (sender != (*i))
        {
          //For now don't account for inter channel interference
          if ((*i)->GetChannelNumber () != sender->GetChannelNumber ())
            {
              continue;
            }

          Ptr<MobilityModel> receiverMobility = (*i)->GetMobility ()->GetObject<MobilityModel> ();
          Time delay = m_delay->GetDelay (senderMobility, receiverMobility);
          double rxPowerDbm = m_loss->CalcRxPower (txPowerDbm, senderMobility, receiverMobility);
          NS_LOG_DEBUG ("propagation: txPower=" << txPowerDbm << "dbm, rxPower=" << rxPowerDbm << "dbm, " <<
                        "distance=" << senderMobility->GetDistanceFrom (receiverMobility) << "m, delay=" << delay);
          Ptr<Packet> copy = packet->Copy ();
          Ptr<Object> dstNetDevice = m_phyList[j]->GetDevice ();
          uint32_t dstNode;
          if (dstNetDevice == 0)
            {
              dstNode = 0xffffffff;
            }
          else
            {
              dstNode = dstNetDevice->GetObject<NetDevice> ()->GetNode ()->GetId ();
            }

          struct Parameters parameters;
          parameters.rxPowerDbm = rxPowerDbm;
          parameters.aMpdu = aMpdu;
          parameters.duration = duration;
          parameters.txVector = txVector;
          parameters.preamble = preamble;
          
          // <M>
          uint32_t packetUid = copy->GetUid ();
          bool isSymbolic = false;
          if (m_symPacketList != 0)
            {
              isSymbolic = m_symPacketList->IsSymPacket (packetUid);
            }
          //Time delay;
          uint32_t srcNode = sender->GetDevice ()->GetNode ()->GetId ();

          if (isSymbolic)
            {
              delay = TimeStep (m_symPacketList->GetSymPacketDelay (packetUid));
              NS_LOG_DEBUG ("(Symbolic) Packet UID: " << packetUid <<
                        ", Size: " << copy->GetSize () <<
                        ", Dst: " << dstNode <<
                        ", Src: " << srcNode <<
                        ", Delay:" << delay);
            }
          else
            {
              if (m_randomDelay)
                {
                  delay = TimeStep (rand() % m_maxDelay);
                }
              /*else
                {
                  delay = m_delay;
                }*/
              NS_LOG_DEBUG ("Packet UID: " << packetUid <<
                        ", Size: " << copy->GetSize () <<
                        ", Dst: " << dstNode <<
                        ", Src: " << srcNode <<
                        ", Delay: " << delay);
            }

          Simulator::ScheduleWithContext (dstNode,
                                          delay, &YansWifiChannel::Receive, this,
                                          j, copy, parameters);
        }
    }
}

void
YansWifiChannel::Receive (uint32_t i, Ptr<Packet> packet, struct Parameters parameters) const
{
  m_phyList[i]->StartReceivePreambleAndHeader (packet, parameters.rxPowerDbm, parameters.txVector, parameters.preamble, parameters.aMpdu, parameters.duration);
}

uint32_t
YansWifiChannel::GetNDevices (void) const
{
  return m_phyList.size ();
}

Ptr<NetDevice>
YansWifiChannel::GetDevice (uint32_t i) const
{
  return m_phyList[i]->GetDevice ()->GetObject<NetDevice> ();
}

void
YansWifiChannel::Add (Ptr<YansWifiPhy> phy)
{
  m_phyList.push_back (phy);
}

int64_t
YansWifiChannel::AssignStreams (int64_t stream)
{
  int64_t currentStream = stream;
  currentStream += m_loss->AssignStreams (stream);
  return (currentStream - stream);
}

} //namespace ns3
