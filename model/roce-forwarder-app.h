#ifndef ROCE_FORWARDER_APP_H
#define ROCE_FORWARDER_APP_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "roce-header-tag.h"
#include <string>

namespace ns3 {

class RoceForwarderApp : public Application {
public:
  RoceForwarderApp();
  virtual ~RoceForwarderApp();
  void Setup(Ipv4Address forwardAddress, uint16_t port, std::string type);

private:
  virtual void StartApplication() override;
  virtual void StopApplication() override;
  void HandleRead(Ptr<Socket> socket);

  Ptr<Socket> m_socket;
  Ipv4Address m_forwardAddress;
  uint16_t m_port;
  std::string m_type;
};

} // namespace ns3

#endif // ROCE_FORWARDER_APP_H
