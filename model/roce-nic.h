#ifndef ROCE_NIC_H
#define ROCE_NIC_H

#include "ns3/application.h"
#include "ns3/ipv4-address.h"
#include "ns3/socket.h"
#include "ns3/packet.h"
#include "ns3/event-id.h"

#include "roce-header-tag.h"

#include <map>
#include <set>
#include <queue>

namespace ns3 {

    class RoceNic : public Application {
    public:
        static TypeId GetTypeId();

        RoceNic();
        virtual ~RoceNic();

        void Setup(Ipv4Address remoteIp, uint16_t port);
        void Send(Ptr<Packet> pkt);

    private:
        virtual void StartApplication() override;
        virtual void StopApplication() override;

        void ReceivePacket(Ptr<Socket> socket);
        void ScheduleRetransmission();
        void Retransmit();

        void SendAck(uint32_t psn);

        void CheckAckTimeout(uint32_t psn);
        void ProcessSendQueue();
        void HandleRead(Ptr<Socket> socket);
        Ipv4Address GetLocalAddress() const;

        void HandleDataPacket(const RoceHeaderTag& tag, Ptr<Packet> pkt, const Address& from);
        void HandleAck(const RoceHeaderTag& tag);
        void RetransmitPacket(uint32_t psn);

        std::map<uint32_t, Ptr<Packet>> m_unackedPackets;

        Ptr<Socket> m_socket;
        Ptr<Socket> m_sendSocket;
        Ipv4Address m_peerAddr;
        uint16_t m_port;
        uint16_t m_psn;
        std::set<uint32_t> m_sentPsn;


        uint32_t m_expectedPsn;
        std::set<uint32_t> m_receivedPsn;
        std::map<uint32_t, Ptr<Packet>> m_reorderBuffer;
        std::queue<Ptr<Packet>> m_sendQueue; // coda

        EventId m_retransmitEvent;
        Time m_retransmitTimeout;
    };

} // namespace ns3

#endif // ROCE_NIC_H