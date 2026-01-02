//! FastICA (Independent Component Analysis) implementation.

use alloc::vec::Vec;
use crate::matrix::Matrix;
use crate::vector::Vector;

/// FastICA parameters.
#[derive(Debug, Clone)]
pub struct FastIcaParams {
    /// Number of independent components to extract.
    pub num_components: usize,
    /// Maximum number of iterations.
    pub max_iterations: usize,
    /// Convergence tolerance.
    pub tolerance: f64,
}

impl Default for FastIcaParams {
    fn default() -> Self {
        Self {
            num_components: 0, // 0 means use all
            max_iterations: 300,
            tolerance: 0.0001,
        }
    }
}

/// FastICA result.
#[derive(Debug, Clone)]
pub struct FastIcaResult {
    /// Unmixing matrix W (num_components × num_components).
    pub unmixing_matrix: Matrix,
    /// Mixing matrix A (num_channels × num_components).
    pub mixing_matrix: Matrix,
    /// Whitening matrix K (num_components × num_channels).
    pub whitening_matrix: Matrix,
    /// Independent components S (num_components × num_samples).
    pub sources: Matrix,
    /// Number of iterations performed.
    pub iterations: usize,
}

/// Perform FastICA on multi-channel data.
///
/// # Arguments
/// * `data` - Data matrix (num_channels × num_samples).
/// * `params` - Algorithm parameters.
///
/// # Returns
/// FastICA result containing unmixing matrix, sources, etc.
pub fn fastica(data: &Matrix, params: &FastIcaParams) -> Option<FastIcaResult> {
    let (num_channels, num_samples) = data.shape();

    if num_channels == 0 || num_samples == 0 {
        return None;
    }

    let num_components = if params.num_components == 0 {
        num_channels
    } else {
        params.num_components.min(num_channels)
    };

    // Step 1: Center the data (subtract mean)
    let centered = center_data(data);

    // Step 2: Whiten the data using PCA
    let (whitened, whitening_matrix) = whiten(&centered, num_components)?;

    // Step 3: FastICA iteration
    let (unmixing_matrix, iterations) = fastica_iteration(
        &whitened,
        num_components,
        params.max_iterations,
        params.tolerance,
    )?;

    // Step 4: Compute sources S = W * whitened
    let sources = unmixing_matrix.matmul(&whitened);

    // Step 5: Compute mixing matrix A = (W * K)^-1
    let wk = unmixing_matrix.matmul(&whitening_matrix);
    let mixing_matrix = wk.inverse().unwrap_or_else(|| wk.transpose());

    Some(FastIcaResult {
        unmixing_matrix,
        mixing_matrix,
        whitening_matrix,
        sources,
        iterations,
    })
}

/// Center data by subtracting the mean of each row.
fn center_data(data: &Matrix) -> Matrix {
    let (rows, cols) = data.shape();
    let mut result = Matrix::zeros(rows, cols);

    for r in 0..rows {
        let row_mean: f64 = (0..cols).map(|c| data.get(r, c)).sum::<f64>() / cols as f64;
        for c in 0..cols {
            result.set(r, c, data.get(r, c) - row_mean);
        }
    }

    result
}

/// Whiten data using eigenvalue decomposition.
fn whiten(data: &Matrix, num_components: usize) -> Option<(Matrix, Matrix)> {
    let (rows, cols) = data.shape();

    // Compute covariance matrix: X * X^T / n
    let cov = data.matmul(&data.transpose()).scale(1.0 / cols as f64);

    // Eigenvalue decomposition (simplified power iteration)
    let (eigenvalues, eigenvectors) = eigen_decomposition(&cov, num_components)?;

    // Compute whitening matrix: D^(-1/2) * E^T
    let mut d_inv_sqrt = Vec::with_capacity(num_components);
    for &ev in &eigenvalues {
        if ev > 1e-10 {
            d_inv_sqrt.push(1.0 / libm::sqrt(ev));
        } else {
            d_inv_sqrt.push(0.0);
        }
    }

    // K = D^(-1/2) * E^T
    let mut whitening_matrix = Matrix::zeros(num_components, rows);
    for i in 0..num_components {
        for j in 0..rows {
            whitening_matrix.set(i, j, d_inv_sqrt[i] * eigenvectors.get(j, i));
        }
    }

    // Whitened data: K * X
    let whitened = whitening_matrix.matmul(data);

    Some((whitened, whitening_matrix))
}

