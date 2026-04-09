/*
 * See header file for a description of this class.
 *
 * \author J. Shin -- Kyung Hee University
 */

#include "DataFormats/RPCDigi/interface/IRPCDigiTime.h"

#include "IRPCClusterizer.h"

#include <iterator>

IRPCClusterContainer IRPCClusterizer::doAction(const IRPCDigiCollection::Range& digiRange) const {
  return makeInitialClusters(digiRange);
}

IRPCClusterContainer IRPCClusterizer::makeInitialClusters(const IRPCDigiCollection::Range& digiRange) const {
  IRPCClusterContainer initialClusters;
  IRPCClusterContainer finalClusters;

  if (std::distance(digiRange.first, digiRange.second) == 0) {
    return finalClusters;
  }

  for (auto digi = digiRange.first; digi != digiRange.second; ++digi) {
    IRPCCluster cl(digi->strip(), digi->strip(), digi->bx());

    IRPCDigiTime digiTime(*digi);
    cl.addTime(digiTime.time());

    initialClusters.insert(cl);
  }

  if (initialClusters.empty()) {
    return finalClusters;
  }

  IRPCCluster prev = *initialClusters.begin();
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