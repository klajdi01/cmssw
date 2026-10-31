/** \class RPCTriggerPrimitivesIRPCValidator
 *
 * Test module for the iRPC trigger primitives built from IRPCDigi:
 *  - compares them with the Phase-2 RPC RecHits (rpcRecHitsPhase2) made from the same digis.
 *    Both use the same clustering, so every iRPC trigger primitive should have a RecHit with
 *    the same strips and BX, and the same time and local y;
 *  - compares their local y with the simulated muon hit (resolution of the along-strip position).
 * A summary is printed at the end of the job.
 */

#include "DataFormats/MuonDetId/interface/RPCDetId.h"
#include "DataFormats/RPCDigi/interface/RPCTriggerPrimitiveDigiCollection.h"
#include "DataFormats/RPCRecHit/interface/RPCRecHitCollection.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/RPCGeometry/interface/RPCRoll.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "L1Trigger/RPCTriggerPrimitives/interface/RPCTriggerPrimitivesBuilder.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>

class RPCTriggerPrimitivesIRPCValidator : public edm::one::EDAnalyzer<> {
public:
  explicit RPCTriggerPrimitivesIRPCValidator(const edm::ParameterSet&);

  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  // Mean and RMS of a quantity
  struct Stats {
    long n = 0;
    double sum = 0.;
    double sum2 = 0.;
    void fill(double x) {
      ++n;
      sum += x;
      sum2 += x * x;
    }
    double mean() const { return n > 0 ? sum / n : 0.; }
    double rms() const { return n > 0 ? std::sqrt(std::max(0., sum2 / n - mean() * mean())) : 0.; }
  };

  const edm::EDGetTokenT<RPCTriggerPrimitiveDigiCollection> tpToken_;
  const edm::EDGetTokenT<RPCRecHitCollection> recHitToken_;
  edm::EDGetTokenT<edm::PSimHitContainer> simHitToken_;
  const edm::ESGetToken<RPCGeometry, MuonGeometryRecord> geomToken_;
  const double tolerance_;

  long nEvents_ = 0;
  long nRPC_ = 0;              // standard RPC trigger primitives
  long nIRPC_ = 0;             // iRPC trigger primitives
  long nIRPCTimeAndY_ = 0;     // ... with a time and a local y
  long nIRPCRecHits_ = 0;      // iRPC RecHits
  long nMatched_ = 0;          // iRPC trigger primitives with a RecHit of the same strips and BX
  long nSame_ = 0;             // ... and the same time and local y
  double maxDiffTime_ = 0.;
  double maxDiffY_ = 0.;
  Stats time_, y_, residualY_;
};

RPCTriggerPrimitivesIRPCValidator::RPCTriggerPrimitivesIRPCValidator(const edm::ParameterSet& ps)
    : tpToken_(consumes(ps.getParameter<edm::InputTag>("triggerPrimitives"))),
      recHitToken_(consumes(ps.getParameter<edm::InputTag>("recHits"))),
      geomToken_(esConsumes()),
      tolerance_(ps.getParameter<double>("tolerance")) {
  const auto& simHitTag = ps.getParameter<edm::InputTag>("simHits");
  if (!simHitTag.label().empty())
    simHitToken_ = consumes<edm::PSimHitContainer>(simHitTag);
}

