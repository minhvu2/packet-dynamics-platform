#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/constant-position-mobility-model.h"

#include <ctime>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace ns3;
void setPos (Ptr<Node> n, int x, int y, int z)
{
  Ptr<ConstantPositionMobilityModel> loc = CreateObject<ConstantPositionMobilityModel> ();
  n->AggregateObject (loc);
  Vector locVec2 (x, y, z);
  loc->SetPosition (locVec2);
}

int main (int argc, char *argv[])
{
  uint32_t nRtrs = 2;
  uint64_t m_delay1 = 500;
  uint64_t m_delay2 = 1000;
  uint32_t m_maxBytes = 500;
  uint32_t m_num = 1;
  uint64_t m_maxDelay= 16384000000;
  bool m_randomDelay = false;
  bool m_verbose = false;
  std::string m_delayFile = "";
  std::string m_symDelayFile = "";
  CommandLine cmd;
  cmd.AddValue ("nRtrs", "Number of routers. Default 2", nRtrs);
  cmd.AddValue ("delay1", "Delay of symbolic packet 1", m_delay1);
  cmd.AddValue ("delay2", "Delay of symbolic packet 2", m_delay2);
  cmd.AddValue ("maxBytes", "Max bytes to send", m_maxBytes);
  cmd.AddValue ("num", "Number of symbolic delays", m_num);
  cmd.AddValue ("maxDelay", "Maximum value of delay", m_maxDelay);
  cmd.AddValue ("randomDelay", "Use random delay", m_randomDelay);
  cmd.AddValue ("verbose", "Verbose mode", m_verbose);
  cmd.AddValue ("delayFile", "Path to delay file", m_delayFile);
  cmd.AddValue ("symDelayFile", "Path to sym delay file", m_symDelayFile);
  cmd.Parse (argc, argv);
  
  srand (time (NULL));
  
  if (m_verbose)
    {
      LogComponentEnable ("PointToPointChannel", LOG_LEVEL_DEBUG);
    }

  NodeContainer nodes, routers;
  nodes.Create (2);
  routers.Create (nRtrs);

  DceManagerHelper dceManager;
  dceManager.SetTaskManagerAttribute ("FiberManagerType",
                                      StringValue ("UcontextFiberManager"));

  dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                              "Library", StringValue ("liblinux.so"));
  LinuxStackHelper stack;
  stack.Install (nodes);
  stack.Install (routers);

  dceManager.Install (nodes);
  dceManager.Install (routers);

  PointToPointHelper pointToPoint;
  NetDeviceContainer devices1, devices2;
  Ipv4AddressHelper address1, address2;
  std::ostringstream cmd_oss;
  address1.SetBase ("10.1.0.0", "255.255.255.0");
  address2.SetBase ("10.2.0.0", "255.255.255.0");
  
  std::list<uint32_t> packetUidList;
  std::list<uint64_t> packetDelayList;

// <M>
  if (m_num==1)
    {
      //std::cout << "One symbolic packet, num=" << m_num << std::endl;
      uint32_t UidList [1] = {20};
      packetUidList.assign (UidList, UidList+1);
      uint64_t delayList [1] = {m_delay1};
      packetDelayList.assign (delayList, delayList+1);
    }
        
  if (m_num==2)
    {
      //std::cout << "Two symbolic packet, num=" << m_num << std::endl;
      uint32_t UidList [2] = {20, 21};
      packetUidList.assign (UidList, UidList+2);
      uint64_t delayList [2] = {m_delay1, m_delay2};
      packetDelayList.assign (delayList, delayList+2);
    }
  
  // All packets get delay values from input file
  if (m_num==123456789)
    {
      std::ifstream delayIn (m_delayFile.c_str ());
      std::ifstream symDelayIn (m_symDelayFile.c_str ());
      
      if (!delayIn.is_open() && !symDelayIn.is_open())
        {
          std::cout << "Error reading input files" << std::endl;
        }
      
      uint32_t packetUid;
      uint64_t delay;
      std::string line = "";
      
      while (getline (delayIn, line))
        {
          std::istringstream iss (line);
          if (!(iss >> packetUid >> delay)) 
            {
              break;
            }
          if (delay == 0)
            {
              symDelayIn >> delay;
            }
          packetUidList.push_back (packetUid);
          packetDelayList.push_back (delay);
        }
      delayIn.close ();
      symDelayIn.close ();  
    }
