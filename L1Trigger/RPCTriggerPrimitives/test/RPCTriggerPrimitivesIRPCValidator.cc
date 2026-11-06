/** Test module: compares the Phase-2 RPC and iRPC trigger primitives with rpcRecHitsPhase2 and the iRPC y
 *  with the simulated muon, and counts them per BX and sub-BX. Prints a summary at the end of the job. */

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
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <string>

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

  // Counters for one kind of chamber (endcap RPC or iRPC)
  struct Counts {
    long nTPs = 0;          // trigger primitives
    long nWithTime = 0;     // ... with a time (and a local y for iRPC)
    long nBXFromTime = 0;   // ... with BX == round(time / 25 ns)
    long nRecHits = 0;      // RecHits
    long nMatched = 0;      // trigger primitives with a RecHit of the same strips and BX
    long nSame = 0;         // ... and the same time (and local y for iRPC)
    double maxDiffTime = 0.;
    double maxDiffY = 0.;
    Stats time, y;
    std::map<int, long> bx, subbx;

    bool identical() const {
      return nWithTime > 0 && nWithTime == nTPs && nMatched == nTPs && nSame == nTPs && nRecHits == nTPs;
    }
  };

  void print(const std::string& name, const Counts& counts, bool withY) const;

  const edm::InputTag tpTag_;
  const edm::EDGetTokenT<RPCTriggerPrimitiveDigiCollection> tpToken_;
  edm::EDGetTokenT<RPCRecHitCollection> recHitToken_;
  edm::EDGetTokenT<edm::PSimHitContainer> simHitToken_;
  const edm::ESGetToken<RPCGeometry, MuonGeometryRecord> geomToken_;
  const bool compareWithRecHits_;
  const double tolerance_;

  long nEvents_ = 0;
  Counts rpc_, irpc_;
  Stats residualY_;
};

RPCTriggerPrimitivesIRPCValidator::RPCTriggerPrimitivesIRPCValidator(const edm::ParameterSet& ps)
    : tpTag_(ps.getParameter<edm::InputTag>("triggerPrimitives")),
      tpToken_(consumes(tpTag_)),
      geomToken_(esConsumes()),
      compareWithRecHits_(ps.getParameter<bool>("compareWithRecHits")),
      tolerance_(ps.getParameter<double>("tolerance")) {
  if (compareWithRecHits_)
    recHitToken_ = consumes<RPCRecHitCollection>(ps.getParameter<edm::InputTag>("recHits"));
  const auto& simHitTag = ps.getParameter<edm::InputTag>("simHits");
  if (!simHitTag.label().empty())
    simHitToken_ = consumes<edm::PSimHitContainer>(simHitTag);
}

