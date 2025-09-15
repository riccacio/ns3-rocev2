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
            : m_port(4791), m_psn(100), m_expectedPsn(100), m_retransmitTimeout(Seconds(0.01)){}

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

    void RoceNic::SetAckCallback(Callback<void, uint32_t> cb) {
        m_ackCb = cb;
    }

    Ipv4Address RoceNic::GetPeerAddress(){
        return m_peerAddr;
    }

    void RoceNic::StartApplication() {
        m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());

        m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port));

        m_socket->SetRecvCallback(MakeCallback(&RoceNic::HandleRead, this));

        m_sendSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());

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

        std::cout << "NIC ha messo in coda PSN=" << m_psn
                  << " (queue=" << m_sendQueue.size() << ")\n";

        Simulator::ScheduleNow(&RoceNic::ProcessSendQueue, this);
        m_psn++;
    }

    void RoceNic::ProcessSendQueue() {
        if(m_sendQueue.empty()) return;

        Ptr<Packet> pkt = m_sendQueue.front();
        m_sendQueue.pop();

        InetSocketAddress dest(m_peerAddr, m_port);

        std::cout << "[NIC] Inoltro a  " << m_peerAddr
                  << ":" << m_port
                  << " (rimasti nella queue=" << m_sendQueue.size() << ")\n";

        m_sendSocket->SendTo(pkt, 0, dest);

        RoceHeaderTag tag;
        pkt->PeekPacketTag(tag);
        m_sentPsn.insert(tag.GetPsn());
        m_dataSent++;

        std::cout << "NIC ha inoltrato PSN=" << tag.GetPsn() << std::endl;
        Simulator::Schedule(m_retransmitTimeout,
                            &RoceNic::CheckAckTimeout, this, tag.GetPsn());
    }

    void RoceNic::HandleRead(Ptr<Socket> socket) {
        Address from;
        Ptr<Packet> packet;

        while ((packet = socket->RecvFrom(from))) {
            RoceHeaderTag tag;
            if (!packet->PeekPacketTag(tag)) continue;

            if (tag.GetOpcode() == 0xFF) {
                // ACK ricevuto
                m_sentPsn.erase(tag.GetPsn());
                m_ackReceived++;
                if (!m_ackCb.IsNull()) m_ackCb(tag.GetPsn()); // notifica opzionale a chi vuole (es. ClientApp)
                continue;
            }

            // Data packet ricevuto
            m_dataReceived++;

            if (m_receivedPsn.insert(tag.GetPsn()).second) {
                if (tag.GetPsn() == m_expectedPsn) {
                    if (!m_receiveCallback.IsNull()) m_receiveCallback(packet);
                    m_expectedPsn++;
                    while (m_reorderBuffer.count(m_expectedPsn)) {
                        if (!m_receiveCallback.IsNull()) m_receiveCallback(m_reorderBuffer[m_expectedPsn]);
                        m_reorderBuffer.erase(m_expectedPsn);
                        m_expectedPsn++;
                    }
                } else {
                    m_reorderBuffer[tag.GetPsn()] = packet;
                }
                // ⬇️ Genera ACK con il NIC del server
                SendAck(tag.GetPsn(), from);
            }
        }
    }

    void RoceNic::SendAck(uint32_t psn, Address from) {
        Ptr<Packet> ack = Create<Packet>(0);
        RoceHeaderTag ackTag(0x5678, 0xFF, psn, psn, GetLocalAddress());
        ack->AddPacketTag(ackTag);

        // rimanda all'IP sorgente del pacchetto, forzando la porta 4791 del peer
        Ipv4Address srcIp = InetSocketAddress::ConvertFrom(from).GetIpv4();
        Address to = InetSocketAddress(srcIp, m_port);
        m_socket->SendTo(ack, 0, to);
        m_ackSent++;
    }

    void RoceNic::CheckAckTimeout(uint32_t psn) {
        if (m_sentPsn.count(psn)) {
            std::cout << "NIC timeout for PSN=" << psn << ", retransmitting NOT YET IMPLEMENTED" << std::endl;
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

        return ipv4->GetAddress(1, 0).GetLocal();
    }

    uint32_t RoceNic::GetReceivedPsn() {
        return m_dataReceived;
    }

    uint32_t RoceNic::GetAckReceived() {return m_ackReceived; };
    uint32_t RoceNic::GetAckSent(){return m_ackSent;};
    uint32_t RoceNic::GetDataReceived(){return m_dataReceived;};
    uint32_t RoceNic::GetDataSent(){return m_dataSent;};

} // namespace ns3