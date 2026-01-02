//! Matrix operations for linear algebra.

use alloc::vec::Vec;
use crate::vector::Vector;

/// A row-major matrix of f64 values.
#[derive(Debug, Clone, PartialEq)]
pub struct Matrix {
    data: Vec<f64>,
    rows: usize,
    cols: usize,
}

impl Matrix {
    /// Create a new matrix from row-major data.
    ///
    /// # Panics
    /// Panics if data length doesn't match rows * cols.
    #[must_use]
    pub fn from_row_major(data: Vec<f64>, rows: usize, cols: usize) -> Self {
        assert_eq!(
            data.len(),
            rows * cols,
            "Data length must match rows * cols"
        );
        Self { data, rows, cols }
    }

    /// Create a matrix of zeros.
    #[must_use]
    pub fn zeros(rows: usize, cols: usize) -> Self {
        Self {
            data: alloc::vec![0.0; rows * cols],
            rows,
            cols,
        }
    }

    /// Create an identity matrix.
    #[must_use]
    pub fn identity(n: usize) -> Self {
        let mut m = Self::zeros(n, n);
        for i in 0..n {
            m.set(i, i, 1.0);
        }
        m
    }

    /// Create a matrix of ones.
    #[must_use]
    pub fn ones(rows: usize, cols: usize) -> Self {
        Self {
            data: alloc::vec![1.0; rows * cols],
            rows,
            cols,
        }
    }

    /// Create a diagonal matrix from a vector.
    #[must_use]
    pub fn diag(values: &[f64]) -> Self {
        let n = values.len();
        let mut m = Self::zeros(n, n);
        for (i, &v) in values.iter().enumerate() {
            m.set(i, i, v);
        }
        m
    }

    /// Get the number of rows.
    #[must_use]
    pub const fn rows(&self) -> usize {
        self.rows
    }

    /// Get the number of columns.
    #[must_use]
    pub const fn cols(&self) -> usize {
        self.cols
    }

    /// Get the shape as (rows, cols).
    #[must_use]
    pub const fn shape(&self) -> (usize, usize) {
        (self.rows, self.cols)
    }

    /// Get a reference to the underlying data in row-major order.
    #[must_use]
    pub fn as_slice(&self) -> &[f64] {
        &self.data
    }

    /// Get a mutable reference to the underlying data.
    pub fn as_mut_slice(&mut self) -> &mut [f64] {
        &mut self.data
    }

    /// Get an element at (row, col).
    #[must_use]
    pub fn get(&self, row: usize, col: usize) -> f64 {
        self.data[row * self.cols + col]
    }

    /// Set an element at (row, col).
    pub fn set(&mut self, row: usize, col: usize, value: f64) {
        self.data[row * self.cols + col] = value;
    }

    /// Get a row as a Vector.
    #[must_use]
    pub fn row(&self, row: usize) -> Vector {
        let start = row * self.cols;
        Vector::from_slice(&self.data[start..start + self.cols])
    }

    /// Get a column as a Vector.
    #[must_use]
    pub fn col(&self, col: usize) -> Vector {
        let data: Vec<f64> = (0..self.rows).map(|r| self.get(r, col)).collect();
        Vector::from(data)
    }

    /// Set a row from a slice.
    pub fn set_row(&mut self, row: usize, values: &[f64]) {
        let start = row * self.cols;
        self.data[start..start + self.cols].copy_from_slice(values);
    }

    /// Set a column from a slice.
    pub fn set_col(&mut self, col: usize, values: &[f64]) {
        for (r, &v) in values.iter().enumerate() {
            self.set(r, col, v);
        }
    }

    /// Transpose the matrix.
    #[must_use]
    pub fn transpose(&self) -> Self {
        let mut result = Self::zeros(self.cols, self.rows);
        for r in 0..self.rows {
            for c in 0..self.cols {
                result.set(c, r, self.get(r, c));
            }
        }
        result
    }

    /// Matrix multiplication.
    ///
    /// # Panics
    /// Panics if dimensions don't match.
    #[must_use]
    pub fn matmul(&self, other: &Self) -> Self {
        assert_eq!(
            self.cols, other.rows,
            "Matrix dimensions don't match for multiplication"
        );

        let mut result = Self::zeros(self.rows, other.cols);
        for i in 0..self.rows {
            for j in 0..other.cols {
                let mut sum = 0.0;
                for k in 0..self.cols {
                    sum += self.get(i, k) * other.get(k, j);
                }
                result.set(i, j, sum);
            }
        }
        result
    }

    /// Matrix-vector multiplication.
    #[must_use]
    pub fn matvec(&self, v: &Vector) -> Vector {
        assert_eq!(
            self.cols,
            v.len(),
            "Matrix columns must match vector length"
        );

        let data: Vec<f64> = (0..self.rows)
            .map(|r| {
                (0..self.cols)
                    .map(|c| self.get(r, c) * v[c])
                    .sum()
            })
            .collect();
        Vector::from(data)
    }

