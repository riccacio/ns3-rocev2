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

    RoceServerApp::RoceServerApp() : m_packetsReceived(0) {}
    RoceServerApp::~RoceServerApp() {}

    void RoceServerApp::Setup(Address address, uint16_t port) {
        m_peer = address;
        m_port = port;
    }

    void RoceServerApp::SetNic(Ptr<ns3::RoceNic> nic) {
        m_nic = nic;
    }

    void RoceServerApp::StartApplication() {
        auto addr = InetSocketAddress::ConvertFrom(m_peer);
        std::cout << "SERVER: " << addr.GetIpv4() << " in ascolto sulla porta: " << m_port << std::endl;

        m_nic = CreateObject<RoceNic>();
        m_nic->Setup(m_port);
        m_nic->SetReceiveCallback(MakeCallback(&RoceServerApp::HandlePacket, this));
        GetNode()->AddApplication(m_nic);
        m_nic->SetStartTime(Seconds(0.0));
        m_nic->SetStopTime(Seconds(1.5)); //VERIFICA SE È DA ADATTARE...
    }

    void RoceServerApp::StopApplication() {
        if (m_nic) {
            m_nic = nullptr;
        }
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
        return m_packetsReceived;
    }


} // namespace ns3