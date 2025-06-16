#ifndef ROCE_FORWARDER_APP_H
#define ROCE_FORWARDER_APP_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/address.h"
#include "ns3/ipv4-address.h"


namespace ns3 {

    class RoceForwarderApp : public Application {
    public:
        RoceForwarderApp();
        virtual ~RoceForwarderApp();

        void Setup(Ipv4Address dest, uint16_t port, std::string id, Ipv4Address sender);

    protected:
        virtual void StartApplication() override;
        virtual void StopApplication() override;

    private:
        void HandleRead(Ptr<Socket> socket);

        Ptr<Socket> m_socket;
        Ptr<Socket> m_forwardSocket;
        Ipv4Address m_destAddr;
        Ipv4Address m_senderAddr;
        uint16_t m_port;
        std::string m_id;
    };

} // namespace ns3

#endif // ROCE_FORWARDER_APP_H