// <M>

  for (uint32_t i = 0; i < nRtrs; i++)
    {
      // Left link
      // <M>
      Ptr<SymbolicPacketList> symPacketList = CreateObject<SymbolicPacketList> ();
      symPacketList->SetList (packetUidList, packetDelayList); 
      pointToPoint.SetChannelAttribute ("SymbolicPacketList", PointerValue (symPacketList));
      pointToPoint.SetChannelAttribute ("RandomDelay", BooleanValue (m_randomDelay));
      pointToPoint.SetChannelAttribute ("MaxDelay", UintegerValue (m_maxDelay));
      // <M>
      pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
      pointToPoint.SetChannelAttribute ("Delay", StringValue ("10ms"));
      devices1 = pointToPoint.Install (nodes.Get (0), routers.Get (i));
      // Assign ip addresses
      Ipv4InterfaceContainer if1 = address1.Assign (devices1);
      address1.NewNetwork ();
      // setup ip routes
      cmd_oss.str ("");
      cmd_oss << "rule add from " << if1.GetAddress (0, 0) << " table " << (i+1);
      LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add 10.1." << i << ".0/24 dev sim" << i << " scope link table " << (i+1);
      LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add default via " << if1.GetAddress (1, 0) << " dev sim" << i << " table " << (i+1);
      LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add 10.1.0.0/16 via " << if1.GetAddress (1, 0) << " dev sim0";
      LinuxStackHelper::RunIp (routers.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());

      // Right link
      pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
      pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ns"));
      devices2 = pointToPoint.Install (nodes.Get (1), routers.Get (i));
      // Assign ip addresses
      Ipv4InterfaceContainer if2 = address2.Assign (devices2);
      address2.NewNetwork ();
      // setup ip routes
      cmd_oss.str ("");
      cmd_oss << "rule add from " << if2.GetAddress (0, 0) << " table " << (i+1);
      LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add 10.2." << i << ".0/24 dev sim" << i << " scope link table " << (i+1);
      LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add default via " << if2.GetAddress (1, 0) << " dev sim" << i << " table " << (i+1);
      LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add 10.2.0.0/16 via " << if2.GetAddress (1, 0) << " dev sim1";
      LinuxStackHelper::RunIp (routers.Get (i), Seconds (0.2), cmd_oss.str ().c_str ());

      setPos (routers.Get (i), 50, i * 20, 0);
    }

  // default route
  LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.1), "route add default via 10.1.0.2 dev sim0");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (0.1), "route add default via 10.2.0.2 dev sim0");
  LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.1), "rule show");

  // Schedule Up/Down (XXX: didn't work...)
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (1.0), "link set dev sim0 multipath off");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (15.0), "link set dev sim0 multipath on");
  LinuxStackHelper::RunIp (nodes.Get (1), Seconds (30.0), "link set dev sim0 multipath off");


  // debug
  stack.SysctlSet (nodes, ".net.mptcp.mptcp_debug", "1");

  // Launch iperf client on node 0
  ApplicationContainer apps;
/*  OnOffHelper onoff = OnOffHelper ("ns3::LinuxTcpSocketFactory",
                                   InetSocketAddress ("10.2.0.1", 9));
  onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute ("PacketSize", StringValue ("1024"));
  onoff.SetAttribute ("DataRate", StringValue ("500Kbps"));
  apps = onoff.Install (nodes.Get (0));*/
  BulkSendHelper bulk = BulkSendHelper ("ns3::LinuxTcpSocketFactory",
                                       InetSocketAddress ("10.2.0.1", 9));
  bulk.SetAttribute ("MaxBytes", UintegerValue (m_maxBytes));
  apps = bulk.Install (nodes.Get (0));
  apps.Start (Seconds (5.0));
  apps.Stop (Seconds (15.0));

  // server on node 1
  PacketSinkHelper sink = PacketSinkHelper ("ns3::LinuxTcpSocketFactory",
                                            InetSocketAddress (Ipv4Address::GetAny (), 9));
  apps = sink.Install (nodes.Get (1));
  apps.Start (Seconds (3.9999));

  pointToPoint.EnablePcapAll ("mptcp-dce-cradle", false);

  apps.Start (Seconds (4));

  setPos (nodes.Get (0), 0, 20 * (nRtrs - 1) / 2, 0);
  setPos (nodes.Get (1), 100, 20 * (nRtrs - 1) / 2, 0);

  Simulator::Stop (Seconds (15.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
