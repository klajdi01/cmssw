/*
 * See header file for a description of this class.
 *
 * \author J. Shin -- Kyung Hee University
 */

#include "RPCClusterPhase2.h"

#include <algorithm>
#include <cmath>

RPCClusterPhase2::RPCClusterPhase2()
    : fstrip_(0), lstrip_(0), bx_(0), sumTime_(0.f), sumTime2_(0.f), nTime_(0), sumY_(0.f), sumY2_(0.f), nY_(0) {}

RPCClusterPhase2::RPCClusterPhase2(int firstStrip, int lastStrip, int bx)
    : fstrip_(firstStrip),
      lstrip_(lastStrip),
      bx_(bx),
      sumTime_(0.f),
      sumTime2_(0.f),
      nTime_(0),
      sumY_(0.f),
      sumY2_(0.f),
      nY_(0) {}

RPCClusterPhase2::~RPCClusterPhase2() = default;

int RPCClusterPhase2::firstStrip() const { return fstrip_; }

int RPCClusterPhase2::lastStrip() const { return lstrip_; }

int RPCClusterPhase2::clusterSize() const { return lstrip_ - fstrip_ + 1; }

int RPCClusterPhase2::bx() const { return bx_; }

bool RPCClusterPhase2::hasTime() const { return nTime_ > 0; }

float RPCClusterPhase2::time() const { return hasTime() ? sumTime_ / nTime_ : 0.f; }

float RPCClusterPhase2::timeRMS() const {
  if (!hasTime()) {
    return -1.f;
  }
  return std::sqrt(std::max(0.f, sumTime2_ * nTime_ - sumTime_ * sumTime_)) / nTime_;
}

bool RPCClusterPhase2::hasY() const { return nY_ > 0; }

float RPCClusterPhase2::y() const { return hasY() ? sumY_ / nY_ : 0.f; }

float RPCClusterPhase2::yRMS() const {
  if (!hasY()) {
    return -1.f;
  }
  return std::sqrt(std::max(0.f, sumY2_ * nY_ - sumY_ * sumY_)) / nY_;
}

bool RPCClusterPhase2::isAdjacent(const RPCClusterPhase2& other) const {
  return (other.firstStrip() == this->firstStrip() - 1) && (other.bx() == this->bx());
}

void RPCClusterPhase2::addTime(float time) {
  ++nTime_;
  sumTime_ += time;
  sumTime2_ += time * time;
}

void RPCClusterPhase2::addY(float y) {
  ++nY_;
  sumY_ += y;
  sumY2_ += y * y;
}

void RPCClusterPhase2::merge(const RPCClusterPhase2& other) {
  if (!this->isAdjacent(other)) {
    return;
  }

  fstrip_ = other.firstStrip();

  nTime_ += other.nTime_;
  sumTime_ += other.sumTime_;
  sumTime2_ += other.sumTime2_;

  nY_ += other.nY_;
  sumY_ += other.sumY_;
  sumY2_ += other.sumY2_;
}

bool RPCClusterPhase2::operator<(const RPCClusterPhase2& other) const {
  if (other.bx() == this->bx()) {
    return other.firstStrip() < this->firstStrip();
  }
  return other.bx() < this->bx();
}

bool RPCClusterPhase2::operator==(const RPCClusterPhase2& other) const {
  return (this->clusterSize() == other.clusterSize()) && (this->bx() == other.bx()) &&
         (this->firstStrip() == other.firstStrip());
}