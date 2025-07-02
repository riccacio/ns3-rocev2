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
                std::cout << "At time " << Simulator::Now().GetSeconds()
                          << "s, SERVER ha ricevuto: " << tag << std::endl;
                m_packetsReceived++;

                // Crea ACK
                Ptr<Packet> ackPacket = Create<Packet>(0);
                RoceHeaderTag ackTag(0x5678, 0xFF, tag.GetPsn(), tag.GetPsn(), tag.GetClientIp());
                ackPacket->AddPacketTag(ackTag);

                // Determina verso quale forwarder inviare in base al clientIp
                Ipv4Address clientIp = tag.GetClientIp();
                Ipv4Address forwarderIp;

                if (clientIp.CombineMask("255.255.255.0") == Ipv4Address("10.1.1.0")) {
                    forwarderIp = Ipv4Address("10.1.2.1"); // Switch 1
                } else if (clientIp.CombineMask("255.255.255.0") == Ipv4Address("10.1.3.0")) {
                    forwarderIp = Ipv4Address("10.1.4.1"); // Switch 2
                } else {
                    std::cerr << "ACK: IP client sconosciuto: " << clientIp << std::endl;
                    return;
                }

                //socket->SendTo(ackPacket, 0, InetSocketAddress(forwarderIp, m_port));

                Ptr<Packet> packet = Create<Packet>();
                m_nic->Send(packet);

                std::cout << "At time " << Simulator::Now().GetSeconds()
                          << "s, SERVER ha inviato ACK per PSN=" << tag.GetPsn()
                          << " verso " << forwarderIp << std::endl;
            }
        }
    }

    uint32_t RoceServerApp::GetPacketsReceived() const {
        return m_packetsReceived;
    }

    void RoceServerApp::SetNic(Ptr<RoceNic> nic){
        m_nic = nic;
    }

} // namespace ns3