#ifndef L1Trigger_RPCTriggerPrimitives_RPCTriggerPrimitivesBuilder_h
#define L1Trigger_RPCTriggerPrimitives_RPCTriggerPrimitivesBuilder_h

/** \class RPCTriggerPrimitivesBuilder
 *
 * Builds RPC trigger primitives from RPC digis: in each roll, fired strips
 * in the same BX are sorted, de-duplicated and adjacent strips are merged
 * into one cluster. This is the same nearest-neighbour clustering that
 * EMTFSubsystemCollector::cluster_rpc() performs in the EMTF emulator.
 *
 * Output clusters are ordered by (BX, first strip) within each roll.
 */

#include "DataFormats/RPCDigi/interface/RPCDigiCollection.h"
#include "DataFormats/RPCDigi/interface/RPCTriggerPrimitiveDigiCollection.h"

#include <vector>

class RPCTriggerPrimitivesBuilder {
public:
  struct Config {
    bool includeBarrel = false;
    // Maximum cluster size (in strips) kept in the output; a negative value disables the cut
    int maxClusterSizeRPC = -1;
    int maxClusterSizeIRPC = -1;
  };

  explicit RPCTriggerPrimitivesBuilder(const Config& config) : config_(config) {}

  void build(const RPCDigiCollection& digis, RPCTriggerPrimitiveDigiCollection& out) const;

  // Clusters the digis of a single roll
  static std::vector<RPCTriggerPrimitiveDigi> cluster(const std::vector<RPCDigi>& digis);

  static bool isIRPC(const RPCDetId& id) { return id.region() != 0 && id.station() >= 3 && id.ring() == 1; }

private:
  bool passClusterSize(const RPCDetId& id, const RPCTriggerPrimitiveDigi& tp) const;

  const Config config_;
};

#endif
