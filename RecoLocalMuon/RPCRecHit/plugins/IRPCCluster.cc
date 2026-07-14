/*
 * See header file for a description of this class.
 *
 * \author J. Shin -- Kyung Hee University
 */

#include "IRPCCluster.h"

#include <algorithm>
#include <cmath>

namespace {
  constexpr float kMaxClusterYDistance = 9.f;       // cm
  constexpr float kMaxClusterTimeDifference = 3.f;  // ns
}

IRPCCluster::IRPCCluster()
    : fstrip_(0),
      lstrip_(0),
      bx_(0),
      sumTime_(0.f),
      sumTime2_(0.f),
      minTime_(0.f),
      maxTime_(0.f),
      nTime_(0),
      sumY_(0.f),
      sumY2_(0.f),
      minY_(0.f),
      maxY_(0.f),
      nY_(0) {}

IRPCCluster::IRPCCluster(int firstStrip, int lastStrip, int bx)
    : fstrip_(firstStrip),
      lstrip_(lastStrip),
      bx_(bx),
      sumTime_(0.f),
      sumTime2_(0.f),
      minTime_(0.f),
      maxTime_(0.f),
      nTime_(0),
      sumY_(0.f),
      sumY2_(0.f),
      minY_(0.f),
      maxY_(0.f),
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
  if ((other.firstStrip() != this->firstStrip() && other.firstStrip() != this->firstStrip() - 1) ||
      (other.bx() != this->bx())) {
    return false;
  }

  if (hasTime() && other.hasTime() &&
      std::max(maxTime_, other.maxTime_) - std::min(minTime_, other.minTime_) > kMaxClusterTimeDifference) {
    return false;
  }

  if (hasY() && other.hasY() && std::max(maxY_, other.maxY_) - std::min(minY_, other.minY_) > kMaxClusterYDistance) {
    return false;
  }

  return true;
}

void IRPCCluster::addTime(float time) {
  if (!hasTime()) {
    minTime_ = time;
    maxTime_ = time;
  } else {
    minTime_ = std::min(minTime_, time);
    maxTime_ = std::max(maxTime_, time);
  }

  ++nTime_;
  sumTime_ += time;
  sumTime2_ += time * time;
}

void IRPCCluster::addY(float y) {
  if (!hasY()) {
    minY_ = y;
    maxY_ = y;
  } else {
    minY_ = std::min(minY_, y);
    maxY_ = std::max(maxY_, y);
  }

  ++nY_;
  sumY_ += y;
  sumY2_ += y * y;
}

void IRPCCluster::merge(const IRPCCluster& other) {
  if (!this->isAdjacent(other)) {
    return;
  }

  const bool hadTime = hasTime();
  const bool hadY = hasY();
  fstrip_ = other.firstStrip();

  nTime_ += other.nTime_;
  sumTime_ += other.sumTime_;
  sumTime2_ += other.sumTime2_;
  if (other.hasTime()) {
    minTime_ = hadTime ? std::min(minTime_, other.minTime_) : other.minTime_;
    maxTime_ = hadTime ? std::max(maxTime_, other.maxTime_) : other.maxTime_;
  }

  nY_ += other.nY_;
  sumY_ += other.sumY_;
  sumY2_ += other.sumY2_;
  if (other.hasY()) {
    minY_ = hadY ? std::min(minY_, other.minY_) : other.minY_;
    maxY_ = hadY ? std::max(maxY_, other.maxY_) : other.maxY_;
  }
}

bool IRPCCluster::operator<(const IRPCCluster& other) const {
  if (bx_ != other.bx_) {
    return bx_ > other.bx_;
  }
  if (fstrip_ != other.fstrip_) {
    return fstrip_ > other.fstrip_;
  }
  if (lstrip_ != other.lstrip_) {
    return lstrip_ > other.lstrip_;
  }
  if (nTime_ != other.nTime_) {
    return nTime_ < other.nTime_;
  }
  if (minTime_ != other.minTime_) {
    return minTime_ < other.minTime_;
  }
  if (maxTime_ != other.maxTime_) {
    return maxTime_ < other.maxTime_;
  }
  if (nY_ != other.nY_) {
    return nY_ < other.nY_;
  }
  if (minY_ != other.minY_) {
    return minY_ < other.minY_;
  }
  return maxY_ < other.maxY_;
}

bool IRPCCluster::operator==(const IRPCCluster& other) const {
  return (this->clusterSize() == other.clusterSize()) && (this->bx() == other.bx()) &&
         (this->firstStrip() == other.firstStrip());
}
