//! Statistical functions.

use alloc::vec::Vec;

/// Compute the mean of a slice.
#[must_use]
pub fn mean(data: &[f64]) -> f64 {
    if data.is_empty() {
        return 0.0;
    }
    data.iter().sum::<f64>() / data.len() as f64
}

/// Compute the variance of a slice (population variance).
#[must_use]
pub fn variance(data: &[f64]) -> f64 {
    if data.is_empty() {
        return 0.0;
    }
    let m = mean(data);
    data.iter().map(|x| { let d = x - m; d * d }).sum::<f64>() / data.len() as f64
}

/// Compute the sample variance of a slice.
#[must_use]
pub fn sample_variance(data: &[f64]) -> f64 {
    if data.len() < 2 {
        return 0.0;
    }
    let m = mean(data);
    data.iter().map(|x| { let d = x - m; d * d }).sum::<f64>() / (data.len() - 1) as f64
}

/// Compute the standard deviation (population).
#[must_use]
pub fn stddev(data: &[f64]) -> f64 {
    libm::sqrt(variance(data))
}

/// Compute the sample standard deviation.
#[must_use]
pub fn sample_stddev(data: &[f64]) -> f64 {
    libm::sqrt(sample_variance(data))
}

/// Compute the root mean square.
#[must_use]
pub fn rms(data: &[f64]) -> f64 {
    if data.is_empty() {
        return 0.0;
    }
    let sum_sq: f64 = data.iter().map(|x| x * x).sum();
    libm::sqrt(sum_sq / data.len() as f64)
}

/// Compute the covariance between two slices.
#[must_use]
pub fn covariance(x: &[f64], y: &[f64]) -> f64 {
    if x.len() != y.len() || x.is_empty() {
        return 0.0;
    }

    let mx = mean(x);
    let my = mean(y);
    let n = x.len() as f64;

    x.iter()
        .zip(y.iter())
        .map(|(xi, yi)| (xi - mx) * (yi - my))
        .sum::<f64>()
        / n
}

/// Compute the Pearson correlation coefficient.
#[must_use]
pub fn correlation(x: &[f64], y: &[f64]) -> f64 {
    let sx = stddev(x);
    let sy = stddev(y);
    if sx == 0.0 || sy == 0.0 {
        return 0.0;
    }
    covariance(x, y) / (sx * sy)
}

