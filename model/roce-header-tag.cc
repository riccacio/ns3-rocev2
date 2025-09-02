#include "roce-header-tag.h"
#include "ns3/ipv4-address.h"
namespace ns3 {


RoceHeaderTag::RoceHeaderTag() : m_qpn(0), m_opcode(0), m_psn(0), m_imm(0) {}
RoceHeaderTag::RoceHeaderTag(uint32_t qpn, uint8_t opcode, uint32_t psn, uint32_t imm, Ipv4Address clientIp)
  : m_qpn(qpn), m_opcode(opcode), m_psn(psn), m_imm(imm), m_clientIp(clientIp) {}

RoceHeaderTag::~RoceHeaderTag () {}

TypeId RoceHeaderTag::GetTypeId(void) {
  static TypeId tid = TypeId("ns3::RoceHeaderTag")
    .SetParent<Tag>()
    .AddConstructor<RoceHeaderTag>();
  return tid;
}

TypeId RoceHeaderTag::GetInstanceTypeId(void) const {
  return GetTypeId();
}

void RoceHeaderTag::Serialize(TagBuffer i) const {
  i.WriteU32(m_qpn);
  i.WriteU8(m_opcode);
  i.WriteU32(m_psn);
  i.WriteU32(m_imm);
  i.WriteU32(m_clientIp.Get());
}

void RoceHeaderTag::Deserialize(TagBuffer i) {
  m_qpn = i.ReadU32();
  m_opcode = i.ReadU8();
  m_psn = i.ReadU32();
  m_imm = i.ReadU32();
  m_clientIp = Ipv4Address(i.ReadU32());
}

uint32_t RoceHeaderTag::GetSerializedSize(void) const {
  return 4 + 1 + 4 + 4 + 4;
}

void RoceHeaderTag::Print(std::ostream &os) const {
  os << "QPN=" << m_qpn
     << ", Opcode=" << static_cast<uint32_t>(m_opcode)
     << ", PSN=" << m_psn
     << ", IMM=" << m_imm;
}

void RoceHeaderTag::SetFields(uint32_t qpn, uint8_t opcode, uint32_t psn, uint32_t imm) {
  m_qpn = qpn;
  m_opcode = opcode;
  m_psn = psn;
  m_imm = imm;
}

uint32_t RoceHeaderTag::GetQpn() const { return m_qpn; }
uint8_t RoceHeaderTag::GetOpcode() const { return m_opcode; }
uint32_t RoceHeaderTag::GetPsn() const { return m_psn; }
uint32_t RoceHeaderTag::GetImm() const { return m_imm; }
Ipv4Address RoceHeaderTag::GetClientIp() const { return m_clientIp; }
void RoceHeaderTag::SetClientIp(Ipv4Address ip) { m_clientIp = ip; }

std::ostream& operator<<(std::ostream& os, const RoceHeaderTag& tag){
    tag.Print(os);
    return os;
}
} // namespace ns3