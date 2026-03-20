"""
ANT Neuro EDX: standard API mode transition test.

Documents actual behavior of the standard BrainFlow API for mode switching.
No custom edx: commands -- only config_board("impedance_mode:X"),
start_stream(), stop_stream(), release_session().

Usage:
    python python_package/examples/tests/test_mode_transitions.py
    python python_package/examples/tests/test_mode_transitions.py --board-id 81 --verbose
"""

import argparse
import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, IpProtocolTypes


def make_board(args):
    params = BrainFlowInputParams()
    params.ip_address = args.ip
    params.ip_port = args.port
    params.ip_protocol = IpProtocolTypes.EDX.value
    params.timeout = args.timeout
    return BoardShim(args.board_id, params)


def step(name):
    print(f"\n--- {name} ---")


def main():
    p = argparse.ArgumentParser(description="BrainFlow standard mode transition test")
    p.add_argument("--ip", default="localhost")
    p.add_argument("--port", type=int, default=3390)
    p.add_argument("--board-id", type=int, default=81)
    p.add_argument("--timeout", type=int, default=15)
    p.add_argument("--verbose", action="store_true")
    args = p.parse_args()

    if args.verbose:
        BoardShim.enable_dev_board_logger()
    else:
        BoardShim.enable_board_logger()

    board = make_board(args)

    step("1. prepare_session (connect)")
    board.prepare_session()
    print("  OK")

    step("2. start_stream (EEG)")
    board.start_stream()
    time.sleep(1.0)
    data = board.get_board_data()
    print(f"  EEG samples: {data.shape[1]}")
    assert data.shape[1] > 0, "No EEG data"

    step("3. config_board('impedance_mode:1') while streaming")
    t0 = time.time()
    resp = board.config_board("impedance_mode:1")
    dt = time.time() - t0
    print(f"  Response: {resp}")
    print(f"  Took: {dt:.3f}s")
    time.sleep(2.0)
    board.get_board_data()
    time.sleep(1.0)
    data = board.get_board_data()
    print(f"  Impedance samples: {data.shape[1]}")

    step("4. config_board('impedance_mode:0') while streaming")
    t0 = time.time()
    resp = board.config_board("impedance_mode:0")
    dt = time.time() - t0
    print(f"  Response: {resp}")
    print(f"  Took: {dt:.3f}s")
    time.sleep(1.0)
    board.get_board_data()
    time.sleep(1.0)
    data = board.get_board_data()
    print(f"  EEG samples after switch back: {data.shape[1]}")
    assert data.shape[1] > 0, "No EEG data after impedance->EEG"

    step("5. stop_stream (goes to idle)")
    t0 = time.time()
    board.stop_stream()
    dt = time.time() - t0
    print(f"  Took: {dt:.3f}s")

    step("6. start_stream again after stop")
    t0 = time.time()
    board.start_stream()
    dt = time.time() - t0
    print(f"  Took: {dt:.3f}s")
    time.sleep(1.0)
    data = board.get_board_data()
    print(f"  EEG samples after restart: {data.shape[1]}")
    assert data.shape[1] > 0, "No EEG data after restart"

    step("7. stop_stream + release_session")
    t0 = time.time()
    board.stop_stream()
    dt_stop = time.time() - t0
    t0 = time.time()
    board.release_session()
    dt_release = time.time() - t0
    print(f"  stop: {dt_stop:.3f}s, release: {dt_release:.3f}s")

    step("8. reconnect probe (verify clean dispose)")
    time.sleep(1.0)
    board2 = make_board(args)
    t0 = time.time()
    board2.prepare_session()
    dt = time.time() - t0
    print(f"  Reconnected in {dt:.3f}s -- no 'Amplifier in use' error")
    board2.release_session()
    print("  Probe released cleanly")

    print(f"\n{'=' * 50}")
    print("ALL STEPS PASSED")
    print(f"{'=' * 50}")


if __name__ == "__main__":
    main()