void RPCTriggerPrimitivesIRPCValidator::analyze(const edm::Event& event, const edm::EventSetup& setup) {
  ++nEvents_;
  const auto& tps = event.get(tpToken_);
  const auto& recHits = event.get(recHitToken_);
  const auto& geometry = setup.getData(geomToken_);

  edm::Handle<edm::PSimHitContainer> simHits;
  if (!simHitToken_.isUninitialized())
    event.getByToken(simHitToken_, simHits);

  for (auto const& recHit : recHits) {
    if (RPCTriggerPrimitivesBuilder::isIRPC(recHit.rpcId()))
      ++nIRPCRecHits_;
  }

  for (auto const& [detId, range] : tps) {
    if (!RPCTriggerPrimitivesBuilder::isIRPC(detId)) {
      nRPC_ += std::distance(range.first, range.second);
      continue;
    }

    const auto rollRecHits = recHits.get(detId);
    const RPCRoll* roll = geometry.roll(detId);

    for (auto tp = range.first; tp != range.second; ++tp) {
      ++nIRPC_;
      if (!tp->hasTime() || !tp->hasY())
        continue;
      ++nIRPCTimeAndY_;
      time_.fill(tp->time());
      y_.fill(tp->y());

      // RecHit with the same strips and BX
      auto recHit = std::find_if(rollRecHits.first, rollRecHits.second, [&](const RPCRecHit& hit) {
        return hit.BunchX() == tp->bx() && hit.firstClusterStrip() == tp->stripLow() &&
               hit.clusterSize() == tp->clusterSize();
      });
      if (recHit != rollRecHits.second) {
        ++nMatched_;
        const double diffTime = std::abs(recHit->time() - tp->time());
        const double diffY = std::abs(recHit->localPosition().y() - tp->y());
        maxDiffTime_ = std::max(maxDiffTime_, diffTime);
        maxDiffY_ = std::max(maxDiffY_, diffY);
        if (diffTime <= tolerance_ && diffY <= tolerance_)
          ++nSame_;
      }

      // Simulated muon hit on the cluster strips (or next to them)
      if (simHits.isValid() && roll != nullptr) {
        for (auto const& simHit : *simHits) {
          if (simHit.detUnitId() != detId.rawId() || std::abs(simHit.particleType()) != 13)
            continue;
          const int strip = static_cast<int>(roll->strip(simHit.localPosition())) + 1;
          if (strip < tp->stripLow() - 1 || strip > tp->stripHi() + 1)
            continue;
          residualY_.fill(tp->y() - simHit.localPosition().y());
          break;
        }
      }
    }
  }
}

void RPCTriggerPrimitivesIRPCValidator::endJob() {
  const bool identical = nIRPCTimeAndY_ > 0 && nIRPCTimeAndY_ == nIRPC_ && nMatched_ == nIRPC_ && nSame_ == nIRPC_ &&
                         nIRPCRecHits_ == nIRPC_;

  std::cout << "\n===== RPC trigger primitives: iRPC check =====\n"
            << "events                           : " << nEvents_ << "\n"
            << "RPC trigger primitives           : " << nRPC_ << "\n"
            << "iRPC trigger primitives          : " << nIRPC_ << "\n"
            << "  with time and y                : " << nIRPCTimeAndY_ << "\n"
            << "iRPC RecHits (rpcRecHitsPhase2)  : " << nIRPCRecHits_ << "\n"
            << "  same strips and BX as a TP     : " << nMatched_ << "\n"
            << "  and same time and y            : " << nSame_ << "\n"
            << "largest time difference (ns)     : " << maxDiffTime_ << "\n"
            << "largest y difference (cm)        : " << maxDiffY_ << "\n"
            << "iRPC TP time (ns)   mean, RMS    : " << time_.mean() << ", " << time_.rms() << "\n"
            << "iRPC TP y (cm)      mean, RMS    : " << y_.mean() << ", " << y_.rms() << "\n"
            << "y - sim muon y (cm) mean, RMS    : " << residualY_.mean() << ", " << residualY_.rms() << "  ("
            << residualY_.n << " hits)\n"
            << "RESULT: " << (identical ? "IDENTICAL" : "DIFFERENT") << std::endl;
}

void RPCTriggerPrimitivesIRPCValidator::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("triggerPrimitives", edm::InputTag("simRpcTriggerPrimitiveDigis"));
  desc.add<edm::InputTag>("recHits", edm::InputTag("rpcRecHitsPhase2"));
  desc.add<edm::InputTag>("simHits", edm::InputTag("g4SimHits", "MuonRPCHits"))
      ->setComment("Simulated hits for the y resolution; empty label to skip");
  desc.add<double>("tolerance", 1e-4)->setComment("Largest time (ns) and y (cm) difference counted as the same");
  descriptions.add("rpcTriggerPrimitivesIRPCValidator", desc);
}

DEFINE_FWK_MODULE(RPCTriggerPrimitivesIRPCValidator);
