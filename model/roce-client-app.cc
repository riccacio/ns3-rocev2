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

    RoceClientApp::RoceClientApp()
            : m_socket(nullptr), m_packetSize(0), m_nPackets(0), m_interval(Seconds(0)),
              m_sent(0), m_psn(100), m_packetsReceived(0) {}

    RoceClientApp::~RoceClientApp() {}

    void RoceClientApp::Setup(Address path1, Address path2, uint32_t packetSize, uint32_t numPackets, Time interval) {
        m_peerPath1 = path1;
        m_peerPath2 = path2;
        m_packetSize = packetSize;
        m_nPackets = numPackets;
        m_interval = interval;
    }

    void RoceClientApp::StartApplication() {
        m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());

        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 4791);
        m_socket->Bind(local);
        m_socket->SetRecvCallback(MakeCallback(&RoceClientApp::HandleRead, this));

        SendPacket();
    }

    void RoceClientApp::StopApplication() {
        if (m_socket) {
            m_socket->Close();
        }
    }

    void RoceClientApp::SendPacket() {
        Ptr<Packet> packet = Create<Packet>(m_packetSize);

        // Scegli path e IP client in base a PSN
        Address destination;
        Ipv4Address clientIp;
        Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();

        if (m_psn % 2 == 0) {
            destination = m_peerPath1;
            clientIp = ipv4->GetAddress(1, 0).GetLocal();  // interfaccia su path1
            std::cout << "\nAt time " << Simulator::Now().GetSeconds()
                      << "s, CLIENT ha trasmesso (via path 1 IP Switch: " << InetSocketAddress::ConvertFrom(destination).GetIpv4()  << ")" << std::endl;
        } else {
            destination = m_peerPath2;
            clientIp = ipv4->GetAddress(2, 0).GetLocal();  // interfaccia su path2
            std::cout << "\nAt time " << Simulator::Now().GetSeconds()
                      << "s, CLIENT ha trasmesso (via path 2 IP Switch: " << InetSocketAddress::ConvertFrom(destination).GetIpv4() << ")" << std::endl;
        }

        RoceHeaderTag tag(0x1234, 0x1A, m_psn, 0xDEADBEEF, clientIp);
        packet->AddPacketTag(tag);

        m_socket->SendTo(packet, 0, destination);

        std::cout << "Packet : " << tag << std::endl;

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
                std::cout << "At time " << Simulator::Now().GetSeconds()
                          << "s, CLIENT ha ricevuto: OPCODE=" << (uint32_t)tag.GetOpcode()
                          << " PSN=" << tag.GetPsn() << std::endl;
                if (tag.GetOpcode() == 0xFF) {
                    std::cout << "At time " << Simulator::Now().GetSeconds()
                              << "s, CLIENT ha ricevuto ACK per PSN=" << tag.GetPsn()
                              << " e IMM=" << tag.GetImm() << std::endl;
                }
            }
        }
    }

    uint32_t RoceClientApp::GetPacketsReceived() const {
        return m_packetsReceived;
    }

} // namespace ns3