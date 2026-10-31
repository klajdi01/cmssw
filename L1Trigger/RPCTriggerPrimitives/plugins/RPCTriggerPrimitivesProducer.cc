/** \class RPCTriggerPrimitivesProducer
 *
 * Produces RPC trigger primitives (clusters of adjacent strips) from RPC
 * digis, for use by the L1 muon track finders. Currently used by the
 * Phase-1 EMTF emulator (RPCTPEnable in simEmtfDigis_cfi.py).
 */

#include "DataFormats/RPCDigi/interface/RPCDigiCollection.h"
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
  const edm::EDPutTokenT<RPCTriggerPrimitiveDigiCollection> putToken_;
  const RPCTriggerPrimitivesBuilder builder_;
};

RPCTriggerPrimitivesProducer::RPCTriggerPrimitivesProducer(const edm::ParameterSet& ps)
    : digiToken_(consumes(ps.getParameter<edm::InputTag>("RPCDigiProducer"))),
      putToken_(produces()),
      builder_(makeConfig(ps)) {}

RPCTriggerPrimitivesBuilder::Config RPCTriggerPrimitivesProducer::makeConfig(const edm::ParameterSet& ps) {
  RPCTriggerPrimitivesBuilder::Config config;
  config.includeBarrel = ps.getParameter<bool>("includeBarrel");
  config.maxClusterSizeRPC = ps.getParameter<int>("maxClusterSizeRPC");
  config.maxClusterSizeIRPC = ps.getParameter<int>("maxClusterSizeIRPC");
  return config;
}

void RPCTriggerPrimitivesProducer::produce(edm::StreamID, edm::Event& iEvent, const edm::EventSetup&) const {
  RPCTriggerPrimitiveDigiCollection out;
  builder_.build(iEvent.get(digiToken_), out);
  iEvent.emplace(putToken_, std::move(out));
}

void RPCTriggerPrimitivesProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("RPCDigiProducer", edm::InputTag("simMuonRPCDigis"));
  desc.add<bool>("includeBarrel", false)->setComment("Barrel TPs are not yet used by any track finder");
  // The Phase-1 EMTF applies its cluster size cut only after keeping the first two clusters per
  // chamber (as in firmware), so the cuts here are disabled by default to keep its output unchanged
  desc.add<int>("maxClusterSizeRPC", -1)->setComment("Maximum cluster size for RPC, negative = no cut");
  desc.add<int>("maxClusterSizeIRPC", -1)
      ->setComment("Maximum cluster size for iRPC (RE3/1, RE4/1), negative = no cut");
  descriptions.add("rpcTriggerPrimitiveDigis", desc);
}

DEFINE_FWK_MODULE(RPCTriggerPrimitivesProducer);
