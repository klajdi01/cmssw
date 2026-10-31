# Checks the iRPC trigger primitives built from the Phase-2 iRPC digis (IRPCDigi):
#   simRpcTriggerPrimitiveDigis : RPC trigger primitives, iRPC from simMuonIRPCDigis (phase2_rpc_devel)
#   rpcRecHitsPhase2            : Phase-2 RPC RecHits made from the same digis
# rpcTriggerPrimitivesIRPCValidator compares the two (same strips, BX, time and local y are expected)
# and compares the trigger primitive y with the simulated muon hits.
#
# Input: a Phase-2 GEN-SIM-DIGI file made with --era Phase2C22I13M9,phase2_rpc_devel and
# --eventcontent FEVTDEBUG (it must contain simMuonRPCDigis, simMuonIRPCDigis and g4SimHits), e.g.
#   cmsRun runRPCTriggerPrimitivesIRPC_cfg.py inputFiles=file:step1.root

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
from Configuration.Eras.Era_Phase2C22I13M9_cff import Phase2C22I13M9
from Configuration.Eras.Modifier_phase2_rpc_devel_cff import phase2_rpc_devel

options = VarParsing('analysis')
options.register('outFile', 'rpcTriggerPrimitivesIRPC.root', VarParsing.multiplicity.singleton,
                 VarParsing.varType.string, 'Output file name')
options.parseArguments()

process = cms.Process('RPCTPIRPC', Phase2C22I13M9, phase2_rpc_devel)

process.load('Configuration.StandardSequences.Services_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.Geometry.GeometryExtendedRun4D125Reco_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase2_realistic_T35', '')

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))
process.source = cms.Source('PoolSource', fileNames = cms.untracked.vstring(options.inputFiles))
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.load('L1Trigger.RPCTriggerPrimitives.simRpcTriggerPrimitiveDigis_cfi')
process.load('RecoLocalMuon.RPCRecHit.rpcRecHitsPhase2_cfi')
process.rpcTriggerPrimitivesIRPCValidator = cms.EDAnalyzer('RPCTriggerPrimitivesIRPCValidator')

process.p = cms.Path(process.simRpcTriggerPrimitiveDigis + process.rpcRecHitsPhase2 +
                     process.rpcTriggerPrimitivesIRPCValidator)

process.out = cms.OutputModule('PoolOutputModule',
    fileName = cms.untracked.string(options.outFile),
    outputCommands = cms.untracked.vstring(
        'drop *',
        'keep *_simRpcTriggerPrimitiveDigis_*_*',
        'keep *_rpcRecHitsPhase2_*_*',
    ),
)
process.e = cms.EndPath(process.out)
