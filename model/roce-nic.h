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

        void Setup(uint16_t port = 4791);
        void Send(Ptr<Packet> pkt);

        void SetPeer(Ipv4Address ip);
        void SetReceiveCallback(Callback<void, Ptr<Packet>> cb);

        Ipv4Address GetPeerAddress();

        uint32_t GetReceivedPsn();


        void SetAckCallback(Callback<void, uint32_t> cb);

        uint32_t GetAckReceived();
        uint32_t GetAckSent();
        uint32_t GetDataReceived();
        uint32_t GetDataSent();


    private:
        virtual void StartApplication() override;
        virtual void StopApplication() override;

        void HandleRead(Ptr<Socket> socket);
        void ProcessSendQueue();
        void CheckAckTimeout(uint32_t psn);

        void SendAck(uint32_t psn, Address to);

        Ipv4Address GetLocalAddress() const;

        Ptr<Socket> m_socket;
        Ptr<Socket> m_sendSocket;

        Ipv4Address m_peerAddr;
        uint16_t m_port;
        uint32_t m_psn = 100;

        bool m_sendBound = false;
        bool m_sending = false;

        std::set<uint32_t> m_sentPsn;
        std::set<uint32_t> m_receivedPsn;
        std::map<uint32_t, Ptr<Packet>> m_reorderBuffer;
        uint32_t m_expectedPsn = 100;

        Callback<void, uint32_t> m_ackCb{};
        uint32_t m_ackReceived = 0;
        uint32_t m_ackSent = 0;
        uint32_t m_dataReceived = 0;
        uint32_t m_dataSent = 0;

        std::queue<Ptr<Packet>> m_sendQueue;
        Callback<void, Ptr<Packet>> m_receiveCallback;

        EventId m_retransmitEvent;
        Time m_retransmitTimeout = Seconds(0.01);

    };


} // namespace ns3

#endif // ROCE_NIC_H