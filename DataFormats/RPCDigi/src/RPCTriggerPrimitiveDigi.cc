#include "DataFormats/RPCDigi/interface/RPCTriggerPrimitiveDigi.h"
#include <cmath>
#include <iostream>

namespace io_v1 {
  RPCTriggerPrimitiveDigi::RPCTriggerPrimitiveDigi(int stripLow, int stripHi, int bx)
      : stripLow_(stripLow), stripHi_(stripHi), bx_(bx), time_(0.f), y_(0.f), hasTime_(false), hasY_(false) {}

  RPCTriggerPrimitiveDigi::RPCTriggerPrimitiveDigi()
      : stripLow_(0), stripHi_(0), bx_(0), time_(0.f), y_(0.f), hasTime_(false), hasY_(false) {}

  bool RPCTriggerPrimitiveDigi::operator==(const RPCTriggerPrimitiveDigi& digi) const {
    return stripLow_ == digi.stripLow_ && stripHi_ == digi.stripHi_ && bx_ == digi.bx_;
  }

  bool RPCTriggerPrimitiveDigi::operator<(const RPCTriggerPrimitiveDigi& digi) const {
    if (bx_ != digi.bx_)
      return bx_ < digi.bx_;
    return stripLow_ < digi.stripLow_;
  }

  int RPCTriggerPrimitiveDigi::subbx() const {
    if (!hasTime_)
      return 0;
    const int subbx = static_cast<int>(std::floor(time_ * 16. / 25.)) % 16;
    return subbx < 0 ? subbx + 16 : subbx;
  }

  std::ostream& operator<<(std::ostream& o, const RPCTriggerPrimitiveDigi& digi) {
    o << " strips " << digi.stripLow() << "-" << digi.stripHi() << " bx " << digi.bx();
    if (digi.hasTime())
      o << " time " << digi.time() << " subbx " << digi.subbx();
    if (digi.hasY())
      o << " y " << digi.y();
    return o;
  }

  void RPCTriggerPrimitiveDigi::print() const { std::cout << "RPCTriggerPrimitiveDigi" << *this << std::endl; }
}  // namespace io_v1
