#ifndef ROCE_HEADER_TAG_H
#define ROCE_HEADER_TAG_H

#include "ns3/tag.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-address.h"
#include <iostream>


namespace ns3 {

class RoceHeaderTag : public Tag {
public:
  RoceHeaderTag();
  RoceHeaderTag(uint32_t qpn, uint8_t opcode, uint32_t psn, uint32_t imm, Ipv4Address clientIp);

  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const override;
  virtual void Serialize(TagBuffer i) const override;
  virtual void Deserialize(TagBuffer i) override;
  virtual uint32_t GetSerializedSize(void) const override;
  virtual void Print(std::ostream &os) const override;

  void SetFields(uint32_t qpn, uint8_t opcode, uint32_t psn, uint32_t imm);
  uint32_t GetQpn() const;
  uint8_t GetOpcode() const;
  uint32_t GetPsn() const;
  uint32_t GetImm() const;
  Ipv4Address GetClientIp() const;
  void SetClientIp(Ipv4Address ip);

private:
  uint32_t m_qpn;
  uint8_t m_opcode;
  uint32_t m_psn;
  uint32_t m_imm;
  Ipv4Address m_clientIp;
};

std::ostream& operator<<(std::ostream& os, const RoceHeaderTag& tag);

} // namespace ns3

#endif // ROCE_HEADER_TAG_H

