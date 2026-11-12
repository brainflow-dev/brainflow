"""
EXPANDABLE_EEG_32_WIFI_BOARD — full validation for 32-channel mappings + streaming.

================================================================================
TEST PLAN（与固件/OpenBCI 约定一致部分；BrainFlow 侧校验 x…X 字母表）
================================================================================
前置
  - 拷贝 compiled/Debug|Release/BoardController.dll → python_package/brainflow/lib/
  - pip: numpy, nptyping==1.4.4
  - 帽端 WiFi 已连接；PC 与 shield 在同一网段；--ip-port 与 HTTP /tcp 配置一致

阶段 0 — BrainFlow 校验（不应发往无效字母）
  0.1 发送非法 x9060110X（通道字母 '9'），期望 INVALID_ARGUMENTS

阶段 1 — x…X 依次「通电开启」32 路（POWER=0，GAIN=24 即 '6'）
  对 ch=1..32：cmd = x + LETTER(ch) + "060110X"
  期望：每步 STATUS_OK（ExpandableEeg32WifiGainTracker 识别 32 个字母）

阶段 2 — x…X 依次「掉电关闭」32 路（POWER=1）
  对 ch=1..32：cmd = x + LETTER(ch) + "160110X"
  期望：每步 STATUS_OK

阶段 3 — 恢复默认
  发送 "d"（与 Cyton+Daisy 一致），期望 STATUS_OK

阶段 4 — 单字符「开启」类命令（与 Cyton 1–16 + 扩展 17–32 映射）
  ch 1–8:   '1'..'8'
  ch 9–16:  Q W E R T Y U I
  ch 17–24: A S D F G H J K
  ch 25–32: Z X C V B N M L
  依次各发送 1 字符，期望 STATUS_OK（由 shield 转发至板卡）

阶段 5 — 单字符「关闭」
  ch 1–8:   ! @ # $ % ^ & * （第 8 路关闭为 *，与常见 Cyton 文档一致）
  ch 9–16:  q w e r t y u i
  ch 17–32: a s d f g h j k z x c v b n m l  （与 OPEN_CHARS 大写一一对应）
  依次发送，期望 STATUS_OK

阶段 6 — 阻抗通道指定（单字符，与 OpenBCI Cyton 选通道测阻抗同一风格）
  说明：板卡描述无 resistance_channels；此处仅验证命令被 BrainFlow 接受并转发。
  可选：先发 'z' 再逐通道发 OPEN_CHAR（--impedance-with-z）；默认仅逐通道 OPEN_CHAR。

阶段 7 — 采集与健康检查
  start_stream → sleep → stop_stream → get_board_data 形状 (num_rows, N)，EEG 有限值比例

================================================================================
"""

from __future__ import annotations

import argparse
import logging
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable, List, Tuple

import numpy as np

_REPO_ROOT = Path(__file__).resolve().parents[3]
_PY_PKG = _REPO_ROOT / "python_package"
if str(_PY_PKG) not in sys.path:
    sys.path.insert(0, str(_PY_PKG))

from brainflow.board_shim import BoardIds, BoardShim, BrainFlowInputParams, IpProtocolTypes
from brainflow.exit_codes import BrainFlowError, BrainFlowExitCodes

# Ch 1–16: Cyton + Daisy；17–32：扩展（与 C++ ExpandableEeg32WifiGainTracker 一致）
OPEN_CHARS = (
    "12345678"
    "QWERTYUI"
    "ASDFGHJK"
    "ZXCVBNML"
)

# 单字符关闭：前 8 路为 !@#$%^&*（Cyton 常见映射；原 "!@#$%^&" 仅 7 个符号需补第 8 路）
# 9–16: qwertyui；17–32: asdfghjk + zxcvbnml
CLOSE_CHARS = (
    "!@#$%^&*"
    "qwertyui"
    "asdfghjk"
    "zxcvbnml"
)
assert len(OPEN_CHARS) == 32 and len(CLOSE_CHARS) == 32


def letter_for_channel(ch: int) -> str:
    if not (1 <= ch <= 32):
        raise ValueError(ch)
    return OPEN_CHARS[ch - 1]


