#!/usr/bin/env python3
import sys

# --- EDIT THESE IF YOU WANT ---
INPUT_FILE  = "calibration_output.dat"   # your .dat from calibration
OUTPUT_FILE = "dssd_treevars.txt"        # where to write the treevariable lines

# constants, matching your example
DEFAULT_THR          = 0
DEFAULT_TIME_OFFSET  = 0

def side_to_name(side):
    """
    side = 0 -> SiFront
    side = 1 -> SiBack
    """
    if side == 0:
        return "SiFront"
    elif side == 1:
        return "SiBack"
    else:
        raise ValueError(f"Unexpected side value {side} (expected 0 or 1).")


def main(input_file, output_file):
    lines_out = []

    with open(input_file, "r") as f:
        for lineno, line in enumerate(f, start=1):
            line = line.strip()
            if not line:
                continue  # skip empty lines
            if line.startswith("#"):
                continue  # skip comment lines if any

            parts = line.split()
            if len(parts) < 6:
                raise ValueError(
                    f"Line {lineno}: expected 6 columns "
                    f"(side strip gain offset gain_err offset_err), got {len(parts)}.\n"
                    f"Line content: {line}"
                )

            side      = int(parts[0])
            strip     = int(parts[1])
            gain      = float(parts[2])
            offset    = float(parts[3])
            # gain_err  = float(parts[4])   # not used here
            # off_err   = float(parts[5])   # not used here

            # convert side to SiFront / SiBack
            side_name = side_to_name(side)

            # convert strip from 1-based (calibration) to 0-based (treevariable)
            strip_index = strip - 1
            if strip_index < 0:
                raise ValueError(
                    f"Line {lineno}: strip index became negative after -1: strip={strip}"
                )

            prefix = f"treevariable -set DDASV.{side_name}{strip_index}"

            lines_out.append(f"{prefix}.Thr {DEFAULT_THR} {{}}")
            lines_out.append(f"{prefix}.offset {offset} {{}}")
            lines_out.append(f"{prefix}.slope {gain} {{}}")
            lines_out.append(f"{prefix}.time_offset {DEFAULT_TIME_OFFSET} {{}}")

    # write output file
    with open(output_file, "w") as out:
        out.write("\n".join(lines_out) + "\n")

    print(f"Written {len(lines_out)} lines to {output_file}")


if __name__ == "__main__":
    # allow optional command-line arguments:
    #   python make_dssd_treevars.py calib.dat out.txt
    if len(sys.argv) >= 2:
        in_file = sys.argv[1]
    else:
        in_file = INPUT_FILE

    if len(sys.argv) >= 3:
        out_file = sys.argv[2]
    else:
        out_file = OUTPUT_FILE

    main(in_file, out_file)
