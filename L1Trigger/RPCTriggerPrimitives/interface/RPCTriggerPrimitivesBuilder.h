#ifndef L1Trigger_RPCTriggerPrimitives_RPCTriggerPrimitivesBuilder_h
#define L1Trigger_RPCTriggerPrimitives_RPCTriggerPrimitivesBuilder_h

/** Builds RPC trigger primitives by merging adjacent strips of the same BX in each roll (as the EMTF emulator).
 *  With IRPCDigi (iRPC) and RPCDigiPhase2 (other chambers), strips are merged only within a time/sub-BX window. */

#include "DataFormats/RPCDigi/interface/IRPCDigiCollection.h"
#include "DataFormats/RPCDigi/interface/RPCDigiCollection.h"
#include "DataFormats/RPCDigi/interface/RPCDigiPhase2Collection.h"
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
    // Maximum spread of the strip sub-BX (1.5625 ns) in one RPC cluster; negative = no window
    int maxSubBXSpreadRPC = 2;
    // BX of the Phase-2 RPC and iRPC clusters = round(time / 25 ns) instead of the BX of the digis
    bool bxFromTime = false;
  };

  explicit RPCTriggerPrimitivesBuilder(const Config& config) : config_(config) {}

  // Builds all rolls; iRPC chambers from irpcDigis and the others from rpcDigisPhase2 when they are given
  void build(const RPCDigiCollection& digis,
             const RPCDigiPhase2Collection* rpcDigisPhase2,
             const IRPCDigiCollection* irpcDigis,
             RPCTriggerPrimitiveDigiCollection& out) const;
  void build(const RPCDigiCollection& digis,
             const IRPCDigiCollection* irpcDigis,
             RPCTriggerPrimitiveDigiCollection& out) const {
    build(digis, nullptr, irpcDigis, out);
  }
  void build(const RPCDigiCollection& digis, RPCTriggerPrimitiveDigiCollection& out) const {
    build(digis, nullptr, nullptr, out);
  }

  // Clusters the digis of a single roll
  static std::vector<RPCTriggerPrimitiveDigi> cluster(const std::vector<RPCDigi>& digis);

  // Clusters the Phase-2 RPC digis of a single roll
  std::vector<RPCTriggerPrimitiveDigi> clusterPhase2(const std::vector<RPCDigiPhase2>& digis) const;

  // Clusters the iRPC digis of a single roll
  std::vector<RPCTriggerPrimitiveDigi> clusterIRPC(const std::vector<IRPCDigi>& digis) const;

  static bool isIRPC(const RPCDetId& id) { return id.region() != 0 && id.station() >= 3 && id.ring() == 1; }

  // Nearest BX of a time in ns (time 0 = prompt muon in BX 0)
  static int bxOfTime(float time);

private:
  bool passClusterSize(const RPCDetId& id, const RPCTriggerPrimitiveDigi& tp) const;
  void put(const RPCDetId& id,
           std::vector<RPCTriggerPrimitiveDigi>& clusters,
           RPCTriggerPrimitiveDigiCollection& out) const;

  const Config config_;
};

#endif
