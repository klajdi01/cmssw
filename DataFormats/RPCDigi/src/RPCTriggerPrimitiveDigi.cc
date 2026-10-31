#include "DataFormats/RPCDigi/interface/RPCTriggerPrimitiveDigi.h"
#include <iostream>

namespace io_v1 {
  RPCTriggerPrimitiveDigi::RPCTriggerPrimitiveDigi(int stripLow, int stripHi, int bx)
      : stripLow_(stripLow), stripHi_(stripHi), bx_(bx) {}

  RPCTriggerPrimitiveDigi::RPCTriggerPrimitiveDigi() : stripLow_(0), stripHi_(0), bx_(0) {}

  bool RPCTriggerPrimitiveDigi::operator==(const RPCTriggerPrimitiveDigi& digi) const {
    return stripLow_ == digi.stripLow_ && stripHi_ == digi.stripHi_ && bx_ == digi.bx_;
  }

  bool RPCTriggerPrimitiveDigi::operator<(const RPCTriggerPrimitiveDigi& digi) const {
    if (bx_ != digi.bx_)
      return bx_ < digi.bx_;
    return stripLow_ < digi.stripLow_;
  }

  std::ostream& operator<<(std::ostream& o, const RPCTriggerPrimitiveDigi& digi) {
    return o << " strips " << digi.stripLow() << "-" << digi.stripHi() << " bx " << digi.bx();
  }

  void RPCTriggerPrimitiveDigi::print() const {
    std::cout << "Strips " << stripLow() << "-" << stripHi() << " bx " << bx() << std::endl;
  }
}  // namespace io_v1
