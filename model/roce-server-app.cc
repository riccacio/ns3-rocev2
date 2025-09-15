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
        std::cout<<"[SERVER APP] Setup\n";
    }

    void RoceServerApp::SetNic(Ptr<ns3::RoceNic> nic) {
        m_nic = nic;
    }
    void RoceServerApp::StartApplication()
    {
        std::cout << "[SERVER APP] StartApplication" << std::endl;

        // Controlli difensivi
        NS_ASSERT_MSG(m_nic, "RoceServerApp: NIC non impostato (chiama SetNic prima di AddApplication).");
        NS_ASSERT_MSG(m_nic->GetNode() == GetNode(), "RoceServerApp: NIC e App sono su nodi diversi!");

        if (!m_connected)
        {
            // Registriamo una callback che riceve i pacchetti consegnati in-ordine dal NIC
            m_nic->SetReceiveCallback(MakeCallback(&RoceServerApp::OnNicReceive, this));
            m_connected = true;
        }
    }

    void RoceServerApp::StopApplication()
    {
        std::cout << "[SERVER APP] StopApplication" << std::endl;
        // Se il NIC sopravvive all’app, togliamo la callback
        if (m_nic && m_connected)
        {
            m_nic->SetReceiveCallback(MakeCallback(&RoceServerApp::OnNicReceive, this));
            m_connected = false;
        }
    }

    void RoceServerApp::OnNicReceive(Ptr<Packet> pkt)
    {
        m_packetsReceived++;
        uint32_t size = pkt->GetSize();
        std::cout << "[SERVER APP] OnNicReceive: packet size=" << size << " (tot=" << m_packetsReceived << ")\n";
        // Qui, se vuoi, puoi anche generare una risposta: crea un nuovo Packet
        // e chiedi al NIC di spedirlo (il NIC gestisce header/tag/ack/reorder).
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