void RPCTriggerPrimitivesIRPCValidator::analyze(const edm::Event& event, const edm::EventSetup& setup) {
  ++nEvents_;
  const auto& tps = event.get(tpToken_);
  const auto& geometry = setup.getData(geomToken_);

  const RPCRecHitCollection* recHits = compareWithRecHits_ ? &event.get(recHitToken_) : nullptr;
  if (recHits != nullptr) {
    for (auto const& recHit : *recHits) {
      if (recHit.rpcId().region() == 0)
        continue;
      if (RPCTriggerPrimitivesBuilder::isIRPC(recHit.rpcId()))
        ++irpc_.nRecHits;
      else
        ++rpc_.nRecHits;
    }
  }

  edm::Handle<edm::PSimHitContainer> simHits;
  if (!simHitToken_.isUninitialized())
    event.getByToken(simHitToken_, simHits);

  for (auto const& [detId, range] : tps) {
    if (detId.region() == 0)
      continue;  // barrel: not used by the track finders
    const bool isIRPC = RPCTriggerPrimitivesBuilder::isIRPC(detId);
    Counts& counts = isIRPC ? irpc_ : rpc_;
    const RPCRoll* roll = geometry.roll(detId);

    for (auto tp = range.first; tp != range.second; ++tp) {
      ++counts.nTPs;
      ++counts.bx[tp->bx()];
      if (!tp->hasTime() || (isIRPC && !tp->hasY()))
        continue;
      ++counts.nWithTime;
      ++counts.subbx[tp->subbx()];
      if (tp->bx() == RPCTriggerPrimitivesBuilder::bxOfTime(tp->time()))
        ++counts.nBXFromTime;
      counts.time.fill(tp->time());
      if (isIRPC)
        counts.y.fill(tp->y());

      // RecHit with the same strips and BX
      if (recHits != nullptr) {
        const auto rollRecHits = recHits->get(detId);
        auto recHit = std::find_if(rollRecHits.first, rollRecHits.second, [&](const RPCRecHit& hit) {
          return hit.BunchX() == tp->bx() && hit.firstClusterStrip() == tp->stripLow() &&
                 hit.clusterSize() == tp->clusterSize();
        });
        if (recHit != rollRecHits.second) {
          ++counts.nMatched;
          const double diffTime = std::abs(recHit->time() - tp->time());
          const double diffY = isIRPC ? std::abs(recHit->localPosition().y() - tp->y()) : 0.;
          counts.maxDiffTime = std::max(counts.maxDiffTime, diffTime);
          counts.maxDiffY = std::max(counts.maxDiffY, diffY);
          if (diffTime <= tolerance_ && diffY <= tolerance_)
            ++counts.nSame;
        }
      }

      // Simulated muon hit on the iRPC cluster strips (or next to them)
      if (isIRPC && simHits.isValid() && roll != nullptr) {
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

void RPCTriggerPrimitivesIRPCValidator::print(const std::string& name, const Counts& counts, bool withY) const {
  std::cout << std::left << std::setw(34) << name + " trigger primitives" << ": " << counts.nTPs << "\n"
            << std::setw(34) << (withY ? "  with time and y" : "  with time") << ": " << counts.nWithTime << "\n"
            << "  with BX == round(time / 25 ns)  : " << counts.nBXFromTime << "\n"
            << "  per BX                          :";
  for (auto const& [bx, n] : counts.bx)
    std::cout << "  " << bx << ": " << n;
  std::cout << "\n  per sub-BX (1.5625 ns)          :";
  for (auto const& [subbx, n] : counts.subbx)
    std::cout << "  " << subbx << ": " << n;
  std::cout << "\n  time (ns) mean, RMS             : " << counts.time.mean() << ", " << counts.time.rms() << "\n";
  if (withY) {
    std::cout << "  y (cm) mean, RMS                : " << counts.y.mean() << ", " << counts.y.rms() << "\n";
    std::cout << "  y - sim muon y (cm) mean, RMS   : " << residualY_.mean() << ", " << residualY_.rms() << "  ("
              << residualY_.n << " hits)\n";
  }
  if (compareWithRecHits_) {
    std::cout << "  RecHits (rpcRecHitsPhase2)      : " << counts.nRecHits << "\n";
    std::cout << "    same strips and BX as a TP    : " << counts.nMatched << "\n";
    std::cout << std::setw(34) << (withY ? "    and same time and y" : "    and same time") << ": " << counts.nSame
              << "\n";
    std::cout << "  largest time difference (ns)    : " << counts.maxDiffTime << "\n";
    if (withY)
      std::cout << "  largest y difference (cm)       : " << counts.maxDiffY << "\n";
  }
  std::cout << std::right;
}

void RPCTriggerPrimitivesIRPCValidator::endJob() {
  std::cout << "\n===== RPC trigger primitives: Phase-2 RPC and iRPC check =====\n"
            << "trigger primitives                : " << tpTag_.encode() << "\n"
            << "events                           : " << nEvents_ << "\n";
  print("RPC", rpc_, false);
  print("iRPC", irpc_, true);
  if (compareWithRecHits_) {
    // The RPC trigger primitives are compared only if they were built from RPCDigiPhase2 (they have a time)
    const bool identical = irpc_.identical() && (rpc_.nWithTime == 0 || rpc_.identical());
    std::cout << "RESULT: " << (identical ? "IDENTICAL" : "DIFFERENT") << std::endl;
  } else {
    std::cout << "RESULT: counted only (not compared with the RecHits)" << std::endl;
  }
}

void RPCTriggerPrimitivesIRPCValidator::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("triggerPrimitives", edm::InputTag("simRpcTriggerPrimitiveDigis"));
  desc.add<bool>("compareWithRecHits", true)
      ->setComment("Compare with the RecHits (needs the same clustering); if false, only count");
  desc.add<edm::InputTag>("recHits", edm::InputTag("rpcRecHitsPhase2"));
  desc.add<edm::InputTag>("simHits", edm::InputTag("g4SimHits", "MuonRPCHits"))
      ->setComment("Simulated hits for the y resolution; empty label to skip");
  desc.add<double>("tolerance", 1e-4)->setComment("Largest time (ns) and y (cm) difference counted as the same");
  descriptions.add("rpcTriggerPrimitivesIRPCValidator", desc);
}

DEFINE_FWK_MODULE(RPCTriggerPrimitivesIRPCValidator);
