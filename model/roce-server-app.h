#ifndef ROCE_SERVER_APP_H
#define ROCE_SERVER_APP_H

#include "ns3/application.h"
#include "ns3/address.h"
#include "ns3/socket.h"
#include "roce-nic.h"


namespace ns3 {

    class RoceServerApp : public Application {
    public:
        static TypeId GetTypeId()
        {
            static TypeId tid = TypeId("ns3::RoceServerApp")
                    .SetParent<Application>()
                    .SetGroupName("Applications")
                    .AddConstructor<RoceServerApp>();
            return tid;
        }
        RoceServerApp();
        ~RoceServerApp() override;

        void Setup(InetSocketAddress address);
        uint32_t GetPacketsReceived() const;
        void SetNic(Ptr<RoceNic> nic);

    private:
        virtual void StartApplication() override;
        virtual void StopApplication() override;

        void HandlePacket(Ptr<Packet> pkt);

        void OnNicReceive(Ptr<Packet> pkt);

        Ptr<RoceNic> m_nic;
        uint32_t m_packetsReceived = 0;
        bool m_connected = false;
        Address m_peer;
        uint16_t m_port;
    };

} // namespace ns3

#endif // ROCE_SERVER_APP_H