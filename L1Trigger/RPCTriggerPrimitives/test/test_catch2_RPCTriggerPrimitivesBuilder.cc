#include "L1Trigger/RPCTriggerPrimitives/interface/RPCTriggerPrimitivesBuilder.h"
#include "DataFormats/RPCDigi/interface/IRPCDigiTime.h"

#include "catch2/catch_all.hpp"

#include <cmath>
#include <vector>

using TP = RPCTriggerPrimitiveDigi;

namespace {
  // region, ring, station, sector, layer, subsector, roll
  const RPCDetId kRE12(1, 2, 1, 1, 1, 1, 1);
  const RPCDetId kRE31(-1, 1, 3, 1, 1, 1, 1);  // iRPC
  const RPCDetId kRB1(0, 0, 1, 1, 1, 1, 1);    // barrel

  // iRPC digi in BX 0 with the times at both strip ends given as (sub-BX of 2.5 ns, fine time of 0.2 ns)
  IRPCDigi irpcDigi(int strip, int sbxLR, int fineLR, int sbxHR, int fineHR) {
    return IRPCDigi(strip, 0, 0, sbxLR, sbxHR, fineLR, fineHR);
  }

  float timeOf(const IRPCDigi& digi) { return IRPCDigiTime(digi).time(); }
  float yOf(const IRPCDigi& digi) { return IRPCDigiTime(digi).coordinateY(); }

