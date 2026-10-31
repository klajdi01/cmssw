#ifndef DataFormats_RPCDigi_RPCDigiPhase2Time_h
#define DataFormats_RPCDigi_RPCDigiPhase2Time_h

#include "DataFormats/RPCDigi/interface/RPCDigiPhase2.h"

class RPCDigiPhase2Time {
public:
  RPCDigiPhase2Time(const RPCDigiPhase2& adigi);
  float time();

private:
  RPCDigiPhase2 theDigi;
};

#endif
