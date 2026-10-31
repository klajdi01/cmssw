/*
 * See header file for a description of this class.
 *
 * \author J. Shin -- Kyung Hee University
 */

#include "Geometry/RPCGeometry/interface/RPCRoll.h"
#include "Geometry/CommonTopologies/interface/TrapezoidalStripTopology.h"

#include "RPCRecHitPhase2Algo.h"

#include <algorithm>
#include <cmath>

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
  const double y = cluster.hasY() ? cluster.y() : 0.;

  point = LocalPoint(x, y, 0.f);

  if (!cluster.hasY()) {
    error = roll.localError(clusterMiddleStrip(cluster.firstStrip(), cluster.lastStrip()));
  } else {
    // Estimate the local-y variance from the distance to the strip boundaries.
    // Keep the measured y in the point, but do not let a quantized/smeared
    // time difference produce a negative boundary distance.
    float ex2 = roll.localError(clusterMiddleStrip(cluster.firstStrip(), cluster.lastStrip())).xx();
    const float stripLength = roll.specificTopology().stripLength();
    const float maxDy = std::max(0.f, stripLength / 2.f - std::abs(static_cast<float>(y)));
    const double ey2 = static_cast<double>(maxDy) * maxDy / 3.;

    // For endcap trapezoids, the measured y changes the x position and local pitch.
    if (roll.id().region() != 0) {
      const auto& topology = dynamic_cast<const TrapezoidalStripTopology&>(roll.topology());
      const double angle = topology.stripAngle(clusterMiddleStrip(cluster.firstStrip(), cluster.lastStrip()));
      const double tanAngle = std::tan(angle);
      point = LocalPoint(x - y * tanAngle, y, 0.f);

      const double scale = topology.localPitch(point) / topology.pitch();
      ex2 *= scale * scale;

      // Propagate x' = x - y*tan(angle) to the local (x', y) covariance.
      error = LocalError(ex2 + tanAngle * tanAngle * ey2, -tanAngle * ey2, ey2);
    } else {
      error = LocalError(ex2, 0., ey2);
    }
  }

  if (cluster.hasTime()) {
    time = cluster.time();
    timeErr = cluster.timeRMS();
  } else {
    time = 0.f;
    timeErr = -1.f;
  }

  return true;
}

RPCRecHit RPCRecHitPhase2Algo::build(const RPCRoll& roll,
                                     const RPCDetId& rpcId,
                                     const RPCClusterPhase2& cluster) const {
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
