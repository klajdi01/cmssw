/** Produces RPC trigger primitives (clusters of adjacent strips) for the L1 muon track finders.
 *  With phase2_rpc_devel, iRPC and the other chambers are built from IRPCDigi and RPCDigiPhase2. */

#include "DataFormats/RPCDigi/interface/IRPCDigiCollection.h"
#include "DataFormats/RPCDigi/interface/RPCDigiCollection.h"
#include "DataFormats/RPCDigi/interface/RPCDigiPhase2Collection.h"
#include "DataFormats/RPCDigi/interface/RPCTriggerPrimitiveDigiCollection.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/EDPutToken.h"
#include "L1Trigger/RPCTriggerPrimitives/interface/RPCTriggerPrimitivesBuilder.h"

class RPCTriggerPrimitivesProducer : public edm::global::EDProducer<> {
public:
  explicit RPCTriggerPrimitivesProducer(const edm::ParameterSet&);

  void produce(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  static RPCTriggerPrimitivesBuilder::Config makeConfig(const edm::ParameterSet&);

  const edm::EDGetTokenT<RPCDigiCollection> digiToken_;
  edm::EDGetTokenT<RPCDigiPhase2Collection> rpcDigiPhase2Token_;
  edm::EDGetTokenT<IRPCDigiCollection> irpcDigiToken_;
  const edm::EDPutTokenT<RPCTriggerPrimitiveDigiCollection> putToken_;
  const RPCTriggerPrimitivesBuilder builder_;
};

RPCTriggerPrimitivesProducer::RPCTriggerPrimitivesProducer(const edm::ParameterSet& ps)
    : digiToken_(consumes(ps.getParameter<edm::InputTag>("RPCDigiProducer"))),
      putToken_(produces()),
      builder_(makeConfig(ps)) {
  const auto& rpcPhase2Tag = ps.getParameter<edm::InputTag>("RPCDigiPhase2Producer");
  if (!rpcPhase2Tag.label().empty())
    rpcDigiPhase2Token_ = consumes<RPCDigiPhase2Collection>(rpcPhase2Tag);
  const auto& irpcTag = ps.getParameter<edm::InputTag>("IRPCDigiProducer");
  if (!irpcTag.label().empty())
    irpcDigiToken_ = consumes<IRPCDigiCollection>(irpcTag);
}

RPCTriggerPrimitivesBuilder::Config RPCTriggerPrimitivesProducer::makeConfig(const edm::ParameterSet& ps) {
  RPCTriggerPrimitivesBuilder::Config config;
  config.includeBarrel = ps.getParameter<bool>("includeBarrel");
  config.maxClusterSizeRPC = ps.getParameter<int>("maxClusterSizeRPC");
  config.maxClusterSizeIRPC = ps.getParameter<int>("maxClusterSizeIRPC");
  config.maxTimeSpreadIRPC = ps.getParameter<double>("maxTimeSpreadIRPC");
  config.maxYSpreadIRPC = ps.getParameter<double>("maxYSpreadIRPC");
  config.maxSubBXSpreadRPC = ps.getParameter<int>("maxSubBXSpreadRPC");
  config.bxFromTime = ps.getParameter<bool>("bxFromTime");
  return config;
}

void RPCTriggerPrimitivesProducer::produce(edm::StreamID, edm::Event& iEvent, const edm::EventSetup&) const {
  RPCTriggerPrimitiveDigiCollection out;
  const IRPCDigiCollection* irpcDigis = irpcDigiToken_.isUninitialized() ? nullptr : &iEvent.get(irpcDigiToken_);
  const RPCDigiPhase2Collection* rpcDigisPhase2 =
      rpcDigiPhase2Token_.isUninitialized() ? nullptr : &iEvent.get(rpcDigiPhase2Token_);
  builder_.build(iEvent.get(digiToken_), rpcDigisPhase2, irpcDigis, out);
  iEvent.emplace(putToken_, std::move(out));
}

void RPCTriggerPrimitivesProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("RPCDigiProducer", edm::InputTag("simMuonRPCDigis"));
  desc.add<edm::InputTag>("RPCDigiPhase2Producer", edm::InputTag(""))
      ->setComment("Phase-2 RPC digis with sub-BX timing (simMuonRPCDigisPhase2); if empty, RPC uses RPCDigiProducer");
  desc.add<edm::InputTag>("IRPCDigiProducer", edm::InputTag(""))
      ->setComment("iRPC digis with two-end timing (simMuonIRPCDigis); if empty, iRPC uses RPCDigiProducer");
  desc.add<bool>("includeBarrel", false)->setComment("Barrel TPs are not yet used by any track finder");
  // The Phase-1 EMTF applies its cluster size cut only after keeping the first two clusters per
  // chamber (as in firmware), so the cuts here are disabled by default to keep its output unchanged
  desc.add<int>("maxClusterSizeRPC", -1)->setComment("Maximum cluster size for RPC, negative = no cut");
  desc.add<int>("maxClusterSizeIRPC", -1)
      ->setComment("Maximum cluster size for iRPC (RE3/1, RE4/1), negative = no cut");
  // Same values as IRPCCluster in RecoLocalMuon/RPCRecHit
  desc.add<double>("maxTimeSpreadIRPC", 3.)->setComment("Maximum spread of the strip times in one iRPC cluster (ns)");
  desc.add<double>("maxYSpreadIRPC", 9.)->setComment("Maximum spread of the strip local y in one iRPC cluster (cm)");
  desc.add<int>("maxSubBXSpreadRPC", 2)
      ->setComment(
          "Maximum spread of the strip sub-BX (1.5625 ns) in one RPC cluster built from RPCDigiPhase2, negative = no "
          "window");
  desc.add<bool>("bxFromTime", false)
      ->setComment(
          "BX of the clusters built from RPCDigiPhase2 and IRPCDigi = round(time / 25 ns), as in the Phase-2 EMTF; if "
          "false, the BX of the digis");
  descriptions.add("rpcTriggerPrimitiveDigis", desc);
}

DEFINE_FWK_MODULE(RPCTriggerPrimitivesProducer);