@dataclass
class RunSummary:
    phase: str
    passed: int = 0
    failed: int = 0
    errors: List[str] = field(default_factory=list)


def _ensure_dll(lib_dir: Path, compiled_debug: Path, compiled_release: Path) -> None:
    dst = lib_dir / "BoardController.dll"
    if dst.is_file():
        return
    for src in (compiled_debug, compiled_release):
        if src.is_file():
            import shutil

            lib_dir.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src, dst)
            logging.info("Copied %s -> %s", src, dst)
            return
    logging.warning("BoardController.dll missing; copy from compiled/Debug or Release.")


def _run_phase(
    board: BoardShim,
    phase_name: str,
    steps: List[Tuple[str, str]],
    delay_s: float,
    log_every: int,
) -> RunSummary:
    s = RunSummary(phase=phase_name)
    for i, (label, cmd) in enumerate(steps):
        try:
            board.config_board(cmd)
            s.passed += 1
            if (i + 1) % log_every == 0 or i == 0:
                logging.info("[%s] OK %s cmd=%r", phase_name, label, cmd)
        except BrainFlowError as e:
            s.failed += 1
            msg = f"{label} cmd={cmd!r} -> {e}"
            s.errors.append(msg)
            logging.error("[%s] FAIL %s", phase_name, msg)
        if delay_s > 0:
            time.sleep(delay_s)
    return s


