#ifndef RecoLocalMuon_RPCRecHit_IRPCCluster_h
#define RecoLocalMuon_RPCRecHit_IRPCCluster_h

/*
 * IRPC cluster object.
 *
 * \author J. Shin -- Kyung Hee University
 */

#include <cstdint>

class IRPCCluster {
public:
  IRPCCluster();
  IRPCCluster(int firstStrip, int lastStrip, int bx);
  ~IRPCCluster();

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
  void merge(const IRPCCluster& other);

  bool operator<(const IRPCCluster& other) const;
  bool operator==(const IRPCCluster& other) const;
  bool isAdjacent(const IRPCCluster& other) const;

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