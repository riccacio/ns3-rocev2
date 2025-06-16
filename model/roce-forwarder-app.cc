#include "roce-forwarder-app.h"
#include "roce-header-tag.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/udp-socket-factory.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("RoceForwarderApp");

    RoceForwarderApp::RoceForwarderApp() {}
    RoceForwarderApp::~RoceForwarderApp() {}

    void RoceForwarderApp::Setup(Ipv4Address dest, uint16_t port, std::string id, Ipv4Address sender) {
        m_destAddr = dest;
        m_port = port;
        m_id = id;
        m_senderAddr = sender;
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
                Address nextHop;
                if (tag.GetOpcode() == 0xFF) {
                    // ACK → verso CLIENT
                    nextHop = InetSocketAddress(m_senderAddr, m_port);
                    std::cout << "[FORWARDER ACK] " << m_id << " inoltra ACK PSN=" << tag.GetPsn()
                              << " verso CLIENT " << m_senderAddr << std::endl;
                } else {
                    // Data → verso SERVER
                    nextHop = InetSocketAddress(m_destAddr, m_port);
                    std::cout << "[FORWARDER] " << m_id << " inoltra PSN=" << tag.GetPsn()
                              << " verso SERVER " << m_destAddr << std::endl;
                }

                if (!m_forwardSocket) {
                    m_forwardSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
                }

                m_forwardSocket->SendTo(packet, 0, nextHop);
            }
        }
    }

} // namespace ns3