#ifndef ROCE_SERVER_APP_H
#define ROCE_SERVER_APP_H

#include "ns3/application.h"
#include "ns3/address.h"
#include "ns3/socket.h"
#include "roce-nic.h"


namespace ns3 {

    class RoceServerApp : public Application {
    public:
        RoceServerApp();
        virtual ~RoceServerApp();

        void Setup(Address address, uint16_t port);
        uint32_t GetPacketsReceived() const;
        void SetNic(Ptr<RoceNic> nic);

    private:
        virtual void StartApplication() override;
        virtual void StopApplication() override;

        void HandlePacket(Ptr<Packet> pkt);

        Ptr<RoceNic> m_nic;
        Address m_peer;
        uint16_t m_port;
        uint32_t m_packetsReceived;
    };

} // namespace ns3

#endif // ROCE_SERVER_APP_H