/// Compute the median of a slice.
#[must_use]
pub fn median(data: &[f64]) -> f64 {
    if data.is_empty() {
        return 0.0;
    }

    let mut sorted = data.to_vec();
    // Simple insertion sort for small arrays (no std sort available)
    for i in 1..sorted.len() {
        let key = sorted[i];
        let mut j = i;
        while j > 0 && sorted[j - 1] > key {
            sorted[j] = sorted[j - 1];
            j -= 1;
        }
        sorted[j] = key;
    }

    let n = sorted.len();
    if n % 2 == 0 {
        (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0
    } else {
        sorted[n / 2]
    }
}

/// Perform linear regression and return (slope, intercept).
#[must_use]
pub fn linear_regression(x: &[f64], y: &[f64]) -> (f64, f64) {
    if x.len() != y.len() || x.is_empty() {
        return (0.0, 0.0);
    }

    let mx = mean(x);
    let my = mean(y);
    let cov = covariance(x, y);
    let var_x = variance(x);

    if var_x == 0.0 {
        return (0.0, my);
    }

    let slope = cov / var_x;
    let intercept = my - slope * mx;

    (slope, intercept)
}

/// Detrend a signal by removing linear trend.
#[must_use]
pub fn detrend_linear(data: &[f64]) -> Vec<f64> {
    if data.is_empty() {
        return Vec::new();
    }

    let x: Vec<f64> = (0..data.len()).map(|i| i as f64).collect();
    let (slope, intercept) = linear_regression(&x, data);

    data.iter()
        .enumerate()
        .map(|(i, &y)| y - (slope * i as f64 + intercept))
        .collect()
}

/// Detrend a signal by removing the mean (DC offset).
#[must_use]
pub fn detrend_constant(data: &[f64]) -> Vec<f64> {
    let m = mean(data);
    data.iter().map(|x| x - m).collect()
}

/// Rolling mean with a given window size.
#[must_use]
pub fn rolling_mean(data: &[f64], window: usize) -> Vec<f64> {
    if window == 0 || data.is_empty() {
        return Vec::new();
    }

    let mut result = Vec::with_capacity(data.len());
    let mut sum = 0.0;

    for (i, &val) in data.iter().enumerate() {
        sum += val;
        if i >= window {
            sum -= data[i - window];
        }
        let count = (i + 1).min(window);
        result.push(sum / count as f64);
    }

    result
}

/// Rolling standard deviation with a given window size.
#[must_use]
pub fn rolling_stddev(data: &[f64], window: usize) -> Vec<f64> {
    if window == 0 || data.is_empty() {
        return Vec::new();
    }

    let mut result = Vec::with_capacity(data.len());

    for i in 0..data.len() {
        let start = i.saturating_sub(window - 1);
        let slice = &data[start..=i];
        result.push(stddev(slice));
    }

    result
}

/// Z-score peak detection algorithm.
///
/// Returns a vector of signals: -1 (negative peak), 0 (normal), +1 (positive peak).
#[must_use]
pub fn detect_peaks_z_score(data: &[f64], lag: usize, threshold: f64, influence: f64) -> Vec<i32> {
    if data.len() < lag || lag == 0 {
        return alloc::vec![0; data.len()];
    }

    let mut signals = alloc::vec![0i32; data.len()];
    let mut filtered_y = data.to_vec();
    let mut avg_filter = alloc::vec![0.0; data.len()];
    let mut std_filter = alloc::vec![0.0; data.len()];

    // Initialize with first `lag` elements
    avg_filter[lag - 1] = mean(&data[0..lag]);
    std_filter[lag - 1] = stddev(&data[0..lag]);

    for i in lag..data.len() {
        let diff = libm::fabs(data[i] - avg_filter[i - 1]);

        if diff > threshold * std_filter[i - 1] {
            // Signal detected
            signals[i] = if data[i] > avg_filter[i - 1] { 1 } else { -1 };
            // Reduce influence of this point
            filtered_y[i] = influence * data[i] + (1.0 - influence) * filtered_y[i - 1];
        } else {
            signals[i] = 0;
            filtered_y[i] = data[i];
        }

        // Update rolling statistics
        let window_start = i - lag + 1;
        avg_filter[i] = mean(&filtered_y[window_start..=i]);
        std_filter[i] = stddev(&filtered_y[window_start..=i]);
    }

    signals
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_mean() {
        assert!((mean(&[1.0, 2.0, 3.0, 4.0, 5.0]) - 3.0).abs() < 1e-10);
    }

    #[test]
    fn test_variance() {
        // Variance of [1,2,3,4,5] = 2.0
        assert!((variance(&[1.0, 2.0, 3.0, 4.0, 5.0]) - 2.0).abs() < 1e-10);
    }

    #[test]
    fn test_stddev() {
        let data = [1.0, 2.0, 3.0, 4.0, 5.0];
        assert!((stddev(&data) - libm::sqrt(2.0)).abs() < 1e-10);
    }

    #[test]
    fn test_median() {
        assert!((median(&[1.0, 3.0, 2.0]) - 2.0).abs() < 1e-10);
        assert!((median(&[1.0, 2.0, 3.0, 4.0]) - 2.5).abs() < 1e-10);
    }

    #[test]
    fn test_linear_regression() {
        let x = [1.0, 2.0, 3.0, 4.0, 5.0];
        let y = [2.0, 4.0, 6.0, 8.0, 10.0]; // y = 2x
        let (slope, intercept) = linear_regression(&x, &y);
        assert!((slope - 2.0).abs() < 1e-10);
        assert!(intercept.abs() < 1e-10);
    }

    #[test]
    fn test_correlation() {
        let x = [1.0, 2.0, 3.0, 4.0, 5.0];
        let y = [2.0, 4.0, 6.0, 8.0, 10.0];
        assert!((correlation(&x, &y) - 1.0).abs() < 1e-10);
    }
}
