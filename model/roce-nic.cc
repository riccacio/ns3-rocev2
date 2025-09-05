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

    /*void RoceNic::StartApplication() {
        m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
        m_socket->Bind(local);
        m_socket->SetRecvCallback(MakeCallback(&RoceNic::HandleRead, this));

        m_sendSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());

        std::cout << "NIC started on " << GetLocalAddress() << std::endl;
    }*/


    void RoceNic::StartApplication() {
        std::cout << "[NIC] Inizio StartApplication" << std::endl;

        m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        std::cout << "[NIC] Creato m_socket" << std::endl;

        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
        m_socket->Bind(local);
        std::cout << "[NIC] Bind fatto su porta " << m_port << std::endl;

        m_socket->SetRecvCallback(MakeCallback(&RoceNic::HandleRead, this));
        std::cout << "[NIC] Callback di ricezione settata" << std::endl;

        m_sendSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        std::cout << "[NIC] Creato m_sendSocket" << std::endl;

        Ipv4Address localIp = GetLocalAddress();
        std::cout << "[NIC] StartApplication terminato, IP locale: " << localIp << std::endl;
    }

    void RoceNic::StopApplication() {
        if (m_socket) m_socket->Close();
        if (m_sendSocket) m_sendSocket->Close();
    }
    void RoceNic::Send(Ptr<Packet> packet) {
        RoceHeaderTag tag(0x1234, 0x1A, m_psn, 0xDEADBEEF, GetLocalAddress());
        packet->AddPacketTag(tag);

        m_sendQueue.push(packet);

        // DEBUG
        std::cout << "[NIC] Enqueue PSN=" << m_psn
                  << " (queue=" << m_sendQueue.size() << ")\n";

        // innesca la coda
        Simulator::ScheduleNow(&RoceNic::ProcessSendQueue, this);
        m_psn++;
    }

    void RoceNic::ProcessSendQueue() {
        if (!m_sendBound) {
            // lega il socket di invio all’IP della NIC (porta effimera)
            InetSocketAddress local(GetLocalAddress(), 0);
            if (m_sendSocket->Bind(local) != 0) {
                std::cerr << "[NIC] ERROR: Bind m_sendSocket failed\n";
                return;
            }
            m_sendBound = true;
            std::cout << "[NIC] m_sendSocket bound on " << GetLocalAddress() << "\n";
        }

        while (!m_sendQueue.empty()) {
            Ptr<Packet> pkt = m_sendQueue.front();
            m_sendQueue.pop();

            if (m_peerAddr == Ipv4Address::GetAny()) {
                std::cerr << "[NIC] ERROR: m_peerAddr not set, dropping packet.\n";
                return;
            }
            InetSocketAddress dest(m_peerAddr, m_port);
            Address a = dest;

            std::cout << "[NIC] Sending to " << m_peerAddr
                      << ":" << m_port
                      << " (queue left=" << m_sendQueue.size() << ")\n";

            m_sendSocket->SendTo(pkt, 0, a);

            RoceHeaderTag tag;
            pkt->PeekPacketTag(tag);
            m_sentPsn.insert(tag.GetPsn());

            NS_LOG_INFO("NIC sent PSN=" << tag.GetPsn());
            Simulator::Schedule(m_retransmitTimeout,
                                &RoceNic::CheckAckTimeout, this, tag.GetPsn());
        }
    }
    void RoceNic::HandleRead(Ptr<Socket> socket) {
        Address from;
        Ptr<Packet> packet = socket->RecvFrom(from);
        RoceHeaderTag tag;
        if (packet->PeekPacketTag(tag)) {
            std::cout << "NIC received packet with PSN=" << tag.GetPsn() << std::endl;
            if (InetSocketAddress::IsMatchingType(from)) {
                InetSocketAddress inet = InetSocketAddress::ConvertFrom(from);
                Ipv4Address src = inet.GetIpv4();

            } else {
                std::cout << "from non è InetSocketAddress" << std::endl;
            }
        }
    }

    void RoceNic::SendAck(uint32_t psn, ns3::Address to) {
        Ptr<Packet> ack = Create<Packet>(0);
        RoceHeaderTag ackTag(0x5678, 0xFF, psn, psn, GetLocalAddress());
        ack->AddPacketTag(ackTag);
        m_sendSocket->SendTo(ack, 0, to);
        std::cout << "NIC sent ACK for PSN=" << psn << std::endl;
    }

    void RoceNic::CheckAckTimeout(uint32_t psn) {
        if (m_sentPsn.count(psn)) {
            std::cout << "NIC timeout for PSN=" << psn << ", retransmitting NOT YET IMPLEMENTED" << std::endl;
            // TODO: Retransmit packet with this PSN (if needed)
        }
    }

    /*Ipv4Address RoceNic::GetLocalAddress() const {

        Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();
        return ipv4->GetAddress(1, 0).GetLocal(); // Usa la prima interfaccia utile
    }*/

    Ipv4Address RoceNic::GetLocalAddress() const {
        Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();

        if (!ipv4) {
            std::cerr << "[NIC] Errore: ipv4 non trovato" << std::endl;
            return Ipv4Address::GetAny();
        }

        int32_t nInterfaces = ipv4->GetNInterfaces();
        std::cout << "[NIC] Numero interfacce trovate: " << nInterfaces << std::endl;

        for (uint32_t i = 0; i < ipv4->GetNInterfaces(); ++i) {
            for (uint32_t j = 0; j < ipv4->GetNAddresses(i); ++j) {
                std::cout << "Interfaccia " << i << ", address " << j << ": "
                          << ipv4->GetAddress(i, j).GetLocal() << std::endl;
            }
        }

        return ipv4->GetAddress(1, 0).GetLocal(); // ATTENZIONE: può fallire
    }

} // namespace ns3