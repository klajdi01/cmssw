#ifndef RecoLocalMuon_RPCRecHit_RPCClusterPhase2_h
#define RecoLocalMuon_RPCRecHit_RPCClusterPhase2_h

/*
 * Phase-2 RPC cluster object
 *
 * \author J. Shin -- Kyung Hee University
 */

#include <cstdint>

class RPCClusterPhase2 {
public:
  RPCClusterPhase2();
  RPCClusterPhase2(int firstStrip, int lastStrip, int bx);
  ~RPCClusterPhase2();

  int firstStrip() const;
  int lastStrip() const;
  int clusterSize() const;
  int bx() const;

  bool hasTime() const;
  float time() const;
  float timeRMS() const;

  bool hasY() const;
  float y() const;
  float yRMS() const;

  void addTime(float time);
  void addY(float y);
  void merge(const RPCClusterPhase2& other);

  bool operator<(const RPCClusterPhase2& other) const;
  bool operator==(const RPCClusterPhase2& other) const;
  bool isAdjacent(const RPCClusterPhase2& other) const;

private:
  uint16_t fstrip_;
  uint16_t lstrip_;
  int16_t bx_;

  float sumTime_;
  float sumTime2_;
  uint16_t nTime_;

  float sumY_;
  float sumY2_;
  uint16_t nY_;
};

#endif