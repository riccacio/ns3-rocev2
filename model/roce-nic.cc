#include "roce-nic.h"
#include "roce-header-tag.h"

#include "ns3/log.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("RoceNic");

    TypeId RoceNic::GetTypeId() {
        static TypeId tid = TypeId("ns3::RoceNic")
                .SetParent<Application>()
                .SetGroupName("Applications")
                .AddConstructor<RoceNic>();
        return tid;
    }

    RoceNic::RoceNic()
            : m_port(4791), m_psn(100), m_expectedPsn(100), m_retransmitTimeout(Seconds(0.01)) {}

    RoceNic::~RoceNic() {}

    void RoceNic::Setup(uint16_t port) {
        m_port = port;
    }

    void RoceNic::SetPeer(Ipv4Address ip) {
        m_peerAddr = ip;
    }

    void RoceNic::SetReceiveCallback(Callback<void, Ptr<Packet>> cb) {
        m_receiveCallback = cb;
    }

    Ipv4Address RoceNic::GetPeerAddress(){
        return m_peerAddr;
    }

    void RoceNic::StartApplication() {
        m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
        m_socket->Bind(local);
        m_socket->SetRecvCallback(MakeCallback(&RoceNic::HandleRead, this));

        m_sendSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());

        std::cout << "NIC started on " << GetLocalAddress() << std::endl;
    }

    void RoceNic::StopApplication() {
        if (m_socket) m_socket->Close();
        if (m_sendSocket) m_sendSocket->Close();
    }

    void RoceNic::Send(Ptr<Packet> packet) {
        if (m_peerAddr == Ipv4Address::GetAny()) {
            std::cerr << "NIC ERROR: m_peerAddr is not set! Cannot send.";
            return;
        }
        RoceHeaderTag tag(0x1234, 0x1A, m_psn, 0xDEADBEEF, GetLocalAddress());
        packet->AddPacketTag(tag);
        std::cout << "Sending to peer: " << m_peerAddr << std::endl;
        m_sendQueue.push(packet);
        Simulator::ScheduleNow(&RoceNic::ProcessSendQueue, this);
        m_psn++;
    }



    void RoceNic::ProcessSendQueue() {

        if (!m_sendQueue.empty()) {
            Ptr<Packet> pkt = m_sendQueue.front();
            m_sendQueue.pop();

            if (m_peerAddr == Ipv4Address::GetAny()) {
                std::cerr << "NIC ERROR: m_peerAddr not set, dropping packet.";
                return;
            }
            Address dest = InetSocketAddress(m_peerAddr, m_port);
            std::cout << "Send to address: " << dest << std::endl;
            m_sendSocket->SendTo(pkt, 0, dest);
            RoceHeaderTag tag;
            pkt->PeekPacketTag(tag);
            m_sentPsn.insert(tag.GetPsn());

            NS_LOG_INFO("NIC sent PSN=" << tag.GetPsn());

            Simulator::Schedule(m_retransmitTimeout, &RoceNic::CheckAckTimeout, this, tag.GetPsn());
        }
    }

    void RoceNic::HandleRead(Ptr<Socket> socket) {
        Address from;
        Ptr<Packet> packet = socket->RecvFrom(from);
        RoceHeaderTag tag;
        if (packet->PeekPacketTag(tag)) {
            NS_LOG_INFO("NIC received packet with PSN=" << tag.GetPsn());
            Ipv4Address src = InetSocketAddress::ConvertFrom(from).GetIpv4();
            SendAck(tag.GetPsn(), src);
        }
    }
    void RoceNic::SendAck(uint32_t psn, ns3::Address to) {
        Ptr<Packet> ack = Create<Packet>(0);
        RoceHeaderTag ackTag(0x5678, 0xFF, psn, psn, GetLocalAddress());
        ack->AddPacketTag(ackTag);
        m_sendSocket->SendTo(ack, 0, to);
        NS_LOG_INFO("NIC sent ACK for PSN=" << psn);
    }

    void RoceNic::CheckAckTimeout(uint32_t psn) {
        if (m_sentPsn.count(psn)) {
            NS_LOG_WARN("NIC timeout for PSN=" << psn << ", retransmitting NOT YET IMPLEMENTED");
            // TODO: Retransmit packet with this PSN (if needed)
        }
    }

    Ipv4Address RoceNic::GetLocalAddress() const {
        Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();
        return ipv4->GetAddress(1, 0).GetLocal(); // Usa la prima interfaccia utile
    }

} // namespace ns3