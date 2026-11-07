"""Replay a raw BrainFlow file with opt-in quality rejection and transition logging.

No board is opened. Example limits must come from the device, in the data's units:
  python quality_band_powers.py --file eeg.tsv --channels 1 2 3 4 \
      --sampling-rate 256 --lower-bound -724.8 --upper-bound 724.8 --min-channels 3
"""
import argparse
import json

import numpy as np

from brainflow.data_filter import DataFilter, SignalQualityOptions
from brainflow.utils import LogLevels


class QualityTransitionLogger:
    """One instance per stream. No global blacklist or per-frame warning spam."""
    def __init__(self):
        self.previous = {}
        self.previous_valid = None

    def update(self, result, end_sample):
        for quality in result['quality']:
            channel, flags = quality['channel'], quality['flags']
            previous = self.previous.get(channel, 0)
            if flags != previous:
                event = dict(event='signal_quality.changed', end_sample=end_sample,
                             state='rejected' if flags else 'recovered', **quality)
                DataFilter.log_message(LogLevels.LEVEL_WARN if flags else LogLevels.LEVEL_INFO,
                                       json.dumps(event, allow_nan=False))
            self.previous[channel] = flags
        if (not result['valid'] and self.previous_valid is not False) or (
                result['valid'] and self.previous_valid is False):
            DataFilter.log_message(LogLevels.LEVEL_INFO if result['valid'] else LogLevels.LEVEL_WARN,
                json.dumps(dict(event='band_powers.validity', end_sample=end_sample,
                                valid=result['valid'], reason=result['reason'],
                                used_channels=result['used_channels'])))
        self.previous_valid = result['valid']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--file', required=True)
    parser.add_argument('--channels', type=int, nargs='+', required=True)
    parser.add_argument('--sampling-rate', type=int, required=True)
    parser.add_argument('--window-seconds', type=float, default=6)
    parser.add_argument('--step-seconds', type=float, default=.2)
    parser.add_argument('--lower-bound', type=float)
    parser.add_argument('--upper-bound', type=float)
    parser.add_argument('--max-clipped-percentage', type=float, default=0)
    parser.add_argument('--flatline-tolerance', type=float, default=0)
    parser.add_argument('--max-flatline-seconds', type=float, default=.5)
    parser.add_argument('--min-channels', type=int, default=1)
    parser.add_argument('--log-file', default='signal-quality.log')
    parser.add_argument('--output', default='quality-band-powers.jsonl')
    args = parser.parse_args()
    if (args.lower_bound is None) != (args.upper_bound is None):
        parser.error('provide both clipping bounds, or neither')
    if (args.sampling_rate < 1 or not np.isfinite(args.window_seconds) or
            not np.isfinite(args.step_seconds) or args.window_seconds <= 0 or args.step_seconds <= 0):
        parser.error('sample rate, window and step must be positive and finite')
    bounds = None if args.lower_bound is None else (args.lower_bound, args.upper_bound)
    options = SignalQualityOptions(bounds, args.max_clipped_percentage, args.flatline_tolerance,
                                   args.max_flatline_seconds, args.min_channels)
    data = DataFilter.read_file(args.file)
    count = int(round(args.window_seconds * args.sampling_rate))
    step = int(round(args.step_seconds * args.sampling_rate))
    if count < 2 or step < 1 or data.shape[1] < count:
        parser.error('need at least one complete window and a step of at least one sample')
    DataFilter.enable_data_logger()
    DataFilter.set_log_file(args.log_file)
    logger = QualityTransitionLogger()
    with open(args.output, 'w', encoding='utf-8') as output:
        for end in range(count, data.shape[1] + 1, step):
            result = DataFilter.get_avg_band_powers_with_quality(
                np.ascontiguousarray(data[:, end-count:end]), args.channels, args.sampling_rate,
                True, options)
            logger.update(result, end)
            for key in ('avg_band_powers', 'stddev_band_powers'):
                if result[key] is not None:
                    result[key] = result[key].tolist()
            result.update(end_sample=end, start_sample=end-count)
            output.write(json.dumps(result, allow_nan=False) + '\n')
    print('Wrote', args.output, 'and', args.log_file)


if __name__ == '__main__':
    main()
