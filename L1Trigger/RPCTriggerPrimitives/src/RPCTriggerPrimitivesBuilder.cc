#include "L1Trigger/RPCTriggerPrimitives/interface/RPCTriggerPrimitivesBuilder.h"

#include "DataFormats/RPCDigi/interface/IRPCDigiTime.h"

#include <algorithm>
#include <tuple>
#include <utility>

void RPCTriggerPrimitivesBuilder::build(const RPCDigiCollection& digis,
                                        const IRPCDigiCollection* irpcDigis,
                                        RPCTriggerPrimitiveDigiCollection& out) const {
  for (auto const& [detId, range] : digis) {
    if (detId.region() == 0 && !config_.includeBarrel)
      continue;
    if (irpcDigis != nullptr && isIRPC(detId))
      continue;  // taken from the iRPC digis below

    std::vector<RPCDigi> rollDigis(range.first, range.second);
    std::vector<RPCTriggerPrimitiveDigi> clusters = cluster(rollDigis);
    put(detId, clusters, out);
  }

  if (irpcDigis == nullptr)
    return;

  for (auto const& [detId, range] : *irpcDigis) {
    if (!isIRPC(detId))
      continue;

    std::vector<IRPCDigi> rollDigis(range.first, range.second);
    std::vector<RPCTriggerPrimitiveDigi> clusters = clusterIRPC(rollDigis);
    put(detId, clusters, out);
  }
}

std::vector<RPCTriggerPrimitiveDigi> RPCTriggerPrimitivesBuilder::cluster(const std::vector<RPCDigi>& digis) {
  // Digis sorted by (bx, strip), keeping only the first digi of each strip
  std::vector<const RPCDigi*> hits;
  hits.reserve(digis.size());
  for (auto const& digi : digis)
    hits.push_back(&digi);
  auto key = [](const RPCDigi* digi) { return std::make_pair(digi->bx(), digi->strip()); };
  std::stable_sort(
      hits.begin(), hits.end(), [&](const RPCDigi* lhs, const RPCDigi* rhs) { return key(lhs) < key(rhs); });
  hits.erase(std::unique(hits.begin(),
                         hits.end(),
                         [&](const RPCDigi* lhs, const RPCDigi* rhs) { return key(lhs) == key(rhs); }),
             hits.end());

  std::vector<RPCTriggerPrimitiveDigi> clusters;
  auto it = hits.begin();
  while (it != hits.end()) {
    const int bx = (*it)->bx();
    const int stripLow = (*it)->strip();
    int stripHi = stripLow;
    double sumTime = 0., sumY = 0.;
    int nTime = 0, nY = 0;
    // Extend the cluster while the next strip in the same BX is adjacent
    do {
      if ((*it)->hasTime()) {
        sumTime += (*it)->time();
        ++nTime;
      }
      if ((*it)->hasY()) {
        sumY += (*it)->coordinateY();
        ++nY;
      }
      stripHi = (*it)->strip();
    } while (++it != hits.end() && (*it)->bx() == bx && (*it)->strip() == stripHi + 1);

    RPCTriggerPrimitiveDigi tp(stripLow, stripHi, bx);
    if (nTime > 0)
      tp.setTime(static_cast<float>(sumTime / nTime));
    if (nY > 0)
      tp.setY(static_cast<float>(sumY / nY));
    clusters.push_back(tp);
  }
  return clusters;
}

std::vector<RPCTriggerPrimitiveDigi> RPCTriggerPrimitivesBuilder::clusterIRPC(
    const std::vector<IRPCDigi>& digis) const {
  struct Hit {
    int bx;
    int strip;
    float time;
    float y;
  };

  std::vector<Hit> hits;
  hits.reserve(digis.size());
  for (auto const& digi : digis) {
    IRPCDigiTime digiTime(digi);
    hits.push_back({digi.bx(), digi.strip(), digiTime.time(), digiTime.coordinateY()});
  }

  // Same order as IRPCClusterizer: BX and strip descending, then time and y ascending
  auto key = [](const Hit& hit) { return std::make_tuple(-hit.bx, -hit.strip, hit.time, hit.y); };
  std::sort(hits.begin(), hits.end(), [&](const Hit& lhs, const Hit& rhs) { return key(lhs) < key(rhs); });
  hits.erase(
      std::unique(hits.begin(), hits.end(), [&](const Hit& lhs, const Hit& rhs) { return key(lhs) == key(rhs); }),
      hits.end());

  std::vector<RPCTriggerPrimitiveDigi> clusters;
  auto it = hits.begin();
  while (it != hits.end()) {
    const int bx = it->bx;
    const int stripHi = it->strip;
    int stripLow = stripHi;
    float minTime = it->time, maxTime = it->time, sumTime = it->time;
    float minY = it->y, maxY = it->y, sumY = it->y;
    int n = 1;

    // Add the same strip or the next lower one, while the time and y spreads stay small enough
    for (++it; it != hits.end(); ++it) {
      if (it->bx != bx || (it->strip != stripLow && it->strip != stripLow - 1))
        break;
      if (std::max(maxTime, it->time) - std::min(minTime, it->time) > config_.maxTimeSpreadIRPC)
        break;
      if (std::max(maxY, it->y) - std::min(minY, it->y) > config_.maxYSpreadIRPC)
        break;

      stripLow = it->strip;
      minTime = std::min(minTime, it->time);
      maxTime = std::max(maxTime, it->time);
      sumTime += it->time;
      minY = std::min(minY, it->y);
      maxY = std::max(maxY, it->y);
      sumY += it->y;
      ++n;
    }

    RPCTriggerPrimitiveDigi tp(stripLow, stripHi, bx);
    tp.setTime(sumTime / n);
    tp.setY(sumY / n);
    clusters.push_back(tp);
  }

  std::stable_sort(clusters.begin(), clusters.end());  // by BX, then first strip
  return clusters;
}

void RPCTriggerPrimitivesBuilder::put(const RPCDetId& id,
                                      std::vector<RPCTriggerPrimitiveDigi>& clusters,
                                      RPCTriggerPrimitiveDigiCollection& out) const {
  clusters.erase(std::remove_if(clusters.begin(),
                                clusters.end(),
                                [&](const RPCTriggerPrimitiveDigi& tp) { return !passClusterSize(id, tp); }),
                 clusters.end());

  if (!clusters.empty())
    out.put(RPCTriggerPrimitiveDigiCollection::Range(clusters.cbegin(), clusters.cend()), id);
}

bool RPCTriggerPrimitivesBuilder::passClusterSize(const RPCDetId& id, const RPCTriggerPrimitiveDigi& tp) const {
  const int maxSize = isIRPC(id) ? config_.maxClusterSizeIRPC : config_.maxClusterSizeRPC;
  return maxSize < 0 || tp.clusterSize() <= maxSize;
}
