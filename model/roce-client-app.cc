#include "roce-client-app.h"
#include "roce-header-tag.h"

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/ipv4.h"
#include "ns3/inet-socket-address.h"


namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("RoceClientApp");

    RoceClientApp::RoceClientApp(InetSocketAddress peer1, InetSocketAddress peer2)
            : m_socket(nullptr), m_peerPath1(peer1), m_peerPath2(peer2), m_packetSize(0), m_nPackets(0), m_interval(Seconds(0)),
              m_sent(0), m_psn(100), m_packetsReceived(0) {}

    RoceClientApp::~RoceClientApp() {}

    void RoceClientApp::Setup(InetSocketAddress path1, InetSocketAddress path2, uint32_t packetSize, uint32_t numPackets, Time interval, Ptr<RoceNic> nic) {
        m_peerPath1 = path1;
        m_peerPath2 = path2;
        m_packetSize = packetSize;
        m_nPackets = numPackets;
        m_interval = interval;
        m_nic = nic;

        m_psn = 100;
    }

    void RoceClientApp::StartApplication() {
        std::cout << "CLIENT start: invio di " << m_nPackets << " pacchetti." << std::endl;

        m_sent = 0;
        Simulator::ScheduleNow(&RoceClientApp::SendPacket, this);
    }

    void RoceClientApp::StopApplication() {
        if (m_socket) {
            m_socket->Close();
        }
    }

    void RoceClientApp::SendPacket() {
        // crea il payload
        Ptr<Packet> packet = Create<Packet>(m_packetSize);

        // alterna i path: pari -> path1 (veloce), dispari -> path2 (lento)
        const bool usePath1 = (m_psn % 2 == 0);
        const Address& dst = usePath1 ? m_peerPath1 : m_peerPath2;

        // imposta al NIC l’IP di destinazione (quello del SERVER sulla sottorete corretta)
        Ipv4Address peerIp = InetSocketAddress::ConvertFrom(dst).GetIpv4();
        m_nic->SetPeer(peerIp);

        // invia tramite NIC (il NIC aggiunge il RoCE tag)
        m_nic->Send(packet);

        // log
        std::cout << "CLIENT invia PSN=" << m_psn
                  << " via path " << (usePath1 ? 1 : 2)
                  << " (peer=" << peerIp << ")\n";

        // bookkeeping e schedule del prossimo invio
        ++m_psn;
        ++m_sent;
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
                std::cout << "Al tempo " << Simulator::Now().GetSeconds()
                          << "s, CLIENT ha ricevuto: OPCODE=" << (uint32_t)tag.GetOpcode()
                          << " PSN=" << tag.GetPsn() << std::endl;
                if (tag.GetOpcode() == 0xFF) {
                    std::cout << "Al tempo " << Simulator::Now().GetSeconds()
                              << "s, CLIENT ha ricevuto ACK per PSN=" << tag.GetPsn()
                              << " e IMM=" << tag.GetImm() << std::endl;
                }
            }
        }
    }

    uint32_t RoceClientApp::GetPacketsReceived() const {
        return m_nic->GetReceivedPsn();
    }

    uint32_t RoceClientApp::GetPacketsSent() const{
        return m_sent;
    }


    void RoceClientApp::SetNic(Ptr<RoceNic> nic) {
        m_nic = nic;
    }

} // namespace ns3