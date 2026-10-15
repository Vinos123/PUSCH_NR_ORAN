"""Only headerless little-endian interleaved int16 I/Q. Not a project dump decoder."""
from pathlib import Path
import struct
import sys


def inspect(data):
    if not data or len(data) % 4:
        raise ValueError("expected nonempty complete 4-byte I/Q samples")
    count = len(data) // 4
    peak = 0
    saturated_components = 0
    energy = 0
    for i, q in struct.iter_unpack("<hh", data):
        peak = max(peak, abs(i), abs(q))
        saturated_components += (i in (-32768, 32767)) + (q in (-32768, 32767))
        energy += i * i + q * q
    return {"complex_samples": count, "component_peak": peak,
            "saturated_components": saturated_components,
            "mean_integer_iq_energy": energy / count}


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: python3 iq_inspect.py approved_iq.bin", file=sys.stderr)
        sys.exit(2)
    try:
        print(inspect(Path(sys.argv[1]).read_bytes()))
        print("No scaling/FFT/compression/NR decoding is inferred from this format.")
    except (ValueError, OSError) as error:
        print(error, file=sys.stderr)
        sys.exit(2)
