"""Native wavelet, CSP and ICA safety and numerical regression checks.

Run with the local Python package and freshly built native libraries:
    python python_package/examples/tests/decomposition_regression.py
"""

import ctypes
import unittest

import numpy as np

from brainflow.data_filter import DataHandlerDLL


DOUBLE = ctypes.POINTER(ctypes.c_double)
INTEGER = ctypes.POINTER(ctypes.c_int)
INT = ctypes.c_int
INVALID = 13
BUFFER_ERROR = 9
GENERAL_ERROR = 17


def doubles(array):
    return array.ctypes.data_as(DOUBLE)


def integers(array):
    return array.ctypes.data_as(INTEGER)


class DecompositionRegression(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        library = DataHandlerDLL.get_instance().lib

        def native(name, *arguments):
            # Independent prototypes keep tests from changing binding-level argtypes.
            return ctypes.CFUNCTYPE(INT, *arguments)((name, library))

        cls.forward = staticmethod(native(
            'perform_wavelet_transform', DOUBLE, INT, INT, INT, INT, DOUBLE, INTEGER))
        cls.inverse = staticmethod(native(
            'perform_inverse_wavelet_transform_checked', DOUBLE, INT, INT, INT, INT,
            INT, INTEGER, INT, DOUBLE, INT))
        cls.legacy_inverse = staticmethod(native(
            'perform_inverse_wavelet_transform', DOUBLE, INT, INT, INT, INT, INTEGER, DOUBLE))
        cls.denoise = staticmethod(native(
            'perform_wavelet_denoising', DOUBLE, INT, INT, INT, INT, INT, INT, INT))
        cls.restore = staticmethod(native(
            'restore_data_from_wavelet_detailed_coeffs', DOUBLE, INT, INT, INT, INT, DOUBLE))
        cls.csp = staticmethod(native(
            'get_csp', DOUBLE, DOUBLE, INT, INT, INT, DOUBLE, DOUBLE))
        cls.ica = staticmethod(native(
            'perform_ica_with_options', DOUBLE, INT, INT, INT, DOUBLE, DOUBLE,
            DOUBLE, DOUBLE, INT, ctypes.c_double, INT))

    def transform(self, data, wavelet=3, level=2, extension=0):
        coefficients = np.empty(data.size + 82 * level, dtype=np.float64)
        lengths = np.empty(level + 1, dtype=np.int32)
        self.assertEqual(self.forward(doubles(data), data.size, wavelet, level, extension,
                                      doubles(coefficients), integers(lengths)), 0)
        return coefficients[:int(lengths.sum())].copy(), lengths

    def reconstruct(self, coefficients, lengths, count, wavelet=3, level=2, extension=0):
        output = np.empty(count, dtype=np.float64)
        self.assertEqual(self.inverse(
            doubles(coefficients), coefficients.size, count, wavelet, level, extension,
            integers(lengths), lengths.size, doubles(output), output.size), 0)
        return output

    def test_wavelet_roundtrips_all_families(self):
        generator = np.random.default_rng(745)
        for count in (256, 257):
            data = generator.normal(size=count)
            for wavelet in range(45):
                for extension in (0, 1):
                    for level in (1, 2, 3):
                        with self.subTest(count=count, wavelet=wavelet,
                                          extension=extension, level=level):
                            coefficients, lengths = self.transform(
                                data, wavelet, level, extension)
                            restored = self.reconstruct(
                                coefficients, lengths, count, wavelet, level, extension)
                            np.testing.assert_allclose(restored, data, atol=1e-9, rtol=1e-9)

    def test_inverse_rejects_inconsistent_metadata_before_access(self):
        data = np.linspace(-1.0, 1.0, 64)
        coefficients, lengths = self.transform(data)
        output = np.full(64, 321.0)
        for bad_lengths in (np.array([1000, 1000, 1000], dtype=np.int32),
                            np.array([-1, 19, 34], dtype=np.int32),
                            lengths[::-1].copy()):
            with self.subTest(lengths=bad_lengths):
                self.assertEqual(self.inverse(
                    doubles(coefficients), coefficients.size, 64, 3, 2, 0,
                    integers(bad_lengths), bad_lengths.size, doubles(output), output.size),
                    INVALID)
                self.assertEqual(self.legacy_inverse(
                    doubles(coefficients), 64, 3, 2, 0, integers(bad_lengths),
                    doubles(output)), INVALID)
        self.assertEqual(self.inverse(
            doubles(coefficients), coefficients.size, 32, 3, 2, 0,
            integers(lengths), lengths.size, doubles(output), output.size), INVALID)
        np.testing.assert_array_equal(output, np.full(64, 321.0))

    def test_inverse_rejects_short_actual_arrays(self):
        coefficients, lengths = self.transform(np.arange(64, dtype=np.float64))
        output = np.full(64, 321.0)
        self.assertEqual(self.inverse(
            doubles(coefficients[:1].copy()), 1, 64, 3, 2, 0,
            integers(lengths), 3, doubles(output), 64), INVALID)
        self.assertEqual(self.inverse(
            doubles(coefficients), coefficients.size, 64, 3, 2, 0,
            integers(lengths[:1].copy()), 1, doubles(output), 64), INVALID)
        self.assertEqual(self.inverse(
            doubles(coefficients), coefficients.size, 64, 3, 2, 0,
            integers(lengths), 3, doubles(output[:1]), 1), INVALID)
        np.testing.assert_array_equal(output, np.full(64, 321.0))

    def test_wavelet_rejects_nonfinite_samples_and_bad_levels(self):
        output = np.full(500, 321.0)
        lengths = np.zeros(101, dtype=np.int32)
        for bad_value in (np.nan, np.inf, -np.inf):
            data = np.arange(64, dtype=np.float64)
            data[30] = bad_value
            self.assertEqual(self.forward(
                doubles(data), data.size, 3, 2, 0, doubles(output), integers(lengths)),
                INVALID)
            original = data.copy()
            self.assertEqual(self.denoise(doubles(data), data.size, 3, 2, 0, 0, 0, 0),
                             INVALID)
            np.testing.assert_array_equal(data, original)
        for level in (0, -1, 101, 2**31 - 1):
            self.assertEqual(self.forward(
                doubles(np.ones(64)), 64, 3, level, 0, doubles(output), integers(lengths)),
                INVALID)
        self.assertEqual(self.forward(
            doubles(np.ones(64)), 64, 3, 20, 0, doubles(output), integers(lengths)),
            BUFFER_ERROR)
        coefficients, valid_lengths = self.transform(np.ones(64))
        coefficients[0] = np.nan
        self.assertEqual(self.inverse(
            doubles(coefficients), coefficients.size, 64, 3, 2, 0,
            integers(valid_lengths), valid_lengths.size, doubles(output), output.size),
            INVALID)
        np.testing.assert_array_equal(output, np.full(500, 321.0))

    def test_detail_reconstruction_matches_coefficient_mask(self):
        data = np.random.default_rng(13).normal(size=257)
        for wavelet in (0, 3, 20, 35, 44):
            coefficients, lengths = self.transform(data, wavelet, 3)
            offsets = np.concatenate(([0], np.cumsum(lengths)))
            for detail in (1, 2, 3):
                masked = np.zeros_like(coefficients)
                block = 4 - detail
                masked[offsets[block]:offsets[block + 1]] = coefficients[
                    offsets[block]:offsets[block + 1]]
                expected = self.reconstruct(masked, lengths, data.size, wavelet, 3)
                actual = np.empty_like(data)
                self.assertEqual(self.restore(
                    doubles(data), data.size, wavelet, 3, detail, doubles(actual)), 0)
                np.testing.assert_allclose(actual, expected, atol=1e-12, rtol=1e-12)

    def test_wavelet_denoising_preserves_constant_signals(self):
        for method in (0, 1):
            for threshold in (0, 1):
                for extension in (0, 1):
                    for noise in (0, 1):
                        data = np.full(256, 12.5)
                        self.assertEqual(self.denoise(
                            doubles(data), data.size, 3, 3, method, threshold, extension,
                            noise), 0)
                        np.testing.assert_allclose(data, np.full(256, 12.5), atol=1e-10)

    def test_csp_whitens_and_diagonalizes_covariances(self):
        data = np.random.default_rng(187).normal(size=(6, 3, 256))
        data[3:, 0] *= 4.0
        labels = np.array([0., 0., 0., 1., 1., 1.])
        filters, values = np.empty((3, 3)), np.empty(3)
        self.assertEqual(self.csp(
            doubles(data), doubles(labels), 6, 3, 256, doubles(filters), doubles(values)), 0)
        centered = data - data.mean(axis=2, keepdims=True)
        covariance = centered @ centered.transpose(0, 2, 1) / 256
        first, second = covariance[:3].mean(axis=0), covariance[3:].mean(axis=0)
        np.testing.assert_allclose(filters @ (first + second) @ filters.T,
                                   np.eye(3), atol=1e-12, rtol=1e-12)
        np.testing.assert_allclose(filters @ first @ filters.T,
                                   np.diag(values), atol=1e-12, rtol=1e-12)

    def test_csp_rejects_invalid_labels_rank_and_samples(self):
        valid = np.random.default_rng(187).normal(size=(4, 3, 128))
        filters, values = np.full((3, 3), 321.0), np.full(3, 321.0)
        labels = np.array([0., 0., 1., 1.])
        for bad_labels in (np.zeros(4), np.ones(4), np.array([0.1, 0.9, 1.1, 1.9]),
                           np.array([0., 0., 1., np.nan])):
            self.assertEqual(self.csp(
                doubles(valid), doubles(bad_labels), 4, 3, 128,
                doubles(filters), doubles(values)), INVALID)
        rank_one = np.tile(valid[:, :1], (1, 3, 1))
        for bad_data in (np.zeros_like(valid), np.ones_like(valid), rank_one,
                         np.full_like(valid, np.nan), np.full_like(valid, np.inf)):
            self.assertEqual(self.csp(
                doubles(bad_data), doubles(labels), 4, 3, 128,
                doubles(filters), doubles(values)), INVALID)
        self.assertEqual(self.csp(
            doubles(valid), doubles(labels), 4, 3, 128, None, doubles(values)), INVALID)
        np.testing.assert_array_equal(filters, np.full((3, 3), 321.0))
        np.testing.assert_array_equal(values, np.full(3, 321.0))

    @staticmethod
    def mixture(square=False):
        generator = np.random.default_rng(2841)
        sources = np.vstack((generator.laplace(size=5000), generator.uniform(-1, 1, 5000),
                             generator.standard_t(6, size=5000)))
        mixing = np.array([[1., 0.4, -0.2], [-0.6, 1.3, 0.7], [0.3, -0.7, 1.2]])
        if not square:
            sources, mixing = sources[:2], mixing[:, :2]
        return np.ascontiguousarray(mixing @ sources + np.array([[10.], [-5.], [2.]]))

    def run_ica(self, data, components=2, iterations=1000, tolerance=1e-4, seed=42):
        rows, columns = data.shape
        outputs = (np.full((components, components), 321.0),
                   np.full((components, rows), 321.0),
                   np.full((rows, components), 321.0),
                   np.full((components, columns), 321.0))
        code = self.ica(doubles(data), rows, columns, components,
                        *(doubles(output) for output in outputs),
                        iterations, tolerance, seed)
        return code, outputs

    def test_ica_rectangular_and_square_reconstruction(self):
        for square in (False, True):
            data = self.mixture(square)
            code, (weights, whitening, mixing, sources) = self.run_ica(
                data, 3 if square else 2)
            self.assertEqual(code, 0)
            centered = data - data.mean(axis=1, keepdims=True)
            np.testing.assert_allclose(mixing @ sources, centered, atol=1e-10, rtol=1e-10)
            np.testing.assert_allclose(mixing, np.linalg.pinv(weights @ whitening),
                                       atol=1e-10, rtol=1e-10)
            np.testing.assert_allclose(sources, weights @ whitening @ centered,
                                       atol=1e-10, rtol=1e-10)

    def test_ica_seed_is_repeatable_and_scaling_stays_finite(self):
        data = self.mixture()
        first_code, first = self.run_ica(data)
        second_code, second = self.run_ica(data)
        self.assertEqual(first_code, 0)
        self.assertEqual(second_code, 0)
        for left, right in zip(first, second):
            np.testing.assert_array_equal(left, right)
        for scale in (1e-100, 1e100):
            code, (_, _, mixing, sources) = self.run_ica(data * scale)
            self.assertEqual(code, 0)
            np.testing.assert_allclose(mixing @ sources / scale,
                                       data - data.mean(axis=1, keepdims=True),
                                       atol=1e-10, rtol=1e-10)

    def test_ica_rejects_bad_rank_nonfinite_and_nonconvergence(self):
        data = self.mixture()
        for invalid in (np.zeros((3, 100)), np.ones((3, 100)),
                        np.tile(np.arange(100.), (3, 1)),
                        np.full((3, 100), np.nan), np.full((3, 100), np.inf)):
            code, outputs = self.run_ica(invalid)
            self.assertEqual(code, INVALID)
            for output in outputs:
                self.assertTrue(np.all(output == 321.0))
        code, outputs = self.run_ica(data, iterations=1, tolerance=1e-14)
        self.assertEqual(code, GENERAL_ERROR)
        for output in outputs:
            self.assertTrue(np.all(output == 321.0))
        for options in ({'iterations': 0}, {'tolerance': 0.0}, {'tolerance': np.nan},
                        {'tolerance': 1.0}, {'seed': -2}):
            self.assertEqual(self.run_ica(data, **options)[0], INVALID)


if __name__ == '__main__':
    unittest.main()
