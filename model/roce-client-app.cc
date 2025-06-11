#include "roce-client-app.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RoceClientApp");


RoceClientApp::RoceClientApp() : m_psn(100), m_packetsReceived(0) {}
RoceClientApp::~RoceClientApp() {}

void RoceClientApp::Setup(Address peer1, Address peer2, uint32_t packetSize, uint32_t numPackets, Time interval) {
  m_peerPath1 = peer1;
  m_peerPath2 = peer2;
  m_packetSize = packetSize;
  m_nPackets = numPackets;
  m_interval = interval;
  m_psn = 100;
}

void RoceClientApp::StartApplication() {
  auto addr = InetSocketAddress::ConvertFrom(m_peerPath1);
  auto serverAddr= InetSocketAddress::ConvertFrom(m_peerPath2);
  std::cout << "CLIENT: " << addr.GetIpv4() << ":" << addr.GetPort() 
  << " si collega al SERVER " << serverAddr.GetIpv4() << ":" << serverAddr.GetPort() << std::endl;
  m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());

  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 4791); // 4791 porta UDP che usa RoCEv2
  m_socket->Bind(local);

  m_socket->Connect(m_peerPath1);
  m_socket->SetRecvCallback(MakeCallback(&RoceClientApp::HandleRead, this));

  m_sent = 0;
  SendPacket();
}

void RoceClientApp::StopApplication() {
  if (m_socket) {
    m_socket->Close();
  }
}

void RoceClientApp::SendPacket() {
  Ptr<Packet> packet = Create<Packet>(m_packetSize);
  RoceHeaderTag tag(0x1234, 0x1A, m_psn, 0xDEADBEEF, Ipv4Address::ConvertFrom(m_socket->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal()));
  packet->AddPacketTag(tag);

  // Alterna il path (pari = path1, dispari = path2)
  Address destination = (m_psn % 2 == 0) ? m_peerPath1 : m_peerPath2;

  m_socket->SendTo(packet, 0, destination);

  std::cout << "\nAt time " << Simulator::Now().GetSeconds() << "s, CLIENT ha trasmesso (via path "
            << ((m_psn % 2 == 0) ? "1" : "2") << "): " << tag << std::endl;

  ++m_sent;
  ++m_psn;

  if (m_sent < m_nPackets) {
    Simulator::Schedule(m_interval, &RoceClientApp::SendPacket, this);
  }
}

void RoceClientApp::HandleRead(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  while ((packet = socket->Recv())) {
    m_packetsReceived++;
    RoceHeaderTag tag;
    if (packet->PeekPacketTag(tag)) {
      std::cout << "At time " << Simulator::Now().GetSeconds() << "s, CLIENT ha ricevuto: OPCODE=" << (uint32_t)tag.GetOpcode() << " PSN=" << tag.GetPsn() << std::endl;
      if (tag.GetOpcode() == 0xFF) {
        std::cout <<"At time " << Simulator::Now().GetSeconds() << "s, CLIENT ha ricevuto ACK per PSN=" << tag.GetPsn() << " e IMM=" << tag.GetImm() << std::endl;
      }
    }
  }
}

uint32_t RoceClientApp::GetPacketsReceived() const {
  return m_packetsReceived;
}
} // namespace ns3