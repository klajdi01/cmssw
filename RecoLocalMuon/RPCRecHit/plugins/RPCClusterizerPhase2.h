#ifndef RecoLocalMuon_RPCRecHit_RPCClusterizerPhase2_h
#define RecoLocalMuon_RPCRecHit_RPCClusterizerPhase2_h

/*
 * Clusterizer for RPCDigiPhase2.
 *
 * \author J. Shin -- Kyung Hee University
 */

#include "DataFormats/RPCDigi/interface/RPCDigiPhase2Collection.h"

#include "RPCClusterPhase2.h"
#include "RPCClusterPhase2Container.h"

class RPCClusterizerPhase2 {
public:
  RPCClusterizerPhase2() = default;
  ~RPCClusterizerPhase2() = default;

  RPCClusterPhase2Container doAction(const RPCDigiPhase2Collection::Range& digiRange) const;

private:
  RPCClusterPhase2Container makeInitialClusters(const RPCDigiPhase2Collection::Range& digiRange) const;
};

#endif