#include "roce-forwarder-app.h"
#include "ns3/log.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RoceForwarderApp");

RoceForwarderApp::RoceForwarderApp() {}
RoceForwarderApp::~RoceForwarderApp() {}

void RoceForwarderApp::Setup(Ipv4Address forwardAddress, uint16_t port, std::string type) {
  m_forwardAddress = forwardAddress;
  m_port = port;
  m_type = type;
}

void RoceForwarderApp::StartApplication() {
  m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
  m_socket->Bind(local);
  m_socket->SetRecvCallback(MakeCallback(&RoceForwarderApp::HandleRead, this));
}

void RoceForwarderApp::StopApplication() {
  if (m_socket) {
    m_socket->Close();
    m_socket = nullptr;
  }
}

void RoceForwarderApp::HandleRead(Ptr<Socket> socket) {
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom(from))) {
    RoceHeaderTag tag;
    if (packet->PeekPacketTag(tag)) {
        std::cout << "[FORWARDER]" << m_type << " inoltro PSN=" << tag.GetPsn()
                  << " verso " << m_forwardAddress << std::endl;
      
    }
    socket->SendTo(packet, 0, InetSocketAddress(m_forwardAddress, m_port));
  }
}

} // namespace ns3