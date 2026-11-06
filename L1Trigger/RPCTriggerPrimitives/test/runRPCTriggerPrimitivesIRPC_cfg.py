# Compares the Phase-2 RPC trigger primitives with rpcRecHitsPhase2 (expected: RESULT: IDENTICAL).
# Input: Phase-2 FEVTDEBUG file made with phase2_rpc_devel, e.g. inputFiles=file:step1.root

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
process.simRpcTriggerPrimitiveDigisAsRecHits = process.simRpcTriggerPrimitiveDigis.clone(maxSubBXSpreadRPC = -1)
process.rpcTriggerPrimitivesIRPCValidator = cms.EDAnalyzer('RPCTriggerPrimitivesIRPCValidator',
    triggerPrimitives = cms.InputTag('simRpcTriggerPrimitiveDigisAsRecHits'),
)
process.rpcTriggerPrimitivesBXValidator = cms.EDAnalyzer('RPCTriggerPrimitivesIRPCValidator',
    triggerPrimitives = cms.InputTag('simRpcTriggerPrimitiveDigis'),
    compareWithRecHits = cms.bool(False),
)

process.p = cms.Path(process.simRpcTriggerPrimitiveDigis + process.simRpcTriggerPrimitiveDigisAsRecHits +
                     process.rpcRecHitsPhase2 +
                     process.rpcTriggerPrimitivesIRPCValidator + process.rpcTriggerPrimitivesBXValidator)

process.out = cms.OutputModule('PoolOutputModule',
    fileName = cms.untracked.string(options.outFile),
    outputCommands = cms.untracked.vstring(
        'drop *',
        'keep *_simRpcTriggerPrimitiveDigis_*_*',
        'keep *_rpcRecHitsPhase2_*_*',
    ),
)
process.e = cms.EndPath(process.out)
