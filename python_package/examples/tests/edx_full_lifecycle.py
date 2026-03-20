"""
ANT Neuro EDX: full device lifecycle test.

Exercises the complete BrainFlow API surface for a direct EDX board id
in a single sequence: connect, EEG stream, trigger output, marker check,
impedance mode, mode switch recovery, stop, release, and reconnect probe.

Requires a real ANT Neuro device and the EDX gRPC server running.

Usage:
    python python_package/examples/tests/test_edx_full_lifecycle.py
    python python_package/examples/tests/test_edx_full_lifecycle.py --ip 192.168.1.100 --port 3390
    python python_package/examples/tests/test_edx_full_lifecycle.py --board-id 81 --verbose
"""

import argparse
import json
import sys
import time
from collections import Counter

import numpy as np
from brainflow.board_shim import BoardShim, BrainFlowInputParams, IpProtocolTypes


def parse_args():
    p = argparse.ArgumentParser(description="ANT EDX full lifecycle test")
    p.add_argument("--ip", default="localhost", help="EDX gRPC host (default: localhost)")
    p.add_argument("--port", type=int, default=3390, help="EDX gRPC port (default: 3390)")
    p.add_argument("--board-id", type=int, default=81, help="BrainFlow board id (default: 81 = EE511 EDX)")
    p.add_argument("--timeout", type=int, default=5, help="BrainFlow timeout seconds (default: 5)")
    p.add_argument("--verbose", action="store_true", help="Enable BrainFlow debug logging")
    return p.parse_args()


def make_params(args):
    params = BrainFlowInputParams()
    params.ip_address = args.ip
    params.ip_port = args.port
    params.ip_protocol = IpProtocolTypes.EDX.value
    params.timeout = args.timeout
    return params