    /// Element-wise addition.
    #[must_use]
    pub fn add(&self, other: &Self) -> Self {
        assert_eq!(self.shape(), other.shape());
        Self {
            data: self
                .data
                .iter()
                .zip(other.data.iter())
                .map(|(a, b)| a + b)
                .collect(),
            rows: self.rows,
            cols: self.cols,
        }
    }

    /// Element-wise subtraction.
    #[must_use]
    pub fn sub(&self, other: &Self) -> Self {
        assert_eq!(self.shape(), other.shape());
        Self {
            data: self
                .data
                .iter()
                .zip(other.data.iter())
                .map(|(a, b)| a - b)
                .collect(),
            rows: self.rows,
            cols: self.cols,
        }
    }

    /// Scale by a constant.
    #[must_use]
    pub fn scale(&self, s: f64) -> Self {
        Self {
            data: self.data.iter().map(|x| x * s).collect(),
            rows: self.rows,
            cols: self.cols,
        }
    }

    /// Compute the Frobenius norm.
    #[must_use]
    pub fn frobenius_norm(&self) -> f64 {
        libm::sqrt(self.data.iter().map(|x| x * x).sum())
    }

    /// Get the diagonal elements.
    #[must_use]
    pub fn diagonal(&self) -> Vector {
        let n = self.rows.min(self.cols);
        let data: Vec<f64> = (0..n).map(|i| self.get(i, i)).collect();
        Vector::from(data)
    }

    /// Compute the trace (sum of diagonal elements).
    #[must_use]
    pub fn trace(&self) -> f64 {
        self.diagonal().sum()
    }

    /// LU decomposition with partial pivoting.
    /// Returns (L, U, P) where PA = LU.
    #[must_use]
    pub fn lu(&self) -> Option<(Self, Self, Vec<usize>)> {
        if self.rows != self.cols {
            return None;
        }

        let n = self.rows;
        let mut l = Self::zeros(n, n);
        let mut u = self.clone();
        let mut perm: Vec<usize> = (0..n).collect();

        for k in 0..n {
            // Find pivot
            let mut max_val = libm::fabs(u.get(k, k));
            let mut max_row = k;
            for i in (k + 1)..n {
                let val = libm::fabs(u.get(i, k));
                if val > max_val {
                    max_val = val;
                    max_row = i;
                }
            }

            if max_val < 1e-14 {
                return None; // Singular matrix
            }

            // Swap rows
            if max_row != k {
                perm.swap(k, max_row);
                for j in 0..n {
                    let tmp = u.get(k, j);
                    u.set(k, j, u.get(max_row, j));
                    u.set(max_row, j, tmp);
                }
                for j in 0..k {
                    let tmp = l.get(k, j);
                    l.set(k, j, l.get(max_row, j));
                    l.set(max_row, j, tmp);
                }
            }

            l.set(k, k, 1.0);

            // Elimination
            for i in (k + 1)..n {
                let factor = u.get(i, k) / u.get(k, k);
                l.set(i, k, factor);
                for j in k..n {
                    let val = u.get(i, j) - factor * u.get(k, j);
                    u.set(i, j, val);
                }
            }
        }

        Some((l, u, perm))
    }

    /// Solve Ax = b using LU decomposition.
    #[must_use]
    pub fn solve(&self, b: &Vector) -> Option<Vector> {
        let (l, u, perm) = self.lu()?;
        let n = self.rows;

        // Apply permutation to b
        let mut pb: Vec<f64> = perm.iter().map(|&i| b[i]).collect();

        // Forward substitution: Ly = Pb
        for i in 0..n {
            for j in 0..i {
                pb[i] -= l.get(i, j) * pb[j];
            }
        }

        // Back substitution: Ux = y
        let mut x = pb;
        for i in (0..n).rev() {
            for j in (i + 1)..n {
                x[i] -= u.get(i, j) * x[j];
            }
            x[i] /= u.get(i, i);
        }

        Some(Vector::from(x))
    }

    /// Compute the determinant using LU decomposition.
    #[must_use]
    pub fn det(&self) -> f64 {
        if self.rows != self.cols {
            return 0.0;
        }

        if let Some((_, u, perm)) = self.lu() {
            let mut det = 1.0;
            for i in 0..self.rows {
                det *= u.get(i, i);
            }

            // Count permutation sign
            let mut sign = 1;
            let mut visited = alloc::vec![false; perm.len()];
            for i in 0..perm.len() {
                if visited[i] {
                    continue;
                }
                let mut j = i;
                let mut cycle_len = 0;
                while !visited[j] {
                    visited[j] = true;
                    j = perm[j];
                    cycle_len += 1;
                }
                if cycle_len > 1 && (cycle_len - 1) % 2 == 1 {
                    sign = -sign;
                }
            }

            det * sign as f64
        } else {
            0.0
        }
    }

