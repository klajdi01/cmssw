#include "L1Trigger/RPCTriggerPrimitives/interface/RPCTriggerPrimitivesBuilder.h"

#include "catch2/catch_all.hpp"

#include <vector>

using TP = RPCTriggerPrimitiveDigi;

namespace {
  // region, ring, station, sector, layer, subsector, roll
  const RPCDetId kRE12(1, 2, 1, 1, 1, 1, 1);
  const RPCDetId kRE31(-1, 1, 3, 1, 1, 1, 1);  // iRPC
  const RPCDetId kRB1(0, 0, 1, 1, 1, 1, 1);    // barrel

  std::vector<RPCTriggerPrimitiveDigi> clustersOf(const RPCTriggerPrimitiveDigiCollection& coll, const RPCDetId& id) {
    auto range = coll.get(id);
    return std::vector<RPCTriggerPrimitiveDigi>(range.first, range.second);
  }
}  // namespace

TEST_CASE("RPCTriggerPrimitivesBuilder::cluster", "[RPCTriggerPrimitives]") {
  SECTION("empty input") { REQUIRE(RPCTriggerPrimitivesBuilder::cluster({}).empty()); }

  SECTION("single strip") {
    auto out = RPCTriggerPrimitivesBuilder::cluster({RPCDigi(5, 0)});
    REQUIRE(out == std::vector<TP>{TP(5, 5, 0)});
    REQUIRE(out[0].strip() == 5);
    REQUIRE(out[0].clusterSize() == 1);
  }

  SECTION("adjacent strips are merged regardless of input order") {
    auto out = RPCTriggerPrimitivesBuilder::cluster({RPCDigi(7, 0), RPCDigi(5, 0), RPCDigi(6, 0)});
    REQUIRE(out == std::vector<TP>{TP(5, 7, 0)});
    REQUIRE(out[0].strip() == 6);
    REQUIRE(out[0].clusterSize() == 3);
  }

  SECTION("central strip of even-sized cluster rounds down") {
    auto out = RPCTriggerPrimitivesBuilder::cluster({RPCDigi(3, 0), RPCDigi(4, 0)});
    REQUIRE(out[0].strip() == 3);
  }

  SECTION("gap splits clusters") {
    auto out = RPCTriggerPrimitivesBuilder::cluster({RPCDigi(1, 0), RPCDigi(2, 0), RPCDigi(4, 0)});
    REQUIRE(out == std::vector<TP>{TP(1, 2, 0), TP(4, 4, 0)});
  }

  SECTION("duplicates are removed") {
    auto out = RPCTriggerPrimitivesBuilder::cluster({RPCDigi(3, 0), RPCDigi(3, 0), RPCDigi(4, 0)});
    REQUIRE(out == std::vector<TP>{TP(3, 4, 0)});
  }

  SECTION("different BXs are clustered separately and ordered by BX") {
    auto out = RPCTriggerPrimitivesBuilder::cluster({RPCDigi(3, 1), RPCDigi(4, 0), RPCDigi(3, 0), RPCDigi(4, -1)});
    REQUIRE(out == std::vector<TP>{TP(4, 4, -1), TP(3, 4, 0), TP(3, 3, 1)});
  }
}

TEST_CASE("RPCTriggerPrimitivesBuilder::build", "[RPCTriggerPrimitives]") {
  RPCDigiCollection digis;
  for (int strip : {10, 11, 12, 13, 14, 20})
    digis.insertDigi(kRE12, RPCDigi(strip, 0));
  for (int strip : {1, 2, 3, 4, 5, 6, 7})
    digis.insertDigi(kRE31, RPCDigi(strip, 0));
  digis.insertDigi(kRB1, RPCDigi(8, 0));

  SECTION("default: no cluster size cut, barrel skipped") {
    RPCTriggerPrimitivesBuilder builder(RPCTriggerPrimitivesBuilder::Config{});
    RPCTriggerPrimitiveDigiCollection out;
    builder.build(digis, out);
    REQUIRE(clustersOf(out, kRE12) == std::vector<TP>{TP(10, 14, 0), TP(20, 20, 0)});
    REQUIRE(clustersOf(out, kRE31) == std::vector<TP>{TP(1, 7, 0)});
    REQUIRE(clustersOf(out, kRB1).empty());
  }

  SECTION("cluster size cuts are applied separately for RPC and iRPC") {
    RPCTriggerPrimitivesBuilder::Config config;
    config.maxClusterSizeRPC = 4;
    config.maxClusterSizeIRPC = 6;
    RPCTriggerPrimitivesBuilder builder(config);
    RPCTriggerPrimitiveDigiCollection out;
    builder.build(digis, out);
    REQUIRE(clustersOf(out, kRE12) == std::vector<TP>{TP(20, 20, 0)});
    REQUIRE(clustersOf(out, kRE31).empty());
  }

  SECTION("barrel can be enabled") {
    RPCTriggerPrimitivesBuilder::Config config;
    config.includeBarrel = true;
    RPCTriggerPrimitivesBuilder builder(config);
    RPCTriggerPrimitiveDigiCollection out;
    builder.build(digis, out);
    REQUIRE(clustersOf(out, kRB1) == std::vector<TP>{TP(8, 8, 0)});
  }
}