  RPCDigi pseudoDigi(int strip, int bx, double time, double y) {
    RPCDigi digi(strip, bx);
    digi.hasTime(true);
    digi.setTime(time);
    digi.hasY(true);
    digi.setY(y);
    return digi;
  }

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

TEST_CASE("RPCTriggerPrimitivesBuilder::cluster with time and y", "[RPCTriggerPrimitives]") {
  SECTION("plain digis give no time and no y") {
    auto out = RPCTriggerPrimitivesBuilder::cluster({RPCDigi(5, 0), RPCDigi(6, 0)});
    REQUIRE(out.size() == 1);
    REQUIRE_FALSE(out[0].hasTime());
    REQUIRE_FALSE(out[0].hasY());
  }

  SECTION("time and y are averaged over the strips of the cluster") {
    auto out = RPCTriggerPrimitivesBuilder::cluster({pseudoDigi(5, 0, 1.0, 10.0), pseudoDigi(6, 0, 2.0, 20.0)});
    REQUIRE(out == std::vector<TP>{TP(5, 6, 0)});
    REQUIRE(out[0].hasTime());
    REQUIRE(out[0].time() == Catch::Approx(1.5));
    REQUIRE(out[0].hasY());
    REQUIRE(out[0].y() == Catch::Approx(15.0));
  }
}

TEST_CASE("RPCTriggerPrimitivesBuilder::clusterIRPC", "[RPCTriggerPrimitives]") {
  RPCTriggerPrimitivesBuilder builder(RPCTriggerPrimitivesBuilder::Config{});

  SECTION("empty input") { REQUIRE(builder.clusterIRPC({}).empty()); }

  SECTION("single strip: time and y from the two strip ends") {
    // t_LR = 2.5 ns, t_HR = 1.0 ns
    const IRPCDigi digi = irpcDigi(5, 1, 0, 0, 5);
    auto out = builder.clusterIRPC({digi});
    REQUIRE(out == std::vector<TP>{TP(5, 5, 0)});
    REQUIRE(out[0].hasTime());
    REQUIRE(out[0].time() == timeOf(digi));
    REQUIRE(out[0].time() == Catch::Approx(1.75));
    REQUIRE(out[0].hasY());
    REQUIRE(out[0].y() == yOf(digi));
    REQUIRE(out[0].y() > 0.f);  // t_LR > t_HR gives a positive y
  }

  SECTION("adjacent strips with close time and y are merged, with mean time and y") {
    const IRPCDigi digi5 = irpcDigi(5, 0, 0, 0, 0);  // t = 0, y = 0
    const IRPCDigi digi6 = irpcDigi(6, 0, 2, 0, 1);  // t_LR = 0.4 ns, t_HR = 0.2 ns
    auto out = builder.clusterIRPC({digi5, digi6});
    REQUIRE(out == std::vector<TP>{TP(5, 6, 0)});
    REQUIRE(out[0].time() == Catch::Approx((timeOf(digi5) + timeOf(digi6)) / 2));
    REQUIRE(out[0].y() == Catch::Approx((yOf(digi5) + yOf(digi6)) / 2));
  }

  SECTION("adjacent strips with times more than 3 ns apart are split") {
    const IRPCDigi digi5 = irpcDigi(5, 0, 0, 0, 0);  // t = 0
    const IRPCDigi digi6 = irpcDigi(6, 1, 9, 1, 8);  // t_LR = 4.3 ns, t_HR = 4.1 ns, so t = 4.2 ns and y < 9 cm
    REQUIRE(std::abs(yOf(digi6)) < 9.f);
    auto out = builder.clusterIRPC({digi5, digi6});
    REQUIRE(out == std::vector<TP>{TP(5, 5, 0), TP(6, 6, 0)});
  }

  SECTION("adjacent strips with y more than 9 cm apart are split") {
    const IRPCDigi digi5 = irpcDigi(5, 0, 0, 0, 0);   // t = 0, y = 0
    const IRPCDigi digi6 = irpcDigi(6, 0, 10, 0, 0);  // t_LR = 2.0 ns, t_HR = 0, so t = 1 ns and y > 9 cm
    REQUIRE(yOf(digi6) > 9.f);
    auto out = builder.clusterIRPC({digi5, digi6});
    REQUIRE(out == std::vector<TP>{TP(5, 5, 0), TP(6, 6, 0)});
  }

  SECTION("the time and y windows are configurable") {
    RPCTriggerPrimitivesBuilder::Config config;
    config.maxTimeSpreadIRPC = 5.f;
    config.maxYSpreadIRPC = 30.f;
    RPCTriggerPrimitivesBuilder wideBuilder(config);
    auto out = wideBuilder.clusterIRPC(
        {irpcDigi(5, 0, 0, 0, 0), irpcDigi(6, 1, 9, 1, 8), irpcDigi(7, 0, 10, 0, 0)});
    REQUIRE(out == std::vector<TP>{TP(5, 7, 0)});
  }

  SECTION("gap and BX split clusters, output ordered by BX and first strip") {
    auto out = builder.clusterIRPC({IRPCDigi(9, 1, 1, 0, 0, 0, 0),
                                    irpcDigi(2, 0, 0, 0, 0),
                                    irpcDigi(1, 0, 0, 0, 0),
                                    irpcDigi(4, 0, 0, 0, 0)});
    REQUIRE(out == std::vector<TP>{TP(1, 2, 0), TP(4, 4, 0), TP(9, 9, 1)});
  }
}

TEST_CASE("RPCTriggerPrimitivesBuilder::build with iRPC digis", "[RPCTriggerPrimitives]") {
  RPCDigiCollection digis;
  for (int strip : {10, 11, 12})
    digis.insertDigi(kRE12, RPCDigi(strip, 0));
  for (int strip : {1, 2, 3, 4, 5, 6, 7})
    digis.insertDigi(kRE31, RPCDigi(strip, 0));

  IRPCDigiCollection irpcDigis;
  irpcDigis.insertDigi(kRE31, irpcDigi(20, 1, 0, 0, 5));
  irpcDigis.insertDigi(kRE31, irpcDigi(21, 1, 0, 0, 5));
  irpcDigis.insertDigi(kRE12, irpcDigi(30, 0, 0, 0, 0));  // not an iRPC chamber: ignored

  RPCTriggerPrimitivesBuilder builder(RPCTriggerPrimitivesBuilder::Config{});
  RPCTriggerPrimitiveDigiCollection out;
  builder.build(digis, &irpcDigis, out);

  SECTION("RPC chambers still come from the RPC digis") {
    REQUIRE(clustersOf(out, kRE12) == std::vector<TP>{TP(10, 12, 0)});
    REQUIRE_FALSE(clustersOf(out, kRE12)[0].hasTime());
  }

  SECTION("iRPC chambers come only from the iRPC digis, with time and y") {
    auto irpc = clustersOf(out, kRE31);
    REQUIRE(irpc == std::vector<TP>{TP(20, 21, 0)});
    REQUIRE(irpc[0].hasTime());
    REQUIRE(irpc[0].time() == Catch::Approx(1.75));
    REQUIRE(irpc[0].hasY());
    REQUIRE(irpc[0].y() == Catch::Approx(yOf(irpcDigi(20, 1, 0, 0, 5))));
  }
}
