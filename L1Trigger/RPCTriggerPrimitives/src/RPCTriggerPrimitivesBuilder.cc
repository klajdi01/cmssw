#include "L1Trigger/RPCTriggerPrimitives/interface/RPCTriggerPrimitivesBuilder.h"

#include "DataFormats/RPCDigi/interface/IRPCDigiTime.h"
#include "DataFormats/RPCDigi/interface/RPCDigiPhase2Time.h"

#include <algorithm>
#include <cmath>
#include <tuple>
#include <utility>

namespace {
  constexpr int kSubBXPerBX = 16;  // RPCDigiPhase2 sub-BX of 25/16 = 1.5625 ns
}  // namespace

int RPCTriggerPrimitivesBuilder::bxOfTime(float time) { return static_cast<int>(std::round(time / 25.)); }

void RPCTriggerPrimitivesBuilder::build(const RPCDigiCollection& digis,
                                        const RPCDigiPhase2Collection* rpcDigisPhase2,
                                        const IRPCDigiCollection* irpcDigis,
                                        RPCTriggerPrimitiveDigiCollection& out) const {
  if (rpcDigisPhase2 == nullptr) {
    for (auto const& [detId, range] : digis) {
      if (detId.region() == 0 && !config_.includeBarrel)
        continue;
      if (irpcDigis != nullptr && isIRPC(detId))
        continue;  // taken from the iRPC digis below

      std::vector<RPCDigi> rollDigis(range.first, range.second);
      std::vector<RPCTriggerPrimitiveDigi> clusters = cluster(rollDigis);
      put(detId, clusters, out);
    }
  } else {
    for (auto const& [detId, range] : *rpcDigisPhase2) {
      if (detId.region() == 0 && !config_.includeBarrel)
        continue;
      if (irpcDigis != nullptr && isIRPC(detId))
        continue;  // taken from the iRPC digis below

      std::vector<RPCDigiPhase2> rollDigis(range.first, range.second);
      std::vector<RPCTriggerPrimitiveDigi> clusters = clusterPhase2(rollDigis);
      put(detId, clusters, out);
    }
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
  hits.erase(
      std::unique(
          hits.begin(), hits.end(), [&](const RPCDigi* lhs, const RPCDigi* rhs) { return key(lhs) == key(rhs); }),
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

std::vector<RPCTriggerPrimitiveDigi> RPCTriggerPrimitivesBuilder::clusterPhase2(
    const std::vector<RPCDigiPhase2>& digis) const {
  struct Hit {
    int bx;  // BX in which the strips are grouped: from the time or from the digi
    int strip;
    int subbx;  // time in sub-BX since BX 0, sub-BX 0
  };

  std::vector<Hit> hits;
  hits.reserve(digis.size());
  for (auto const& digi : digis) {
    const float time = RPCDigiPhase2Time(digi).time();
    hits.push_back(
        {config_.bxFromTime ? bxOfTime(time) : digi.bx(), digi.strip(), kSubBXPerBX * digi.bx() + digi.sbx()});
  }

  // Hits sorted by (bx, strip, sub-BX), keeping only the earliest hit of each strip
  std::sort(hits.begin(), hits.end(), [](const Hit& lhs, const Hit& rhs) {
    return std::make_tuple(lhs.bx, lhs.strip, lhs.subbx) < std::make_tuple(rhs.bx, rhs.strip, rhs.subbx);
  });
  hits.erase(std::unique(hits.begin(),
                         hits.end(),
                         [](const Hit& lhs, const Hit& rhs) { return lhs.bx == rhs.bx && lhs.strip == rhs.strip; }),
             hits.end());

  std::vector<RPCTriggerPrimitiveDigi> clusters;
  auto it = hits.begin();
  while (it != hits.end()) {
    const int bx = it->bx;
    const int stripLow = it->strip;
    int stripHi = stripLow;
    int minSubBX = it->subbx, maxSubBX = it->subbx, sumSubBX = 0, n = 0;
    auto inWindow = [&](const Hit& hit) {
      return config_.maxSubBXSpreadRPC < 0 ||
             std::max(maxSubBX, hit.subbx) - std::min(minSubBX, hit.subbx) <= config_.maxSubBXSpreadRPC;
    };
    // Extend the cluster while the next strip in the same BX is adjacent and within the sub-BX window
    do {
      stripHi = it->strip;
      minSubBX = std::min(minSubBX, it->subbx);
      maxSubBX = std::max(maxSubBX, it->subbx);
      sumSubBX += it->subbx;
      ++n;
    } while (++it != hits.end() && it->bx == bx && it->strip == stripHi + 1 && inWindow(*it));

    // Mean time, the same as the mean of RPCDigiPhase2Time
    const float time = 25.f / kSubBXPerBX * sumSubBX / n;
    RPCTriggerPrimitiveDigi tp(stripLow, stripHi, config_.bxFromTime ? bxOfTime(time) : bx);
    tp.setTime(time);
    clusters.push_back(tp);
  }

  std::stable_sort(clusters.begin(), clusters.end());  // by BX, then first strip
  return clusters;
}

std::vector<RPCTriggerPrimitiveDigi> RPCTriggerPrimitivesBuilder::clusterIRPC(const std::vector<IRPCDigi>& digis) const {
  struct Hit {
    int bx;  // BX in which the strips are grouped: from the time or from the digi (LR end)
    int strip;
    float time;
    float y;
  };

  std::vector<Hit> hits;
  hits.reserve(digis.size());
  for (auto const& digi : digis) {
    IRPCDigiTime digiTime(digi);
    const float time = digiTime.time();
    hits.push_back({config_.bxFromTime ? bxOfTime(time) : digi.bx(), digi.strip(), time, digiTime.coordinateY()});
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

    const float time = sumTime / n;
    RPCTriggerPrimitiveDigi tp(stripLow, stripHi, config_.bxFromTime ? bxOfTime(time) : bx);
    tp.setTime(time);
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
