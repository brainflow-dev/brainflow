import numpy as np

from brainflow.data_filter import AggOperations, DataFilter


def main():
    # One large outlier per window, so the median and the mean of a window are far apart
    # and a median that silently falls back to the mean is visible.
    data = [1.0, 2.0, 3.0, 100.0, 4.0, 5.0, 6.0, 200.0]

    # Period 2 is unchanged by design. With two values the median is their mean, so this
    # pins that the even-period path did not shift the one case that was already correct.
    period_2 = DataFilter.perform_downsampling(np.array(data), 2, AggOperations.MEDIAN.value)
    assert np.allclose(period_2, [1.5, 51.5, 4.5, 103.0]), period_2

    # Odd periods keep the existing single middle value.
    period_3 = DataFilter.perform_downsampling(np.array(data), 3, AggOperations.MEDIAN.value)
    assert np.allclose(period_3, [2.0, 5.0]), period_3

    # Period 4 is the regression. Each window is the mean of the two sorted middle values,
    # (2 + 3) / 2 and (5 + 6) / 2, rather than the full-window mean downsample_median
    # used to return for every even period.
    period_4 = DataFilter.perform_downsampling(np.array(data), 4, AggOperations.MEDIAN.value)
    assert np.allclose(period_4, [2.5, 5.5]), period_4

    # The same windows through MEAN, to show the two operations are now distinct where
    # they used to return identical values.
    mean_4 = DataFilter.perform_downsampling(np.array(data), 4, AggOperations.MEAN.value)
    assert np.allclose(mean_4, [26.5, 53.75]), mean_4
    assert not np.allclose(period_4, mean_4), (period_4, mean_4)

    mean_3 = DataFilter.perform_downsampling(np.array(data), 3, AggOperations.MEAN.value)
    assert not np.allclose(period_3, mean_3), (period_3, mean_3)

    print('downsampling median regression passed')


if __name__ == '__main__':
    main()
