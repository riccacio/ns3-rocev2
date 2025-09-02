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
        RoceClientApp();
        virtual ~RoceClientApp();

        void Setup(Address path1, Address path2, uint32_t packetSize, uint32_t numPackets, Time interval, Ptr<RoceNic> nic);

        uint32_t GetPacketsReceived() const;
        void SetNic(Ptr<RoceNic> nic);

    private:
        virtual void StartApplication() override;
        virtual void StopApplication() override;

        void SendPacket();
        void HandleRead(Ptr<Socket> socket);

        Ptr<Socket> m_socket;
        Address m_peerPath1;
        Address m_peerPath2;
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