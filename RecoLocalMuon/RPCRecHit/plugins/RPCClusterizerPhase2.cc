/*
 * See header file for a description of this class.
 *
 * \author J. Shin -- Kyung Hee University
 */

#include "DataFormats/RPCDigi/interface/RPCDigiPhase2Time.h"

#include "RPCClusterizerPhase2.h"

#include <iterator>

RPCClusterPhase2Container RPCClusterizerPhase2::doAction(const RPCDigiPhase2Collection::Range& digiRange) const {
  return makeInitialClusters(digiRange);
}

RPCClusterPhase2Container RPCClusterizerPhase2::makeInitialClusters(
    const RPCDigiPhase2Collection::Range& digiRange) const {
  RPCClusterPhase2Container initialClusters;
  RPCClusterPhase2Container finalClusters;

  if (std::distance(digiRange.first, digiRange.second) == 0) {
    return finalClusters;
  }

  for (auto digi = digiRange.first; digi != digiRange.second; ++digi) {
    RPCClusterPhase2 cl(digi->strip(), digi->strip(), digi->bx());

    RPCDigiPhase2Time digiTime(*digi);
    cl.addTime(digiTime.time());

    initialClusters.insert(cl);
  }

  if (initialClusters.empty()) {
    return finalClusters;
  }

  RPCClusterPhase2 prev = *initialClusters.begin();
  for (auto cl = std::next(initialClusters.begin()); cl != initialClusters.end(); ++cl) {
    if (prev.isAdjacent(*cl)) {
      prev.merge(*cl);
    } else {
      finalClusters.insert(prev);
      prev = *cl;
    }
  }
  finalClusters.insert(prev);

  return finalClusters;
}