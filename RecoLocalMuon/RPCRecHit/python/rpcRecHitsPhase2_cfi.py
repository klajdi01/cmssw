import FWCore.ParameterSet.Config as cms

rpcRecHitsPhase2 = cms.EDProducer(
    "RPCRecHitPhase2Producer",
    rpcDigiPhase2Label=cms.InputTag("simMuonRPCDigisPhase2"),
    irpcDigiLabel=cms.InputTag("simMuonIRPCDigis"),
    useIRPC=cms.bool(True),
)