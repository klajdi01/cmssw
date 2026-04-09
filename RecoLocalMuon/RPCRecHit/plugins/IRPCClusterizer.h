#ifndef RecoLocalMuon_RPCRecHit_IRPCClusterizer_h
#define RecoLocalMuon_RPCRecHit_IRPCClusterizer_h

/*
 * Clusterizer for IRPCDigi.
 *
 * \author J. Shin -- Kyung Hee University
 */

#include "DataFormats/RPCDigi/interface/IRPCDigiCollection.h"

#include "IRPCCluster.h"
#include "IRPCClusterContainer.h"

class IRPCClusterizer {
public:
  IRPCClusterizer() = default;
  ~IRPCClusterizer() = default;

  IRPCClusterContainer doAction(const IRPCDigiCollection::Range& digiRange) const;

private:
  IRPCClusterContainer makeInitialClusters(const IRPCDigiCollection::Range& digiRange) const;
};

#endif