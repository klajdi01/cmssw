# Runs the Phase-1 EMTF with RPC digis and with RPC trigger primitives; the outputs are expected to be identical.
# Input: Run-3 GEN-SIM-DIGI-RAW, e.g. cmsRun runRPCTriggerPrimitivesEMTF_cfg.py inputFiles=file:step2.root maxEvents=100

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
from Configuration.Eras.Era_Run3_2025_cff import Run3_2025

options = VarParsing('analysis')
options.register('outFile', 'rpcTriggerPrimitivesEMTF.root', VarParsing.multiplicity.singleton,
                 VarParsing.varType.string, 'Output file name')
options.parseArguments()

process = cms.Process('RPCTP', Run3_2025)

process.load('Configuration.StandardSequences.Services_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase1_2025_realistic', '')

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))
process.source = cms.Source('PoolSource', fileNames = cms.untracked.vstring(options.inputFiles))
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.load('L1Trigger.RPCTriggerPrimitives.simRpcTriggerPrimitiveDigis_cfi')

process.load('L1Trigger.L1TMuonEndCap.simEmtfDigis_cfi')
process.simEmtfDigisRPCTP = process.simEmtfDigis.clone(RPCTPEnable = True)

process.p = cms.Path(process.simRpcTriggerPrimitiveDigis + process.simEmtfDigis + process.simEmtfDigisRPCTP)

process.out = cms.OutputModule('PoolOutputModule',
    fileName = cms.untracked.string(options.outFile),
    outputCommands = cms.untracked.vstring(
        'drop *',
        'keep *_simRpcTriggerPrimitiveDigis_*_*',
        'keep *_simEmtfDigis_*_*',
        'keep *_simEmtfDigisRPCTP_*_*',
    ),
)
process.e = cms.EndPath(process.out)
