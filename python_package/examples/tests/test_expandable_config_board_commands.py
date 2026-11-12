"""
Real-device test: send config_board strings and report BrainFlow OK vs INVALID_ARGUMENTS.

Why xL161000X may fail with INVALID_ARGUMENTS:
  - Board MUST be EXPANDABLE_EEG_32_WIFI_BOARD (69). On CYTON_DAISY_WIFI (6) etc.,
    channel letter 'L' is not in the 16-letter table -> INVALID_ARGUMENTS before HTTP.
  - DLL must be your rebuilt BoardController.dll (32-letter ExpandableEeg32WifiGainTracker).

Fix nptyping first (BrainFlow pins this version):
  py3.9 -m pip install "nptyping==1.4.4"

Usage (PowerShell):
  $env:PYTHONPATH="E:\\Interesting\\BCI\\brainflow-master\\python_package"
  python python_package/examples/tests/test_expandable_config_board_commands.py ^
    --ip 192.168.4.1 --port 6677

Optional: --board 69 (default) or 6 to demonstrate Daisy rejecting L.
"""

from __future__ import annotations

import argparse
import os
import platform
import struct
import sys
from datetime import datetime
from pathlib import Path

_REPO = Path(__file__).resolve().parents[3]
if str(_REPO / "python_package") not in sys.path:
    sys.path.insert(0, str(_REPO / "python_package"))

try:
    from brainflow.board_shim import (
        BoardControllerDLL,
        BoardIds,
        BoardShim,
        BrainFlowInputParams,
        IpProtocolTypes,
    )
    from brainflow.exit_codes import BrainFlowError, BrainFlowExitCodes
except Exception as e:
    msg = str(e).lower()
    if "nptyping" in msg or "invalidargumentserror" in type(e).__name__.lower():
        print(
            "\nBrainFlow needs nptyping 1.4.4 (see python_package/setup.py).\n"
            "  py3.9 -m pip install \"nptyping==1.4.4\"\n",
            file=sys.stderr,
        )
    raise

def _brainflow_dll_relative_name() -> str:
    if platform.system() == "Windows":
        return (
            "lib\\BoardController32.dll"
            if struct.calcsize("P") * 8 != 64
            else "lib\\BoardController.dll"
        )
    if platform.system() == "Darwin":
        return "lib/libBoardController.dylib"
    return "lib/libBoardController.so"


def resolve_board_controller_dll_path() -> Path:
    """Same path BrainFlow uses (pkg_resources under python_package/brainflow)."""
    import pkg_resources

    rel = _brainflow_dll_relative_name()
    return Path(pkg_resources.resource_filename("brainflow", rel)).resolve()


def _format_dll_stat(path: Path) -> str:
    if not path.is_file():
        return "missing"
    st = path.stat()
    mtime = datetime.fromtimestamp(st.st_mtime).strftime("%Y-%m-%d %H:%M:%S")
    return f"size={st.st_size} bytes, mtime={mtime}"


def _module_path_from_handle(handle: int) -> str | None:
    import ctypes

    if not handle:
        return None
    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    GetModuleFileNameW = kernel32.GetModuleFileNameW
    GetModuleFileNameW.argtypes = [ctypes.c_void_p, ctypes.c_wchar_p, ctypes.c_uint32]
    GetModuleFileNameW.restype = ctypes.c_uint32
    buf = ctypes.create_unicode_buffer(32768)
    n = GetModuleFileNameW(ctypes.c_void_p(handle), buf, len(buf))
    if not n:
        return None
    return buf.value


def _loaded_board_controller_path_windows(dll_instance) -> str | None:
    if platform.system() != "Windows":
        return None
    handle = getattr(dll_instance, "_handle", None)
    if handle:
        return _module_path_from_handle(handle)
    import ctypes

    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    handle = kernel32.GetModuleHandleW("BoardController.dll")
    return _module_path_from_handle(handle) if handle else None


def log_board_controller_dll_paths() -> None:
    configured = resolve_board_controller_dll_path()
    print("BoardController library:")
    print(f"  configured (BrainFlow pkg_resources): {configured}")
    print(f"  on disk: {_format_dll_stat(configured)}")

    # Load native lib (same as first BoardShim call)
    dll = BoardControllerDLL.get_instance()

    loaded = _loaded_board_controller_path_windows(dll.lib)
    if loaded:
        loaded_path = Path(loaded)
        print(f"  loaded in process (GetModuleHandleW): {loaded_path}")
        print(f"  loaded file: {_format_dll_stat(loaded_path)}")
        if loaded_path.resolve() != configured.resolve():
            print("  WARNING: loaded path differs from configured pkg_resources path!")
    elif platform.system() == "Windows":
        print("  loaded in process: (BoardController.dll not found in module list yet)")
    print()


# Channel letters (must match ExpandableEeg32WifiGainTracker / GUI 32ch map)
OPEN_CHARS = "12345678QWERTYUIASDFGHJKZXCVBNML"


