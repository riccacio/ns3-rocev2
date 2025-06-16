#ifndef ROCE_SERVER_APP_H
#define ROCE_SERVER_APP_H

#include "ns3/application.h"
#include "ns3/address.h"
#include "ns3/socket.h"


namespace ns3 {

    class RoceServerApp : public Application {
    public:
        RoceServerApp();
        virtual ~RoceServerApp();

        void Setup(Address address, uint16_t port);
        uint32_t GetPacketsReceived() const;

    protected:
        virtual void StartApplication() override;
        virtual void StopApplication() override;

    private:
        void HandleRead(Ptr<Socket> socket);

        Ptr<Socket> m_socket;
        Address m_peer;
        uint16_t m_port;
        uint32_t m_packetsReceived;
    };

} // namespace ns3

#endif // ROCE_SERVER_APP_H