/*
 * See header file for a description of this class.
 *
 * \author J. Shin -- Kyung Hee University
 */

#include "IRPCCluster.h"

#include <algorithm>
#include <cmath>

IRPCCluster::IRPCCluster()
    : fstrip_(0), lstrip_(0), bx_(0), sumTime_(0.f), sumTime2_(0.f), nTime_(0), sumY_(0.f), sumY2_(0.f), nY_(0) {}

IRPCCluster::IRPCCluster(int firstStrip, int lastStrip, int bx)
    : fstrip_(firstStrip),
      lstrip_(lastStrip),
      bx_(bx),
      sumTime_(0.f),
      sumTime2_(0.f),
      nTime_(0),
      sumY_(0.f),
      sumY2_(0.f),
      nY_(0) {}

IRPCCluster::~IRPCCluster() = default;

int IRPCCluster::firstStrip() const { return fstrip_; }

int IRPCCluster::lastStrip() const { return lstrip_; }

int IRPCCluster::clusterSize() const { return lstrip_ - fstrip_ + 1; }

int IRPCCluster::bx() const { return bx_; }

bool IRPCCluster::hasTime() const { return nTime_ > 0; }

float IRPCCluster::time() const { return hasTime() ? sumTime_ / nTime_ : 0.f; }

float IRPCCluster::timeRMS() const {
  if (!hasTime()) {
    return -1.f;
  }
  return std::sqrt(std::max(0.f, sumTime2_ * nTime_ - sumTime_ * sumTime_)) / nTime_;
}

bool IRPCCluster::hasY() const { return nY_ > 0; }

float IRPCCluster::y() const { return hasY() ? sumY_ / nY_ : 0.f; }

float IRPCCluster::yRMS() const {
  if (!hasY()) {
    return -1.f;
  }
  return std::sqrt(std::max(0.f, sumY2_ * nY_ - sumY_ * sumY_)) / nY_;
}

bool IRPCCluster::isAdjacent(const IRPCCluster& other) const {
  return (other.firstStrip() == this->firstStrip() - 1) && (other.bx() == this->bx());
}

void IRPCCluster::addTime(float time) {
  ++nTime_;
  sumTime_ += time;
  sumTime2_ += time * time;
}

void IRPCCluster::addY(float y) {
  ++nY_;
  sumY_ += y;
  sumY2_ += y * y;
}

void IRPCCluster::merge(const IRPCCluster& other) {
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

bool IRPCCluster::operator<(const IRPCCluster& other) const {
  if (other.bx() == this->bx()) {
    return other.firstStrip() < this->firstStrip();
  }
  return other.bx() < this->bx();
}

bool IRPCCluster::operator==(const IRPCCluster& other) const {
  return (this->clusterSize() == other.clusterSize()) && (this->bx() == other.bx()) &&
         (this->firstStrip() == other.firstStrip());
}