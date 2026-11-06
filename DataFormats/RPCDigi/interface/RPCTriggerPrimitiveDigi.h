#ifndef DataFormats_RPCDigi_RPCTriggerPrimitiveDigi_h
#define DataFormats_RPCDigi_RPCTriggerPrimitiveDigi_h

/** RPC trigger primitive: a cluster of adjacent strips (numbered from 1) in one roll and one BX,
 *  optionally with its mean time (ns, 0 for a prompt muon) and local y (cm, 0 at the roll centre). */

#include <cstdint>
#include <iosfwd>

namespace io_v1 {
  class RPCTriggerPrimitiveDigi {
  public:
    explicit RPCTriggerPrimitiveDigi(int stripLow, int stripHi, int bx);
    RPCTriggerPrimitiveDigi();

    /// Compares the strips and the BX only, as RPCDigi does
    bool operator==(const RPCTriggerPrimitiveDigi& digi) const;
    /// Order by BX, then by first strip
    bool operator<(const RPCTriggerPrimitiveDigi& digi) const;

    int stripLow() const { return stripLow_; }
    int stripHi() const { return stripHi_; }
    /// Central strip of the cluster, rounded down (in full-strip units)
    int strip() const { return (stripLow_ + stripHi_) / 2; }
    int clusterSize() const { return stripHi_ - stripLow_ + 1; }
    int bx() const { return bx_; }
    bool isValid() const { return stripLow_ > 0 && stripHi_ >= stripLow_; }

    bool hasTime() const { return hasTime_; }
    float time() const { return time_; }
    void setTime(float time) {
      time_ = time;
      hasTime_ = true;
    }
    /// Sub-BX of the time, in steps of 25/16 = 1.5625 ns from 0 to 15, as in the RPC digitizer
    /// (RPCSynchronizer::getBX_SBX): floor(time / 1.5625 ns) modulo 16; 0 without time
    int subbx() const;

    bool hasY() const { return hasY_; }
    float y() const { return y_; }
    void setY(float y) {
      y_ = y;
      hasY_ = true;
    }

    void print() const;

  private:
    uint16_t stripLow_;
    uint16_t stripHi_;
    int16_t bx_;
    float time_;
    float y_;
    bool hasTime_;
    bool hasY_;
  };

  std::ostream& operator<<(std::ostream& o, const RPCTriggerPrimitiveDigi& digi);
}  // namespace io_v1
using RPCTriggerPrimitiveDigi = io_v1::RPCTriggerPrimitiveDigi;
#endif
