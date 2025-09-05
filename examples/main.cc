#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"


#include "../model/roce-client-app.h"
#include "../model/roce-server-app.h"
#include "../model/roce-header-tag.h"
#include "../model/roce-forwarder-app.h"

#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("main");

int main(int argc, char *argv[]) {

    NodeContainer clientNode, switch1Node, switch2Node, serverNode;
    clientNode.Create(1);
    switch1Node.Create(1);
    switch2Node.Create(1);
    serverNode.Create(1);


    InternetStackHelper stack;
    stack.InstallAll();

    // Path 1 (veloce)
    PointToPointHelper p1;
    p1.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
    p1.SetChannelAttribute("Delay", StringValue("1ms"));
    NetDeviceContainer devA = p1.Install(clientNode.Get(0), switch1Node.Get(0));

    PointToPointHelper p2;
    p2.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
    p2.SetChannelAttribute("Delay", StringValue("1ms"));
    NetDeviceContainer devB = p2.Install(switch1Node.Get(0), serverNode.Get(0));

    // Path 2 (lento)
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

    // Routing IP automatico
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Server NIC
    Ptr<RoceNic> serverNic = CreateObject<RoceNic>();
    serverNode.Get(0)->AddApplication(serverNic);
    serverNic->Setup(4791);
    //serverNic->SetPeer(if1.GetAddress(1)); // IP del Client
    serverNic->SetStartTime(Seconds(0));
    serverNic->SetStopTime(Seconds(1.5));

    // Server App
    Ptr<RoceServerApp> serverApp = CreateObject<RoceServerApp>();
    serverApp->SetNic(serverNic);
    //serverApp->Setup(InetSocketAddress(if4.GetAddress(1),4791));
    serverNode.Get(0)->AddApplication(serverApp);
    serverApp->SetStartTime(Seconds(0));
    serverApp->SetStopTime(Seconds(1.5));

    // Client NIC
    Ptr<RoceNic> clientNic = CreateObject<RoceNic>();
    clientNode.Get(0)->AddApplication(clientNic);
    clientNic->Setup();
    clientNic->SetPeer(if2.GetAddress(1)); // primo hop verso server
    clientNic->SetStartTime(Seconds(0.0));
    clientNic->SetStopTime(Seconds(2.0));

    // Client App
    Ptr<RoceClientApp> clientApp = CreateObject<RoceClientApp>(InetSocketAddress(if2.GetAddress(1), 4791), // 10.1.2.2 (server lato path1)
                                                               InetSocketAddress(if4.GetAddress(1), 4791)); // 10.1.4.2 (server lato path2)
    clientApp->Setup(    InetSocketAddress(if2.GetAddress(1), 4791), //path 1 verso switch 1
                         InetSocketAddress(if4.GetAddress(1), 4791), //path 2 verso switch 2
                         1024,
                         10,
                         MilliSeconds(1.0),
                         clientNic
    );
    clientNode.Get(0)->AddApplication(clientApp);
    clientApp->SetStartTime(Seconds(0.5));
    clientApp->SetStopTime(Seconds(1.5));
/*
    // NetAnim
    AnimationInterface::SetConstantPosition(clientNode.Get(0), 10, 30);
    AnimationInterface::SetConstantPosition(switch1Node.Get(0), 30, 40);
    AnimationInterface::SetConstantPosition(switch2Node.Get(0), 30, 20);
    AnimationInterface::SetConstantPosition(serverNode.Get(0), 50, 30);
    AnimationInterface anim("rocev2.xml");


    */


    //std::cout << "Client NIC peer address: " << clientNic->GetPeerAddress() << std::endl;
    LogComponentEnable("RoceNic", LOG_LEVEL_INFO);
    Simulator::Run();
    Simulator::Destroy();

    std::cout << "\n--- STATISTICHE ---" << std::endl;
    std::cout << "Totale pacchetti inviati dal client: " << clientApp->GetPacketsSent() << std::endl;
    std::cout << "Totale pacchetti ricevuti dal server: " << serverApp->GetPacketsReceived() << std::endl;
    std::cout << "Totale pacchetti ricevuti dal client: " << clientApp->GetPacketsReceived() << std::endl;

    return 0;
}