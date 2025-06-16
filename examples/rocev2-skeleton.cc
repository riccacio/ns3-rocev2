#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

#include "../model/roce-client-app.h"
#include "../model/roce-server-app.h"
#include "../model/roce-header-tag.h"
#include "../model/roce-forwarder-app.h"

#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("rocev2-skeleton");

int main(int argc, char *argv[]) {
  LogComponentEnable("rocev2-skeleton", LOG_LEVEL_INFO);

  NodeContainer clientNode, switch1Node, switch2Node, serverNode;
  clientNode.Create(1);
  switch1Node.Create(1);
  switch2Node.Create(1);
  serverNode.Create(1);

  InternetStackHelper internet;
  internet.InstallAll();

  // Path 1
  PointToPointHelper p1;
  p1.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
  p1.SetChannelAttribute("Delay", StringValue("1ms"));
  NetDeviceContainer devA = p1.Install(clientNode.Get(0), switch1Node.Get(0));

  PointToPointHelper p2;
  p2.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
  p2.SetChannelAttribute("Delay", StringValue("1ms"));
  NetDeviceContainer devB = p2.Install(switch1Node.Get(0), serverNode.Get(0));

  // Path 2
  PointToPointHelper p3;
  p3.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
  p3.SetChannelAttribute("Delay", StringValue("2ms"));
  NetDeviceContainer devC = p3.Install(clientNode.Get(0), switch2Node.Get(0));

  PointToPointHelper p4;
  p4.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
  p4.SetChannelAttribute("Delay", StringValue("2ms"));
  NetDeviceContainer devD = p4.Install(switch2Node.Get(0), serverNode.Get(0));

  Ipv4AddressHelper ipv4;

  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer if1 = ipv4.Assign(devA);

  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer if2 = ipv4.Assign(devB);

  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer if3 = ipv4.Assign(devC);

  ipv4.SetBase("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer if4 = ipv4.Assign(devD);

    // Enable routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();


  Ptr<RoceForwarderApp> forwarder1 = CreateObject<RoceForwarderApp>();
  forwarder1->Setup(if2.GetAddress(1), 4791, "Switch 1",  if1.GetAddress(0));  // switch1 → server via link B
  switch1Node.Get(0)->AddApplication(forwarder1);
  forwarder1->SetStartTime(Seconds(0));
  forwarder1->SetStopTime(Seconds(1.0));

  Ptr<RoceForwarderApp> forwarder2 = CreateObject<RoceForwarderApp>();
  forwarder2->Setup(if4.GetAddress(1), 4791, "Swicht 2",  if3.GetAddress(0));  // switch2 → server via link D
  switch2Node.Get(0)->AddApplication(forwarder2);
  forwarder2->SetStartTime(Seconds(0));
  forwarder2->SetStopTime(Seconds(1.0));

  // Install server on serverNode
  Ptr<RoceServerApp> serverApp = CreateObject<RoceServerApp>();
  serverApp->Setup(Address(InetSocketAddress(if3.GetAddress(1))),4791);
  serverNode.Get(0)->AddApplication(serverApp);
  serverApp->SetStartTime(Seconds(0));
  serverApp->SetStopTime(Seconds(1.5));


  // Install client on clientNode
  Ptr<RoceClientApp> clientApp = CreateObject<RoceClientApp>();

  // 🟡 CLIENT → usa IP del secondo path (più lento) per creare misorder
  // Server ascolta su tutte le interfacce (non cambia)
  Address path1 = InetSocketAddress(if1.GetAddress(1), 4791); // verso switch1
  Address path2 = InetSocketAddress(if3.GetAddress(1), 4791); // verso switch2

  clientApp->Setup(path1, path2, 1024, 10, MilliSeconds(1.0));
  clientNode.Get(0)->AddApplication(clientApp);
  clientApp->SetStartTime(Seconds(0.5));
  clientApp->SetStopTime(Seconds(1.0));
  /*
  std::ofstream logFile("rocev2.log");
  std::streambuf* coutBuf = std::cout.rdbuf();  // salva il buffer originale
  std::cout.rdbuf(logFile.rdbuf());             // reindirizza cout al file
*/
  Simulator::Run();
  Simulator::Destroy();

  std::cout << "\n--- STATISTICHE ---" << std::endl;
  std::cout << "Totale pacchetti ricevuti dal server: " << serverApp->GetPacketsReceived() << std::endl;
  std::cout << "Totale pacchetti ricevuti dal client: " << clientApp->GetPacketsReceived() << std::endl;
/*
  std::cout.rdbuf(coutBuf);  // ripristina il cout standard
  std::cout << "\nLog scritto in rocev2.log" << std::endl;
*/
  return 0;
}