#include "roce-nic.h"
#include "roce-header-tag.h"
#include "ns3/log.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("RoceNic");

    RoceNic::RoceNic()
            : m_port(4791), m_psn(100){}

    RoceNic::~RoceNic() {}

    void RoceNic::Setup(Ipv4Address peerAddr, uint16_t port) {
        m_peerAddr = peerAddr;
        m_port = port;
    }

    void RoceNic::StartApplication() {
        m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
        m_socket->Bind(local);
        m_socket->SetRecvCallback(MakeCallback(&RoceNic::HandleRead, this));

        m_sendSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    }

    void RoceNic::StopApplication() {
        if (m_socket) m_socket->Close();
        if (m_sendSocket) m_sendSocket->Close();
    }

    void RoceNic::Send(Ptr<Packet> packet, Address destination) {
        RoceHeaderTag tag(0x1234, 0x1A, m_psn, 0xDEADBEEF, GetLocalAddress());
        packet->AddPacketTag(tag);

        m_sendQueue.push(packet);
        m_unackedPackets[m_psn] = packet->Copy();
        Simulator::ScheduleNow(&RoceNic::ProcessSendQueue, this);
        m_psn++;
    }

    void RoceNic::ProcessSendQueue() {
        if (!m_sendQueue.empty()) {
            Ptr<Packet> pkt = m_sendQueue.front();
            m_sendQueue.pop();

            m_sendSocket->SendTo(pkt, 0, InetSocketAddress(m_peerAddr, m_port));
            RoceHeaderTag tag;
            pkt->PeekPacketTag(tag);
            m_sentPsn.insert(tag.GetPsn());

            std::cout << "NIC sent PSN=" << tag.GetPsn();
            Simulator::Schedule(Seconds(0.01), &RoceNic::CheckAckTimeout, this, tag.GetPsn());
        }
    }

    void RoceNic::HandleRead(Ptr<Socket> socket) {
        Address from;
        Ptr<Packet> packet;

        while ((packet = socket->RecvFrom(from))) {
            RoceHeaderTag tag;
            if (packet->PeekPacketTag(tag)) {
                if (tag.GetOpcode() == 0xFF) { // ACK
                    HandleAck(tag);
                } else{
                    HandleDataPacket(tag, packet, from);
                }
                /*
                std::cout << "NIC received PSN=" << tag.GetPsn();

                if (m_receivedPsn.count(tag.GetPsn()) == 0) {
                    m_receivedPsn.insert(tag.GetPsn());
                    if (tag.GetPsn() == m_expectedPsn) {
                        m_expectedPsn++;

                        while (m_reorderBuffer.count(m_expectedPsn)) {
                            m_reorderBuffer.erase(m_expectedPsn);
                            m_expectedPsn++;
                        }
                    } else {
                        m_reorderBuffer[tag.GetPsn()] = packet;
                    }

                    // Send ACK
                    Ptr<Packet> ack = Create<Packet>(0);
                    RoceHeaderTag ackTag(0x5678, 0xFF, tag.GetPsn(), tag.GetPsn(), GetLocalAddress());
                    ack->AddPacketTag(ackTag);
                    m_sendSocket->SendTo(ack, 0, from);
                    std::cout << "NIC sent ACK for PSN=" << tag.GetPsn();

                }*/
            }
        }
    }

    void RoceNic::HandleAck(const RoceHeaderTag& tag) {
        std::cout << "NIC received ACK for PSN=" << tag.GetPsn() << std::endl;
        m_sentPsn.erase(tag.GetPsn());
        m_unackedPackets.erase(tag.GetPsn());
    }

    void RoceNic::HandleDataPacket(const RoceHeaderTag& tag, Ptr<Packet> pkt, const Address& from) {
        std::cout << "NIC received PSN=" << tag.GetPsn() << std::endl;

        if (m_receivedPsn.count(tag.GetPsn()) == 0) {
            m_receivedPsn.insert(tag.GetPsn());

            if (tag.GetPsn() == m_expectedPsn) {
                m_expectedPsn++;

                while (m_reorderBuffer.count(m_expectedPsn)) {
                    m_reorderBuffer.erase(m_expectedPsn);
                    m_expectedPsn++;
                }
            } else {
                m_reorderBuffer[tag.GetPsn()] = pkt;
            }

            // ACK (simulate NIC delay)
            Simulator::Schedule(MicroSeconds(5), [this, tag, from]() {
                Ptr<Packet> ack = Create<Packet>(0);
                RoceHeaderTag ackTag(0x5678, 0xFF, tag.GetPsn(), tag.GetPsn(), GetLocalAddress());
                ack->AddPacketTag(ackTag);
                m_sendSocket->SendTo(ack, 0, from);
                std::cout << "NIC sent ACK for PSN=" << tag.GetPsn() << std::endl;
            });
        }
    }

    void RoceNic::CheckAckTimeout(uint32_t psn) {
        if (m_sentPsn.count(psn)) {
            std::cout << "NIC timeout for PSN=" << psn << ", retransmitting" << std::endl;
            RetransmitPacket(psn);
        }
    }

    void RoceNic::RetransmitPacket(uint32_t psn) {
        if (m_unackedPackets.count(psn)) {
            Ptr<Packet> pkt = m_unackedPackets[psn]->Copy();
            m_sendSocket->SendTo(pkt, 0, InetSocketAddress(m_peerAddr, m_port));
            Simulator::Schedule(Seconds(0.01), &RoceNic::CheckAckTimeout, this, psn);
            std::cout << "NIC retransmitted PSN=" << psn << std::endl;
        }
    }

    Ipv4Address RoceNic::GetLocalAddress() const {
        Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();
        return ipv4->GetAddress(1, 0).GetLocal();
    }

} // namespace ns3