/// Simplified eigenvalue decomposition using power iteration.
fn eigen_decomposition(matrix: &Matrix, num_components: usize) -> Option<(Vec<f64>, Matrix)> {
    let n = matrix.rows();
    if n != matrix.cols() {
        return None;
    }

    let mut eigenvalues = Vec::with_capacity(num_components);
    let mut eigenvectors = Matrix::zeros(n, num_components);
    let mut deflated = matrix.clone();

    for k in 0..num_components {
        // Power iteration to find dominant eigenvector
        let mut v = Vector::ones(n);
        v = v.normalize();

        let mut eigenvalue = 0.0;

        for _ in 0..100 {
            let av = deflated.matvec(&v);
            let new_eigenvalue = v.dot(&av);

            if (new_eigenvalue - eigenvalue).abs() < 1e-10 {
                break;
            }
            eigenvalue = new_eigenvalue;
            v = av.normalize();
        }

        eigenvalues.push(eigenvalue);
        for i in 0..n {
            eigenvectors.set(i, k, v[i]);
        }

        // Deflate: A = A - λ * v * v^T
        for i in 0..n {
            for j in 0..n {
                let val = deflated.get(i, j) - eigenvalue * v[i] * v[j];
                deflated.set(i, j, val);
            }
        }
    }

    Some((eigenvalues, eigenvectors))
}

/// FastICA iteration using tanh non-linearity.
fn fastica_iteration(
    whitened: &Matrix,
    num_components: usize,
    max_iterations: usize,
    tolerance: f64,
) -> Option<(Matrix, usize)> {
    let (_, num_samples) = whitened.shape();

    // Initialize W randomly (using deterministic seed for reproducibility)
    let mut w = initialize_w(num_components);

    // Orthogonalize initial W
    w = orthogonalize(&w)?;

    let mut iterations = 0;

    for iter in 0..max_iterations {
        iterations = iter + 1;
        let w_old = w.clone();

        // Compute W * whitened
        let wx = w.matmul(whitened);

        // Apply non-linearity g(u) = tanh(u)
        // g'(u) = 1 - tanh²(u)
        let mut gwx = Matrix::zeros(num_components, num_samples);
        let mut g_wx_mean = Vector::zeros(num_components);

        for i in 0..num_components {
            for j in 0..num_samples {
                let u = wx.get(i, j);
                let tanh_u = libm::tanh(u);
                gwx.set(i, j, tanh_u);
                g_wx_mean[i] += 1.0 - tanh_u * tanh_u;
            }
            g_wx_mean[i] /= num_samples as f64;
        }

        // Update: W_new = E{X * g(W*X)^T} - E{g'(W*X)} * W
        // Simplified: W_new = (gwx * X^T) / n - diag(g_wx_mean) * W
        let gwx_xt = gwx.matmul(&whitened.transpose()).scale(1.0 / num_samples as f64);

        let mut w_new = Matrix::zeros(num_components, num_components);
        for i in 0..num_components {
            for j in 0..num_components {
                let val = gwx_xt.get(i, j) - g_wx_mean[i] * w.get(i, j);
                w_new.set(i, j, val);
            }
        }

        // Orthogonalize
        w = orthogonalize(&w_new)?;

        // Check convergence
        let mut max_change = 0.0f64;
        for i in 0..num_components {
            let mut dot = 0.0;
            for j in 0..num_components {
                dot += w.get(i, j) * w_old.get(i, j);
            }
            let change = libm::fabs(libm::fabs(dot) - 1.0);
            max_change = max_change.max(change);
        }

        if max_change < tolerance {
            break;
        }
    }

    Some((w, iterations))
}

