#include "L1Trigger/RPCTriggerPrimitives/interface/RPCTriggerPrimitivesBuilder.h"

#include <algorithm>
#include <utility>

void RPCTriggerPrimitivesBuilder::build(const RPCDigiCollection& digis, RPCTriggerPrimitiveDigiCollection& out) const {
  for (auto const& [detId, range] : digis) {
    if (detId.region() == 0 && !config_.includeBarrel)
      continue;

    std::vector<RPCDigi> rollDigis(range.first, range.second);
    std::vector<RPCTriggerPrimitiveDigi> clusters = cluster(rollDigis);

    clusters.erase(std::remove_if(clusters.begin(),
                                  clusters.end(),
                                  [&](const RPCTriggerPrimitiveDigi& tp) { return !passClusterSize(detId, tp); }),
                   clusters.end());

    if (!clusters.empty())
      out.put(RPCTriggerPrimitiveDigiCollection::Range(clusters.cbegin(), clusters.cend()), detId);
  }
}

std::vector<RPCTriggerPrimitiveDigi> RPCTriggerPrimitivesBuilder::cluster(const std::vector<RPCDigi>& digis) {
  // (bx, strip) pairs, sorted with duplicates removed
  std::vector<std::pair<int, int>> hits;
  hits.reserve(digis.size());
  for (auto const& digi : digis)
    hits.emplace_back(digi.bx(), digi.strip());
  std::sort(hits.begin(), hits.end());
  hits.erase(std::unique(hits.begin(), hits.end()), hits.end());

  std::vector<RPCTriggerPrimitiveDigi> clusters;
  auto it = hits.begin();
  while (it != hits.end()) {
    const int bx = it->first;
    const int stripLow = it->second;
    int stripHi = stripLow;
    // Extend the cluster while the next strip in the same BX is adjacent
    while (++it != hits.end() && it->first == bx && it->second == stripHi + 1)
      ++stripHi;
    clusters.emplace_back(stripLow, stripHi, bx);
  }
  return clusters;
}

bool RPCTriggerPrimitivesBuilder::passClusterSize(const RPCDetId& id, const RPCTriggerPrimitiveDigi& tp) const {
  const int maxSize = isIRPC(id) ? config_.maxClusterSizeIRPC : config_.maxClusterSizeRPC;
  return maxSize < 0 || tp.clusterSize() <= maxSize;
}
