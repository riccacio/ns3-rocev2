#ifndef ROCE_CLIENT_APP_H
#define ROCE_CLIENT_APP_H

#include "ns3/application.h"
#include "ns3/address.h"
#include "ns3/socket.h"
#include "ns3/ipv4-address.h"
#include "roce-nic.h"


namespace ns3 {

    class RoceClientApp : public Application {
    public:
        RoceClientApp(InetSocketAddress peer1, InetSocketAddress peer2);
        virtual ~RoceClientApp();

        void Setup(InetSocketAddress path1, InetSocketAddress path2, uint32_t packetSize, uint32_t numPackets, Time interval, Ptr<RoceNic> nic);

        uint32_t GetPacketsReceived() const;
        uint32_t GetPacketsSent() const;
        void SetNic(Ptr<RoceNic> nic);

    private:
        virtual void StartApplication() override;
        virtual void StopApplication() override;

        void SendPacket();
        void HandleRead(Ptr<Socket> socket);

        Ptr<Socket> m_socket;
        InetSocketAddress m_peerPath1;
        InetSocketAddress m_peerPath2;
        uint32_t m_packetSize;
        uint32_t m_nPackets;
        Time m_interval;
        uint32_t m_sent;
        uint32_t m_psn;
        uint32_t m_packetsReceived;

        Ptr<RoceNic> m_nic;
    };

} // namespace ns3

#endif // ROCE_CLIENT_APP_H