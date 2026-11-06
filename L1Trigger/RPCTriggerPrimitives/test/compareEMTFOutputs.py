#!/usr/bin/env python3
# Compares event by event the two EMTF outputs of runRPCTriggerPrimitivesEMTF_cfg.py
# Usage: python3 compareEMTFOutputs.py rpcTriggerPrimitivesEMTF.root

import argparse
import ROOT
from DataFormats.FWLite import Events, Handle

HIT_FIELDS = ("Subsystem", "Endcap", "Sector", "Station", "Ring", "Chamber", "Roll", "Neighbor", "Strip",
              "Strip_low", "Strip_hi", "BX", "Phi_fp", "Theta_fp", "Quality", "Valid")
TRACK_FIELDS = ("Endcap", "Sector", "BX", "Mode", "Mode_RPC", "Pt", "Charge", "Phi_fp", "Theta_fp", "GMT_pt",
                "GMT_phi", "GMT_eta", "GMT_quality", "NumHits")
RPC = 2  # L1TMuon::kRPC


def summarize(objects, fields):
    return [tuple(getattr(o, f)() for f in fields) for o in objects]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("file")
    parser.add_argument("--process", default="RPCTP")
    parser.add_argument("--old", default="simEmtfDigis")
    parser.add_argument("--new", default="simEmtfDigisRPCTP")
    parser.add_argument("--show", type=int, default=5, help="number of differing events to print")
    args = parser.parse_args()

    hitType = "std::vector<l1t::io_v1::EMTFHit>"
    trackType = "std::vector<l1t::io_v1::EMTFTrack>"
    handles = {
        (label, kind): Handle(t)
        for label in (args.old, args.new)
        for kind, t in (("hits", hitType), ("tracks", trackType))
    }

    nEvents = nSame = nShown = 0
    totals = {key: 0 for key in ("old hits", "new hits", "old RPC hits", "new RPC hits", "old tracks", "new tracks")}

    for event in Events(args.file):
        data = {}
        for (label, kind), handle in handles.items():
            event.getByLabel(label, "", args.process, handle)
            fields = HIT_FIELDS if kind == "hits" else TRACK_FIELDS
            data[(label, kind)] = summarize(handle.product(), fields)

        oldHits, newHits = data[(args.old, "hits")], data[(args.new, "hits")]
        oldTracks, newTracks = data[(args.old, "tracks")], data[(args.new, "tracks")]
        totals["old hits"] += len(oldHits)
        totals["new hits"] += len(newHits)
        totals["old RPC hits"] += sum(h[0] == RPC for h in oldHits)
        totals["new RPC hits"] += sum(h[0] == RPC for h in newHits)
        totals["old tracks"] += len(oldTracks)
        totals["new tracks"] += len(newTracks)

        nEvents += 1
        if oldHits == newHits and oldTracks == newTracks:
            nSame += 1
        elif nShown < args.show:
            nShown += 1
            aux = event.eventAuxiliary()
            print(f"--- Difference in run {aux.run()} event {aux.event()}")
            for name, fields, old, new in (("hits", HIT_FIELDS, oldHits, newHits),
                                          ("tracks", TRACK_FIELDS, oldTracks, newTracks)):
                if old != new:
                    print(f"  {name}: {len(old)} old, {len(new)} new  fields={fields}")
                    for o in sorted(set(old) - set(new)):
                        print(f"    only old: {o}")
                    for n in sorted(set(new) - set(old)):
                        print(f"    only new: {n}")
                    if sorted(old) == sorted(new):
                        print("    (same content, different order)")

    print("=" * 60)
    for key, value in totals.items():
        print(f"{key:>14}: {value}")
    print(f"Events identical: {nSame} / {nEvents}")
    print("RESULT: IDENTICAL" if nSame == nEvents and nEvents > 0 else "RESULT: DIFFERENT")


if __name__ == "__main__":
    ROOT.gROOT.SetBatch(True)
    main()