def letter_for_channel(ch: int) -> str:
    if not (1 <= ch <= 32):
        raise ValueError(ch)
    return OPEN_CHARS[ch - 1]


def impedance_on_cmd(ch: int) -> str:
    """OpenBCI GUI style when enabling impedance on one channel (see setCheckingImpedanceCyton)."""
    c = letter_for_channel(ch)
    return f"x{c}000100Xz{c}01Z"


def impedance_off_cmd(ch: int) -> str:
    c = letter_for_channel(ch)
    return f"x{c}060110Xz{c}00Z"


def impedance_check_cmd(ch: int) -> str:
    """GUI 'Check' path often uses x...060110X + z...00Z (ch1 example: x1060110Xz100Z)."""
    c = letter_for_channel(ch)
    if ch <= 8:
        return f"x{c}060110Xz{c}00Z"
    return f"x{c}060110Xz{c}00Z"


# Power / channel on-off
DEFAULT_CMDS = [
    ("invalid_digit9", "x9060110X"),
    ("ch32_power_off_user", "xL161000X"),
    ("ch32_power_off_alt", "xL160110X"),
    ("ch32_power_on", "xL060110X"),
    ("ch1_power_off", "x1160110X"),
    ("single_L", "L"),
    ("single_l_close", "l"),
]

# Impedance (BrainFlow only validates x...X letters; z...Z is forwarded as-is)
IMPEDANCE_CMDS = [
    ("imp_prefix_z", "z"),
    ("imp_suffix_Z", "Z"),
    ("imp_ch1_gui_check", "x1060110Xz100Z"),
    ("imp_ch1_on", impedance_on_cmd(1)),
    ("imp_ch1_off", impedance_off_cmd(1)),
    ("imp_ch8_on", impedance_on_cmd(8)),
    ("imp_ch16_on", impedance_on_cmd(16)),
    ("imp_ch16_off", impedance_off_cmd(16)),
    ("imp_ch17_on", impedance_on_cmd(17)),
    ("imp_ch17_gui_log", "xA000100XzA01Z"),
    ("imp_ch17_off", impedance_off_cmd(17)),
    ("imp_ch32_on", impedance_on_cmd(32)),
    ("imp_ch32_off", impedance_off_cmd(32)),
]


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--ip", default="192.168.4.1")
    p.add_argument("--port", type=int, default=6677)
    p.add_argument(
        "--board",
        type=int,
        default=69,
        help="69=EXPANDABLE_EEG_32_WIFI_BOARD (L valid). 6=CYTON_DAISY_WIFI (L invalid).",
    )
    p.add_argument("--extra", action="append", default=[], help="Extra command to test (repeatable)")
    p.add_argument("--impedance-only", action="store_true", help="Only run IMPEDANCE_CMDS (+ --extra)")
    p.add_argument("--skip-impedance", action="store_true", help="Skip IMPEDANCE_CMDS")
    p.add_argument(
        "--dll-only",
        action="store_true",
        help="Print BoardController.dll paths and exit (no WiFi / config_board tests)",
    )
    args = p.parse_args()

    if args.dll_only:
        log_board_controller_dll_paths()
        return 0

    params = BrainFlowInputParams()
    params.ip_address = args.ip
    params.ip_port = args.port
    params.ip_protocol = int(IpProtocolTypes.TCP)

    try:
        bid = BoardIds(args.board)
        bname = bid.name
    except ValueError:
        bname = f"unknown({args.board})"
    print(f"board_id={args.board} ({bname})")
    print(f"ip={args.ip} port={args.port}")
    log_board_controller_dll_paths()

    board = BoardShim(args.board, params)
    board.prepare_session()
    try:
        if args.impedance_only:
            steps = list(IMPEDANCE_CMDS)
        elif args.skip_impedance:
            steps = list(DEFAULT_CMDS)
        else:
            steps = list(DEFAULT_CMDS) + list(IMPEDANCE_CMDS)
        steps += [(f"extra_{i}", c) for i, c in enumerate(args.extra)]

        for label, cmd in steps:
            try:
                resp = board.config_board(cmd)
                ok = "OK"
                tail = repr(resp[:120]) if resp else "''"
            except BrainFlowError as e:
                ok = "FAIL"
                tail = f"{e} (exit_code={e.exit_code})"
                if e.exit_code == BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value:
                    tail += "  <- gain_tracker rejected x...X (wrong board id or old DLL?)"
            print(f"  [{ok:4}] {label:24} cmd={cmd!r}")
            print(f"         {tail}")
        print(
            "\nDone. Impedance strings use OpenBCI GUI pattern x{ch}000100Xz{ch}01Z (on) / "
            "x{ch}060110Xz{ch}00Z (off). Ch17-32 use letters A..L in both x and z parts."
        )
        print("If xL... or xA000100XzA01Z fail on GUI but OK here, GUI uses wrong board_id or old DLL.")
        return 0
    finally:
        board.release_session()


if __name__ == "__main__":
    sys.exit(main())
