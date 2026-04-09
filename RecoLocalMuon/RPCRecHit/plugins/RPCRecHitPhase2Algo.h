#ifndef RecoLocalMuon_RPCRecHit_RPCRecHitPhase2Algo_h
#define RecoLocalMuon_RPCRecHit_RPCRecHitPhase2Algo_h

/*
 * RecHit algorithm for Phase-2 RPC reconstruction.
 *
 * Output type is the standard RPCRecHit.
 *
 * \author J. Shin -- Kyung Hee University
 */

#include "DataFormats/GeometrySurface/interface/LocalError.h"
#include "DataFormats/GeometryVector/interface/LocalPoint.h"
#include "DataFormats/RPCRecHit/interface/RPCRecHit.h"

#include "IRPCCluster.h"
#include "RPCClusterPhase2.h"

class RPCRoll;
class RPCDetId;

class RPCRecHitPhase2Algo {
public:
  RPCRecHitPhase2Algo() = default;
  ~RPCRecHitPhase2Algo() = default;

  bool compute(const RPCRoll& roll,
               const RPCClusterPhase2& cluster,
               LocalPoint& point,
               LocalError& error,
               float& time,
               float& timeErr) const;

  bool compute(
      const RPCRoll& roll, const IRPCCluster& cluster, LocalPoint& point, LocalError& error, float& time, float& timeErr) const;

  RPCRecHit build(const RPCRoll& roll, const RPCDetId& rpcId, const RPCClusterPhase2& cluster) const;
  RPCRecHit build(const RPCRoll& roll, const RPCDetId& rpcId, const IRPCCluster& cluster) const;
};

#endif