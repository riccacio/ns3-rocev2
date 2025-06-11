#include "roce-server-app.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RoceServerApp");

RoceServerApp::RoceServerApp() : m_packetsReceived(0) {}
RoceServerApp::~RoceServerApp() {}

void RoceServerApp::Setup(Address address, uint16_t port) {
  m_peer = address;
  m_port = port;
}

void RoceServerApp::StartApplication() {
  auto addr = InetSocketAddress::ConvertFrom(m_peer);
  std::cout << "SERVER: " << addr.GetIpv4() << " in ascolto sulla porta: " << m_port << std::endl;
  m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
  m_socket->Bind(local);
  m_socket->SetRecvCallback(MakeCallback(&RoceServerApp::HandleRead, this));
}

void RoceServerApp::StopApplication() {
  if (m_socket) {
    m_socket->Close();
    m_socket = nullptr;
  }
}

void RoceServerApp::HandleRead(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom(from))) {
    RoceHeaderTag tag;
    if (packet->PeekPacketTag(tag)) {
      std::cout << "At time " << Simulator::Now().GetSeconds() << "s, SERVER ha ricevuto: " << tag << std::endl;
      m_packetsReceived++;

      Ptr<Packet> ackPacket = Create<Packet>(0);
      RoceHeaderTag ackTag(0x5678, 0xFF, tag.GetPsn(), tag.GetPsn(),Ipv4Address::ConvertFrom(m_socket->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal()) );
      ackPacket->AddPacketTag(ackTag);

      InetSocketAddress source = InetSocketAddress::ConvertFrom(from);
      InetSocketAddress replyAddr(source.GetIpv4(), 4791); // forza risposta alla porta client
      Ipv4Address clientIp = tag.GetClientIp();
      socket->SendTo(ackPacket, 0, InetSocketAddress(clientIp, m_port));
      std::cout << "At time " << Simulator::Now().GetSeconds() << "s, SERVER ha inviato ACK per PSN=" << tag.GetPsn() << std::endl;
    }
  }
}

uint32_t RoceServerApp::GetPacketsReceived() const {
  return m_packetsReceived;
}

} // namespace ns3