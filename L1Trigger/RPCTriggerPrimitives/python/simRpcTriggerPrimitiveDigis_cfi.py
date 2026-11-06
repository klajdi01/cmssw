import FWCore.ParameterSet.Config as cms

# RPC trigger primitives for the L1 muon track finders, built from the simulated RPC digis
from L1Trigger.RPCTriggerPrimitives.rpcTriggerPrimitiveDigis_cfi import rpcTriggerPrimitiveDigis as _rpcTriggerPrimitiveDigis
simRpcTriggerPrimitiveDigis = _rpcTriggerPrimitiveDigis.clone(RPCDigiProducer = 'simMuonRPCDigis')

# With the Phase-2 RPC digis, build the iRPC chambers from the two-end readout (time and local y)
# and the other chambers from the digis with sub-BX timing
from Configuration.Eras.Modifier_phase2_rpc_devel_cff import phase2_rpc_devel
phase2_rpc_devel.toModify(simRpcTriggerPrimitiveDigis,
                          RPCDigiPhase2Producer = 'simMuonRPCDigisPhase2',
                          IRPCDigiProducer = 'simMuonIRPCDigis')
