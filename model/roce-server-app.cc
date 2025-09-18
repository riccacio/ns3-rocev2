#include "roce-server-app.h"
#include "roce-header-tag.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/inet-socket-address.h"


namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("RoceServerApp");

    RoceServerApp::RoceServerApp() : m_packetsReceived(0) {std::cout << "[SERVER APP] costruttore chiamato\n";}
    RoceServerApp::~RoceServerApp() {}

    void RoceServerApp::Setup(InetSocketAddress address) {
        m_peer = address.GetIpv4();
        m_port = address.GetPort();
    }

    void RoceServerApp::SetNic(Ptr<ns3::RoceNic> nic) {
        m_nic = nic;
    }

    void RoceServerApp::StartApplication() {
        if (!m_connected) {
            m_nic->SetReceiveCallback(MakeCallback(&RoceServerApp::OnNicReceive, this));
            m_connected = true;
        }
    }

    void RoceServerApp::StopApplication() {
        if (m_nic && m_connected) {
            m_nic->SetReceiveCallback(MakeCallback(&RoceServerApp::OnNicReceive, this));
            m_connected = false;
        }
    }

    void RoceServerApp::OnNicReceive(Ptr<Packet> pkt) {
        m_packetsReceived++;
        uint32_t size = pkt->GetSize();
        std::cout << "[SERVER APP] OnNicReceive: packet size=" << size << " (tot=" << m_packetsReceived << ")\n";
    }

    void RoceServerApp::HandlePacket(Ptr<Packet> pkt) {
        RoceHeaderTag tag;
        if (pkt->PeekPacketTag(tag)) {
            std::cout << "At time " << Simulator::Now().GetSeconds()
                      << "s, SERVER ha ricevuto: " << tag << std::endl;
            m_packetsReceived++;
        }
    }

    uint32_t RoceServerApp::GetPacketsReceived() const {
        return m_nic->GetReceivedPsn();
    }


} // namespace ns3