    /// Compute the inverse using LU decomposition.
    #[must_use]
    pub fn inverse(&self) -> Option<Self> {
        if self.rows != self.cols {
            return None;
        }

        let n = self.rows;
        let mut result = Self::zeros(n, n);

        for j in 0..n {
            let mut e = Vector::zeros(n);
            e[j] = 1.0;
            let col = self.solve(&e)?;
            result.set_col(j, col.as_slice());
        }

        Some(result)
    }

    /// Apply a function element-wise.
    #[must_use]
    pub fn map<F>(&self, f: F) -> Self
    where
        F: Fn(f64) -> f64,
    {
        Self {
            data: self.data.iter().map(|&x| f(x)).collect(),
            rows: self.rows,
            cols: self.cols,
        }
    }

    /// Compute the mean of each column.
    #[must_use]
    pub fn column_mean(&self) -> Vector {
        let data: Vec<f64> = (0..self.cols)
            .map(|c| {
                let sum: f64 = (0..self.rows).map(|r| self.get(r, c)).sum();
                sum / self.rows as f64
            })
            .collect();
        Vector::from(data)
    }

    /// Compute the mean of each row.
    #[must_use]
    pub fn row_mean(&self) -> Vector {
        let data: Vec<f64> = (0..self.rows)
            .map(|r| {
                let sum: f64 = (0..self.cols).map(|c| self.get(r, c)).sum();
                sum / self.cols as f64
            })
            .collect();
        Vector::from(data)
    }

    /// Center the matrix by subtracting the column mean.
    #[must_use]
    pub fn center_columns(&self) -> Self {
        let means = self.column_mean();
        let mut result = self.clone();
        for r in 0..self.rows {
            for c in 0..self.cols {
                result.set(r, c, self.get(r, c) - means[c]);
            }
        }
        result
    }

    /// Compute the covariance matrix (columns are variables).
    #[must_use]
    pub fn covariance(&self) -> Self {
        let centered = self.center_columns();
        let n = self.rows as f64;
        centered.transpose().matmul(&centered).scale(1.0 / n)
    }
}

impl core::ops::Index<(usize, usize)> for Matrix {
    type Output = f64;

    fn index(&self, (row, col): (usize, usize)) -> &Self::Output {
        &self.data[row * self.cols + col]
    }
}

impl core::ops::IndexMut<(usize, usize)> for Matrix {
    fn index_mut(&mut self, (row, col): (usize, usize)) -> &mut Self::Output {
        &mut self.data[row * self.cols + col]
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::vec;

    #[test]
    fn test_identity() {
        let i = Matrix::identity(3);
        assert_eq!(i.get(0, 0), 1.0);
        assert_eq!(i.get(1, 1), 1.0);
        assert_eq!(i.get(2, 2), 1.0);
        assert_eq!(i.get(0, 1), 0.0);
    }

    #[test]
    fn test_matmul() {
        let a = Matrix::from_row_major(vec![1.0, 2.0, 3.0, 4.0], 2, 2);
        let b = Matrix::from_row_major(vec![5.0, 6.0, 7.0, 8.0], 2, 2);
        let c = a.matmul(&b);
        // [1 2] [5 6]   [19 22]
        // [3 4] [7 8] = [43 50]
        assert!((c.get(0, 0) - 19.0).abs() < 1e-10);
        assert!((c.get(0, 1) - 22.0).abs() < 1e-10);
        assert!((c.get(1, 0) - 43.0).abs() < 1e-10);
        assert!((c.get(1, 1) - 50.0).abs() < 1e-10);
    }

    #[test]
    fn test_transpose() {
        let a = Matrix::from_row_major(vec![1.0, 2.0, 3.0, 4.0, 5.0, 6.0], 2, 3);
        let t = a.transpose();
        assert_eq!(t.shape(), (3, 2));
        assert_eq!(t.get(0, 0), 1.0);
        assert_eq!(t.get(0, 1), 4.0);
        assert_eq!(t.get(1, 0), 2.0);
    }

    #[test]
    fn test_det() {
        let a = Matrix::from_row_major(vec![1.0, 2.0, 3.0, 4.0], 2, 2);
        // det = 1*4 - 2*3 = -2
        assert!((a.det() - (-2.0)).abs() < 1e-10);
    }

    #[test]
    fn test_inverse() {
        let a = Matrix::from_row_major(vec![4.0, 7.0, 2.0, 6.0], 2, 2);
        let inv = a.inverse().unwrap();
        let i = a.matmul(&inv);
        // Should be close to identity
        assert!((i.get(0, 0) - 1.0).abs() < 1e-10);
        assert!((i.get(1, 1) - 1.0).abs() < 1e-10);
        assert!(i.get(0, 1).abs() < 1e-10);
        assert!(i.get(1, 0).abs() < 1e-10);
    }
}
