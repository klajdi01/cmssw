/*
 * See header file for a description of this class.
 *
 * \author J. Shin -- Kyung Hee University
 */

#include "Geometry/RPCGeometry/interface/RPCRoll.h"

#include "RPCRecHitPhase2Algo.h"

namespace {
  inline float clusterCenterX(const RPCRoll& roll, int firstStrip, int lastStrip) {
    const float x1 = roll.centreOfStrip(firstStrip).x();
    const float x2 = roll.centreOfStrip(lastStrip).x();
    return 0.5f * (x1 + x2);
  }

  inline float clusterMiddleStrip(int firstStrip, int lastStrip) {
    return 0.5f * (static_cast<float>(firstStrip) + static_cast<float>(lastStrip));
  }
}  // namespace

bool RPCRecHitPhase2Algo::compute(const RPCRoll& roll,
                                  const RPCClusterPhase2& cluster,
                                  LocalPoint& point,
                                  LocalError& error,
                                  float& time,
                                  float& timeErr) const {
  const float x = clusterCenterX(roll, cluster.firstStrip(), cluster.lastStrip());

  point = LocalPoint(x, 0.f, 0.f);
  error = roll.localError(clusterMiddleStrip(cluster.firstStrip(), cluster.lastStrip()));

  if (cluster.hasTime()) {
    time = cluster.time();
    timeErr = cluster.timeRMS();
  } else {
    time = 0.f;
    timeErr = -1.f;
  }

  return true;
}

bool RPCRecHitPhase2Algo::compute(const RPCRoll& roll,
                                  const IRPCCluster& cluster,
                                  LocalPoint& point,
                                  LocalError& error,
                                  float& time,
                                  float& timeErr) const {
  const float x = clusterCenterX(roll, cluster.firstStrip(), cluster.lastStrip());

  point = LocalPoint(x, 0.f, 0.f);
  error = roll.localError(clusterMiddleStrip(cluster.firstStrip(), cluster.lastStrip()));

  if (cluster.hasTime()) {
    time = cluster.time();
    timeErr = cluster.timeRMS();
  } else {
    time = 0.f;
    timeErr = -1.f;
  }

  return true;
}

RPCRecHit RPCRecHitPhase2Algo::build(const RPCRoll& roll, const RPCDetId& rpcId, const RPCClusterPhase2& cluster) const {
  LocalPoint point;
  LocalError error;
  float time = 0.f;
  float timeErr = -1.f;

  compute(roll, cluster, point, error, time, timeErr);

  RPCRecHit recHit(rpcId, cluster.bx(), cluster.firstStrip(), cluster.clusterSize(), point, error);
  recHit.setTimeAndError(time, timeErr);

  return recHit;
}

RPCRecHit RPCRecHitPhase2Algo::build(const RPCRoll& roll, const RPCDetId& rpcId, const IRPCCluster& cluster) const {
  LocalPoint point;
  LocalError error;
  float time = 0.f;
  float timeErr = -1.f;

  compute(roll, cluster, point, error, time, timeErr);

  RPCRecHit recHit(rpcId, cluster.bx(), cluster.firstStrip(), cluster.clusterSize(), point, error);
  recHit.setTimeAndError(time, timeErr);

  return recHit;
}