/// Initialize W matrix (pseudo-random for reproducibility).
fn initialize_w(n: usize) -> Matrix {
    let mut w = Matrix::zeros(n, n);
    let mut seed = 42u64;

    for i in 0..n {
        for j in 0..n {
            // Simple LCG random number generator
            seed = seed.wrapping_mul(1103515245).wrapping_add(12345);
            let rand = ((seed >> 16) & 0x7FFF) as f64 / 32768.0 - 0.5;
            w.set(i, j, rand);
        }
    }

    w
}

/// Orthogonalize W using symmetric decorrelation.
fn orthogonalize(w: &Matrix) -> Option<Matrix> {
    // W = W * (W * W^T)^(-1/2)
    let wwt = w.matmul(&w.transpose());

    // Compute (W * W^T)^(-1/2) using eigendecomposition
    let n = wwt.rows();
    let (eigenvalues, eigenvectors) = eigen_decomposition(&wwt, n)?;

    // D^(-1/2)
    let mut d_inv_sqrt = Matrix::zeros(n, n);
    for i in 0..n {
        if eigenvalues[i] > 1e-10 {
            d_inv_sqrt.set(i, i, 1.0 / libm::sqrt(eigenvalues[i]));
        }
    }

    // (W * W^T)^(-1/2) = E * D^(-1/2) * E^T
    let temp = eigenvectors.matmul(&d_inv_sqrt);
    let wwt_inv_sqrt = temp.matmul(&eigenvectors.transpose());

    Some(w.matmul(&wwt_inv_sqrt))
}

/// Perform ICA and return unmixing matrix, mixing matrix, and sources.
///
/// This is a convenience wrapper matching the BrainFlow interface.
pub fn perform_ica(
    data: &[f64],
    num_channels: usize,
    num_samples: usize,
    num_components: usize,
) -> Option<(Vec<f64>, Vec<f64>, Vec<f64>, Vec<f64>)> {
    if data.len() != num_channels * num_samples {
        return None;
    }

    // Convert to matrix (row-major: channels × samples)
    let matrix = Matrix::from_row_major(data.to_vec(), num_channels, num_samples);

    let params = FastIcaParams {
        num_components,
        ..Default::default()
    };

    let result = fastica(&matrix, &params)?;

    Some((
        result.unmixing_matrix.as_slice().to_vec(),
        result.mixing_matrix.as_slice().to_vec(),
        result.whitening_matrix.as_slice().to_vec(),
        result.sources.as_slice().to_vec(),
    ))
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::vec;

    #[test]
    fn test_center_data() {
        let data = Matrix::from_row_major(vec![1.0, 2.0, 3.0, 4.0, 5.0, 6.0], 2, 3);
        let centered = center_data(&data);

        // Row means: [2, 5]
        // Centered: [-1, 0, 1, -1, 0, 1]
        assert!((centered.get(0, 0) - (-1.0)).abs() < 1e-10);
        assert!((centered.get(0, 1) - 0.0).abs() < 1e-10);
        assert!((centered.get(0, 2) - 1.0).abs() < 1e-10);
    }

    #[test]
    fn test_fastica_runs() {
        // Simple test to ensure FastICA runs without panic
        let data = Matrix::from_row_major(
            vec![
                1.0, 2.0, 3.0, 4.0,
                2.0, 1.0, 4.0, 3.0,
            ],
            2,
            4,
        );

        let params = FastIcaParams {
            num_components: 2,
            max_iterations: 10,
            tolerance: 0.01,
        };

        let result = fastica(&data, &params);
        assert!(result.is_some());
    }
}