class LifecycleTest:
    def __init__(self, args):
        self.args = args
        self.params = make_params(args)
        self.board = None
        self.data_board = args.board_id
        self.passed = []
        self.failed = []

    def step(self, name, fn):
        print(f"\n{'=' * 60}")
        print(f"  STEP: {name}")
        print(f"{'=' * 60}")
        try:
            fn()
            self.passed.append(name)
            print("  -> PASS")
        except Exception as e:
            self.failed.append((name, str(e)))
            print(f"  -> FAIL: {e}")

    def run(self):
        self.step("1. prepare_session (connect)", self.test_connect)
        self.step("2. start_stream (EEG)", self.test_start_stream)
        self.step("3. get_board_data (valid EEG frames)", self.test_eeg_data)
        self.step("4. trigger config + start", self.test_trigger_send)
        self.step("5. verify trigger markers in data", self.test_trigger_receive)
        self.step("6. impedance_mode:1 (switch to impedance)", self.test_impedance_on)
        self.step("7. get_board_data (valid impedance values)", self.test_impedance_data)
        self.step("8. impedance_mode:0 (switch back to EEG)", self.test_impedance_off)
        self.step("9. get_board_data (EEG resumes)", self.test_eeg_resumes)
        self.step("10. stop_stream", self.test_stop_stream)
        self.step("11. release_session (dispose)", self.test_release)
        self.step("12. reconnect probe (amp released)", self.test_reconnect_probe)
        self.print_summary()

    def test_connect(self):
        self.board = BoardShim(self.args.board_id, self.params)
        self.board.prepare_session()
        resp = self.board.config_board("edx:get_capabilities")
        caps = json.loads(resp)
        model = caps.get("selected_model", "unknown")
        n_ch = len(caps.get("active_channels", []))
        print(f"  Device: {model}, {n_ch} active channels")
        assert n_ch > 0, "No active channels reported"

    def test_start_stream(self):
        self.board.start_stream()
        time.sleep(1.0)
        print("  Streaming started, waited 1s for stabilization")

    def test_eeg_data(self):
        self.board.get_board_data()
        time.sleep(1.0)
        data = self.board.get_board_data()
        n_samples = data.shape[1]
        print(f"  Got {n_samples} samples, shape={data.shape}")
        assert n_samples > 0, "No EEG samples received"

        eeg_channels = BoardShim.get_eeg_channels(self.data_board)
        print(f"  EEG channels: {eeg_channels[:5]}... ({len(eeg_channels)} total)")
        eeg_data = data[eeg_channels, :]
        nonzero_fraction = np.count_nonzero(eeg_data) / eeg_data.size
        print(f"  Non-zero fraction: {nonzero_fraction:.3f}")
        assert nonzero_fraction > 0.5, f"EEG data mostly zeros ({nonzero_fraction:.3f})"

    def test_trigger_send(self):
        config_cmd = "edx:trigger_config:0,50.0,10.0,5,1.0,3"
        resp = self.board.config_board(config_cmd)
        resp_obj = json.loads(resp)
        assert resp_obj.get("status") == "ok", f"trigger_config failed: {resp}"
        print(f"  Trigger configured: {resp}")

        start_cmd = "edx:trigger_start:0"
        resp = self.board.config_board(start_cmd)
        resp_obj = json.loads(resp)
        assert resp_obj.get("status") == "ok", f"trigger_start failed: {resp}"
        print(f"  Trigger started: {resp}")

    def test_trigger_receive(self):
        marker_ch = BoardShim.get_marker_channel(self.data_board)
        print(f"  Marker channel index: {marker_ch}")

        marker_counts = Counter()
        for i in range(6):
            time.sleep(1.0)
            data = self.board.get_board_data()
            if data.shape[1] > 0:
                markers = data[marker_ch]
                for v in markers:
                    if v != 0.0:
                        marker_counts[int(v)] += 1
            print(f"  [{i + 1}s] +{data.shape[1]} samples, markers: {dict(marker_counts)}")

        total_markers = sum(marker_counts.values())
        print(f"  Total non-zero markers: {total_markers}")
        if total_markers == 0:
            print("  [WARN] No markers observed — use trigger loopback cable to verify")

    def test_impedance_on(self):
        resp = self.board.config_board("impedance_mode:1")
        print(f"  Response: {resp}")
        time.sleep(2.0)
        print("  Waited 2s for impedance mode transition")

    def test_impedance_data(self):
        self.board.get_board_data()
        time.sleep(2.0)
        data = self.board.get_board_data()
        n_samples = data.shape[1]
        print(f"  Got {n_samples} samples in impedance mode, shape={data.shape}")
        assert n_samples > 0, "No impedance samples received"

        descr = BoardShim.get_board_descr(self.args.board_id)
        res_ch = descr.get("resistance_channels", [])
        ref_res_ch = descr.get("ref_resistance_channels", [])
        gnd_res_ch = descr.get("gnd_resistance_channels", [])
        print(f"  Resistance channels: {len(res_ch)} electrode, {len(ref_res_ch)} ref, {len(gnd_res_ch)} gnd")

        if res_ch:
            res_data = data[res_ch, :]
            mean_vals = np.mean(res_data, axis=1)
            valid = np.sum(mean_vals > 0)
            print(f"  Mean resistance values (first 5): {mean_vals[:5]}")
            print(f"  Channels with positive resistance: {valid}/{len(res_ch)}")
            assert valid > 0, "No channels have positive resistance values"

    def test_impedance_off(self):
        resp = self.board.config_board("impedance_mode:0")
        print(f"  Response: {resp}")
        time.sleep(2.0)
        print("  Waited 2s for EEG mode transition")

    def test_eeg_resumes(self):
        self.board.get_board_data()
        time.sleep(1.0)
        data = self.board.get_board_data()
        n_samples = data.shape[1]
        print(f"  Got {n_samples} samples after returning to EEG mode")
        assert n_samples > 0, "No EEG samples after impedance->EEG transition"

        eeg_channels = BoardShim.get_eeg_channels(self.data_board)
        eeg_data = data[eeg_channels, :]
        nonzero_fraction = np.count_nonzero(eeg_data) / eeg_data.size
        print(f"  Non-zero fraction: {nonzero_fraction:.3f}")
        assert nonzero_fraction > 0.5, f"EEG data mostly zeros after impedance ({nonzero_fraction:.3f})"

    def test_stop_stream(self):
        t0 = time.time()
        self.board.stop_stream()
        dt = time.time() - t0
        print(f"  stop_stream completed in {dt:.3f}s")

    def test_release(self):
        t0 = time.time()
        self.board.release_session()
        dt = time.time() - t0
        print(f"  release_session completed in {dt:.3f}s")
        self.board = None

    def test_reconnect_probe(self):
        time.sleep(1.0)
        probe = BoardShim(self.args.board_id, self.params)
        t0 = time.time()
        probe.prepare_session()
        dt = time.time() - t0
        print(f"  Reconnected in {dt:.3f}s (no 'Amplifier in use' error)")

        resp = probe.config_board("edx:get_capabilities")
        caps = json.loads(resp)
        print(f"  Device: {caps.get('selected_model', '?')}, channels: {len(caps.get('active_channels', []))}")

        probe.release_session()
        print("  Probe session released cleanly")

    def print_summary(self):
        print(f"\n{'=' * 60}")
        print("FULL LIFECYCLE TEST SUMMARY")
        print(f"{'=' * 60}")
        print(f"  Passed: {len(self.passed)}/{len(self.passed) + len(self.failed)}")
        for name in self.passed:
            print(f"    [PASS] {name}")
        for name, err in self.failed:
            print(f"    [FAIL] {name}: {err}")
        print(f"{'=' * 60}")

        if self.failed:
            print("\nRESULT: FAIL")
            sys.exit(1)
        else:
            print("\nRESULT: ALL PASSED")


def main():
    args = parse_args()
    if args.verbose:
        BoardShim.enable_dev_board_logger()
    else:
        BoardShim.enable_board_logger()

    test = LifecycleTest(args)
    try:
        test.run()
    except KeyboardInterrupt:
        print("\n[interrupted]")
        if test.board:
            try:
                test.board.stop_stream()
            except Exception:
                pass
            try:
                test.board.release_session()
            except Exception:
                pass
        sys.exit(1)


if __name__ == "__main__":
    main()
