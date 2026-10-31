#ifndef DataFormats_RPCDigi_RPCTriggerPrimitiveDigiCollection_h
#define DataFormats_RPCDigi_RPCTriggerPrimitiveDigiCollection_h

#include "DataFormats/MuonDetId/interface/RPCDetId.h"
#include "DataFormats/RPCDigi/interface/RPCTriggerPrimitiveDigi.h"
#include "DataFormats/MuonData/interface/MuonDigiCollection.h"

typedef MuonDigiCollection<RPCDetId, RPCTriggerPrimitiveDigi> RPCTriggerPrimitiveDigiCollection;

#endif
