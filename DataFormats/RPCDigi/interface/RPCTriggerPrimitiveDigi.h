#ifndef DataFormats_RPCDigi_RPCTriggerPrimitiveDigi_h
#define DataFormats_RPCDigi_RPCTriggerPrimitiveDigi_h

/** \class RPCTriggerPrimitiveDigi
 *
 * RPC trigger primitive: a cluster of adjacent fired strips in one roll
 * and one bunch crossing, as produced by RPCTriggerPrimitivesProducer
 * (L1Trigger/RPCTriggerPrimitives) for the L1 muon track finders.
 *
 * Strips are numbered from 1, as in RPCDigi.
 */

#include <cstdint>
#include <iosfwd>

namespace io_v1 {
  class RPCTriggerPrimitiveDigi {
  public:
    explicit RPCTriggerPrimitiveDigi(int stripLow, int stripHi, int bx);
    RPCTriggerPrimitiveDigi();

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

    void print() const;

  private:
    uint16_t stripLow_;
    uint16_t stripHi_;
    int16_t bx_;
  };

  std::ostream& operator<<(std::ostream& o, const RPCTriggerPrimitiveDigi& digi);
}  // namespace io_v1
using RPCTriggerPrimitiveDigi = io_v1::RPCTriggerPrimitiveDigi;
#endif