def main() -> int:
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
    parser = argparse.ArgumentParser(description="Expandable 32ch full wire + BrainFlow test")
    parser.add_argument("--ip-address", default="192.168.4.1")
    parser.add_argument("--ip-port", type=int, default=6677)
    parser.add_argument("--stream-seconds", type=float, default=2.0)
    parser.add_argument("--cmd-delay", type=float, default=0.05, help="delay between config_board calls")
    parser.add_argument("--log-every", type=int, default=8, help="log every N steps within long loops")
    parser.add_argument(
        "--impedance-with-z",
        action="store_true",
        help="before impedance chars, send 'z' once (firmware-dependent)",
    )
    parser.add_argument("--skip-stream", action="store_true")
    args = parser.parse_args()

    lib_dir = _REPO_ROOT / "python_package" / "brainflow" / "lib"
    _ensure_dll(
        lib_dir,
        _REPO_ROOT / "compiled" / "Debug" / "BoardController.dll",
        _REPO_ROOT / "compiled" / "Release" / "BoardController.dll",
    )

    params = BrainFlowInputParams()
    params.ip_address = args.ip_address
    params.ip_port = args.ip_port
    params.ip_protocol = int(IpProtocolTypes.TCP)

    board = BoardShim(BoardIds.EXPANDABLE_EEG_32_WIFI_BOARD, params)
    BoardShim.enable_board_logger()

    n_rows = BoardShim.get_num_rows(board.get_board_id())
    eeg_idx = np.array(BoardShim.get_eeg_channels(board.get_board_id()))

    all_summaries: List[RunSummary] = []

    print("\n========== TEST PLAN (see module docstring) ==========\n")

    try:
        logging.info("prepare_session()")
        board.prepare_session()

        # ----- Phase 0 -----
        try:
            board.config_board("x9060110X")
            logging.error("Phase0: expected failure for x9060110X")
            all_summaries.append(RunSummary("phase0_invalid_x", passed=0, failed=1, errors=["no exception"]))
            return 1
        except BrainFlowError as e:
            if e.exit_code != BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value:
                logging.error("Phase0: wrong code %s", e.exit_code)
                return 1
            logging.info("Phase0 OK: invalid x9060110X rejected")
            all_summaries.append(RunSummary("phase0_invalid_x", passed=1, failed=0))

        # ----- Phase 1: x on -----
        steps_x_on = [(f"ch{c}_xon", f"x{letter_for_channel(c)}060110X") for c in range(1, 33)]
        all_summaries.append(
            _run_phase(board, "phase1_x_power_on", steps_x_on, args.cmd_delay, args.log_every)
        )

        # ----- Phase 2: x off -----
        steps_x_off = [(f"ch{c}_xoff", f"x{letter_for_channel(c)}160110X") for c in range(1, 33)]
        all_summaries.append(
            _run_phase(board, "phase2_x_power_off", steps_x_off, args.cmd_delay, args.log_every)
        )

        # ----- Phase 3: defaults -----
        s3 = RunSummary("phase3_defaults_d")
        try:
            board.config_board("d")
            s3.passed = 1
            logging.info("Phase3 OK: d")
        except BrainFlowError as e:
            s3.failed = 1
            s3.errors.append(str(e))
            logging.error("Phase3 FAIL: %s", e)
        all_summaries.append(s3)

        # ----- Phase 4: single-char open -----
        steps_sc_on = [(f"ch{c}_sc_open", OPEN_CHARS[c - 1]) for c in range(1, 33)]
        all_summaries.append(
            _run_phase(board, "phase4_single_char_open", steps_sc_on, args.cmd_delay, args.log_every)
        )

        # ----- Phase 5: single-char close -----
        steps_sc_off = [(f"ch{c}_sc_close", CLOSE_CHARS[c - 1]) for c in range(1, 33)]
        all_summaries.append(
            _run_phase(board, "phase5_single_char_close", steps_sc_off, args.cmd_delay, args.log_every)
        )

        # ----- Phase 6: impedance channel select (same 32 chars as OPEN_CHARS; optional z/Z) -----
        steps_imp: List[Tuple[str, str]] = []
        if args.impedance_with_z:
            steps_imp.append(("imp_prefix_z", "z"))
        for c in range(1, 33):
            steps_imp.append((f"imp_ch{c}", OPEN_CHARS[c - 1]))
        if args.impedance_with_z:
            steps_imp.append(("imp_suffix_Z", "Z"))
        all_summaries.append(
            _run_phase(board, "phase6_impedance_select", steps_imp, args.cmd_delay, args.log_every)
        )

        # ----- Phase 7: stream -----
        if not args.skip_stream:
            board.config_board("d")
            time.sleep(0.2)
            logging.info("Phase7 start_stream %.1fs", args.stream_seconds)
            board.start_stream(45000)
            time.sleep(args.stream_seconds)
            board.stop_stream()
            n_avail = board.get_board_data_count()
            s7 = RunSummary("phase7_stream")
            if n_avail <= 0:
                s7.failed = 1
                s7.errors.append("no samples")
                logging.error("Phase7: no samples")
            else:
                data = board.get_board_data(min(n_avail, 5000))
                if data.shape[0] != n_rows:
                    s7.failed = 1
                    s7.errors.append(f"rows {data.shape[0]} != {n_rows}")
                else:
                    eeg = data[eeg_idx, :]
                    finite_ratio = float(np.mean(np.isfinite(eeg)))
                    logging.info(
                        "Phase7 OK: shape=%s finite_eeg=%.4f",
                        data.shape,
                        finite_ratio,
                    )
                    s7.passed = 1
            all_summaries.append(s7)

        board.release_session()

        # ---------- Summary ----------
        print("\n========== RESULT SUMMARY ==========")
        total_pass = sum(s.passed for s in all_summaries)
        total_fail = sum(s.failed for s in all_summaries)
        for s in all_summaries:
            line = f"  {s.phase}: passed={s.passed} failed={s.failed}"
            if s.errors:
                line += f"  errors={len(s.errors)}"
            print(line)
            for err in s.errors[:5]:
                print(f"    - {err}")
            if len(s.errors) > 5:
                print(f"    ... and {len(s.errors) - 5} more")
        print(f"\nTOTAL: passed_steps={total_pass} failed_steps={total_fail}")
        if total_fail > 0:
            print("OVERALL: FAIL")
            return 1
        print("OVERALL: PASS")
        return 0

    except BrainFlowError as e:
        logging.error("BrainFlowError: %s", e, exc_info=True)
        if board.is_prepared():
            board.release_session()
        return 2
    except Exception:
        logging.exception("Unexpected error")
        if board.is_prepared():
            board.release_session()
        return 3


if __name__ == "__main__":
    sys.exit(main())
