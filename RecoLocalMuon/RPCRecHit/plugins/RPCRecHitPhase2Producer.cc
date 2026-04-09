/*
 * See header file for a description of this class.
 *
 * \author J. Shin -- Kyung Hee University
 */

#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "Geometry/RPCGeometry/interface/RPCRoll.h"

#include "RPCRecHitPhase2Producer.h"

RPCRecHitPhase2Producer::RPCRecHitPhase2Producer(const edm::ParameterSet& config)
    : rpcDigiPhase2Token_(consumes<RPCDigiPhase2Collection>(config.getParameter<edm::InputTag>("rpcDigiPhase2Label"))),
      irpcDigiToken_(consumes<IRPCDigiCollection>(config.getParameter<edm::InputTag>("irpcDigiLabel"))),
      rpcGeomToken_(esConsumes<RPCGeometry, MuonGeometryRecord>()),
      useIRPC_(config.getParameter<bool>("useIRPC")) {
  produces<RPCRecHitCollection>();
}

bool RPCRecHitPhase2Producer::isIRPCDetId(const RPCDetId& rpcId) {
  return (rpcId.region() != 0) && (rpcId.ring() == 1) && (rpcId.station() == 3 || rpcId.station() == 4);
}

void RPCRecHitPhase2Producer::buildFromRPCPhase2(
    const RPCRoll& roll, const RPCDetId& rpcId, const RPCDigiPhase2Collection::Range& range, RPCRecHitCollection& output) const {
  const RPCClusterPhase2Container clusters = rpcClusterizer_.doAction(range);

  edm::OwnVector<RPCRecHit> hits;
  for (const auto& cluster : clusters) {
    hits.push_back(algo_.build(roll, rpcId, cluster));
  }

  if (!hits.empty()) {
    output.put(rpcId, hits.begin(), hits.end());
  }
}

void RPCRecHitPhase2Producer::buildFromIRPC(
    const RPCRoll& roll, const RPCDetId& rpcId, const IRPCDigiCollection::Range& range, RPCRecHitCollection& output) const {
  const IRPCClusterContainer clusters = irpcClusterizer_.doAction(range);

  edm::OwnVector<RPCRecHit> hits;
  for (const auto& cluster : clusters) {
    hits.push_back(algo_.build(roll, rpcId, cluster));
  }

  if (!hits.empty()) {
    output.put(rpcId, hits.begin(), hits.end());
  }
}

void RPCRecHitPhase2Producer::produce(edm::Event& event, const edm::EventSetup& setup) {
  const auto& rpcGeom = setup.getData(rpcGeomToken_);

  edm::Handle<RPCDigiPhase2Collection> rpcPhase2Digis;
  event.getByToken(rpcDigiPhase2Token_, rpcPhase2Digis);

  edm::Handle<IRPCDigiCollection> irpcDigis;
  if (useIRPC_) {
    event.getByToken(irpcDigiToken_, irpcDigis);
  }

  auto recHitCollection = std::make_unique<RPCRecHitCollection>();

  if (rpcPhase2Digis.isValid()) {
    for (auto rpcdgIt = rpcPhase2Digis->begin(); rpcdgIt != rpcPhase2Digis->end(); ++rpcdgIt) {
      const RPCDetId& rpcId = (*rpcdgIt).first;

      if (isIRPCDetId(rpcId)) {
        continue;
      }

      const RPCRoll* roll = rpcGeom.roll(rpcId);
      if (roll == nullptr) {
        edm::LogError("BadDigiInput") << "Failed to find RPCRoll for ID " << rpcId;
        continue;
      }

      const RPCDigiPhase2Collection::Range& range = (*rpcdgIt).second;
      buildFromRPCPhase2(*roll, rpcId, range, *recHitCollection);
    }
  } else {
    edm::LogWarning("MissingInput") << "RPCRecHitPhase2Producer: missing RPCDigiPhase2Collection";
  }

  if (useIRPC_) {
    if (irpcDigis.isValid()) {
      for (auto irpcIt = irpcDigis->begin(); irpcIt != irpcDigis->end(); ++irpcIt) {
        const RPCDetId& rpcId = (*irpcIt).first;

        if (!isIRPCDetId(rpcId)) {
          continue;
        }

        const RPCRoll* roll = rpcGeom.roll(rpcId);
        if (roll == nullptr) {
          edm::LogError("BadDigiInput") << "Failed to find RPCRoll for ID " << rpcId;
          continue;
        }

        const IRPCDigiCollection::Range& range = (*irpcIt).second;
        buildFromIRPC(*roll, rpcId, range, *recHitCollection);
      }
    } else {
      edm::LogWarning("MissingInput") << "RPCRecHitPhase2Producer: missing IRPCDigiCollection";
    }
  }

  event.put(std::move(recHitCollection));
}

void RPCRecHitPhase2Producer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("rpcDigiPhase2Label", edm::InputTag("simMuonRPCDigisPhase2"));
  desc.add<edm::InputTag>("irpcDigiLabel", edm::InputTag("simMuonIRPCDigis"));
  desc.add<bool>("useIRPC", true);
  descriptions.addWithDefaultLabel(desc);
}

DEFINE_FWK_MODULE(RPCRecHitPhase2Producer);