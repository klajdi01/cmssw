#ifndef L1Trigger_RPCTriggerPrimitives_RPCTriggerPrimitivesBuilder_h
#define L1Trigger_RPCTriggerPrimitives_RPCTriggerPrimitivesBuilder_h

/** \class RPCTriggerPrimitivesBuilder
 *
 * Builds RPC trigger primitives from RPC digis: in each roll, fired strips
 * in the same BX are sorted, de-duplicated and adjacent strips are merged
 * into one cluster. This is the same nearest-neighbour clustering that
 * EMTFSubsystemCollector::cluster_rpc() performs in the EMTF emulator.
 * If the digis carry a time and/or a local y (as the Phase-2 iRPC digis of
 * RPCSimModelTiming do), the cluster gets their mean values.
 *
 * iRPC chambers (RE3/1, RE4/1) can instead be built from IRPCDigi, which has
 * the times measured at both strip ends: the strip time and local y are
 * decoded with IRPCDigiTime, and adjacent strips are merged only if the
 * spread of their times and y positions stays within maxTimeSpreadIRPC and
 * maxYSpreadIRPC. This follows IRPCClusterizer (RecoLocalMuon/RPCRecHit),
 * so the clusters, their mean time and y are the same as in the Phase-2
 * RPC RecHits.
 *
 * Output clusters are ordered by (BX, first strip) within each roll.
 */

#include "DataFormats/RPCDigi/interface/IRPCDigiCollection.h"
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
    // Maximum spread of the strip times (ns) and local y (cm) in one iRPC cluster built from IRPCDigi
    float maxTimeSpreadIRPC = 3.f;
    float maxYSpreadIRPC = 9.f;
  };

  explicit RPCTriggerPrimitivesBuilder(const Config& config) : config_(config) {}

  // Builds the trigger primitives of all rolls. If irpcDigis is given, the iRPC chambers
  // are built from it and their RPCDigis are ignored.
  void build(const RPCDigiCollection& digis,
             const IRPCDigiCollection* irpcDigis,
             RPCTriggerPrimitiveDigiCollection& out) const;
  void build(const RPCDigiCollection& digis, RPCTriggerPrimitiveDigiCollection& out) const {
    build(digis, nullptr, out);
  }

  // Clusters the digis of a single roll
  static std::vector<RPCTriggerPrimitiveDigi> cluster(const std::vector<RPCDigi>& digis);

  // Clusters the iRPC digis of a single roll
  std::vector<RPCTriggerPrimitiveDigi> clusterIRPC(const std::vector<IRPCDigi>& digis) const;

  static bool isIRPC(const RPCDetId& id) { return id.region() != 0 && id.station() >= 3 && id.ring() == 1; }

private:
  bool passClusterSize(const RPCDetId& id, const RPCTriggerPrimitiveDigi& tp) const;
  void put(const RPCDetId& id,
           std::vector<RPCTriggerPrimitiveDigi>& clusters,
           RPCTriggerPrimitiveDigiCollection& out) const;

  const Config config_;
};

#endif
