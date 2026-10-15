"""Limited teaching example, not a full NR TBS or DM-RS implementation."""
import math


def resources(mu, prbs, symbols, dmrs_per_prb, overhead=0, layers=1, qm=2):
    if not (isinstance(mu, int) and 0 <= mu <= 4):
        raise ValueError("this normal-CP teaching exercise uses integer mu 0..4")
    if prbs <= 0 or not 1 <= symbols <= 14 or layers <= 0 or qm <= 0:
        raise ValueError("invalid teaching parameters")
    if dmrs_per_prb < 0 or overhead < 0:
        raise ValueError("negative overhead")
    per_prb = 12 * symbols - dmrs_per_prb - overhead
    if per_prb <= 0:
        raise ValueError("no data RE")
    return {
        "scs_khz": 15 * 2**mu,
        "slot_ms": 1 / 2**mu,
        "slots_per_frame": 10 * 2**mu,
        "raw_re": prbs * 12 * symbols,
        "example_mapping_re": prbs * per_prb,
        "tbs_count_re": prbs * min(156, per_prb),
        "example_mapping_bits_no_uci_ptrs": prbs * per_prb * layers * qm,
    }


def base_graph(a, rate):
    if a <= 0 or not 0 < rate <= 1:
        raise ValueError("invalid payload/rate")
    return 2 if a <= 292 or (a <= 3824 and rate <= 0.67) or rate <= 0.25 else 1


def main():
    values = resources(1, 12, 10, 12)
    for key, value in values.items():
        print(f"{key}={value}")
    # Qm=2, R=0.5 are mathematical inputs, not a claimed MCS-index combination.
    n_info = values["tbs_count_re"] * 2 * 0.5
    n = max(3, math.floor(math.log2(n_info)) - 6)
    quantized = max(24, 2**n * math.floor(n_info / 2**n))
    # Only this tiny verified table fragment is included. Reject other ranges.
    if not 1288 < quantized <= 1320:
        raise ValueError("example outside supplied table fragment; use full specification")
    a = 1320
    crc = 24 if a > 3824 else 16
    print(f"N_info={n_info}, N_info_prime={quantized}, example_TBS={a}")
    print(f"TB_CRC_bits={crc}, B={a + crc}, BG={base_graph(a, 0.5)}")
    print("LIMIT: one slot, one layer, no UCI/PT-RS/hopping/transform precoding; not full NR.")
    assert values["slot_ms"] == 0.5
    assert values["example_mapping_bits_no_uci_ptrs"] == 2592
    assert n_info == 1296 and quantized == 1296
    assert a + crc == 1336 and base_graph(a, 0.5) == 2


if __name__ == "__main__":
    main()
