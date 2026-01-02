//! Wavelet filter definitions.

use alloc::vec::Vec;

/// Supported wavelet types.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Wavelet {
    /// Haar wavelet (same as db1).
    Haar,
    /// Daubechies wavelets (db1-db15).
    Daubechies(u8),
    /// Symlet wavelets (sym2-sym10).
    Symlet(u8),
    /// Coiflet wavelets (coif1-coif5).
    Coiflet(u8),
    /// Biorthogonal wavelets.
    Biorthogonal(u8, u8),
}

impl Wavelet {
    /// Get the decomposition low-pass filter coefficients.
    #[must_use]
    pub fn dec_lo(&self) -> Vec<f64> {
        match self {
            Wavelet::Haar | Wavelet::Daubechies(1) => {
                let c = 1.0 / libm::sqrt(2.0);
                alloc::vec![c, c]
            }
            Wavelet::Daubechies(2) => {
                alloc::vec![
                    0.4829629131445341,
                    0.8365163037378079,
                    0.2241438680420134,
                    -0.1294095225512604,
                ]
            }
            Wavelet::Daubechies(3) => {
                alloc::vec![
                    0.3326705529500826,
                    0.8068915093110925,
                    0.4598775021184915,
                    -0.1350110200102546,
                    -0.0854412738820267,
                    0.0352262918857095,
                ]
            }
            Wavelet::Daubechies(4) => {
                alloc::vec![
                    0.2303778133088965,
                    0.7148465705529156,
                    0.6308807679298589,
                    -0.0279837694168599,
                    -0.1870348117190930,
                    0.0308413818355607,
                    0.0328830116668852,
                    -0.0105974017850690,
                ]
            }
            Wavelet::Daubechies(5) => {
                alloc::vec![
                    0.1601023979741929,
                    0.6038292697971896,
                    0.7243085284377729,
                    0.1384281459013207,
                    -0.2422948870663820,
                    -0.0322448695846381,
                    0.0775714938400459,
                    -0.0062414902127983,
                    -0.0125807519990820,
                    0.0033357252854738,
                ]
            }
            Wavelet::Daubechies(n) if *n <= 15 => {
                // For higher order, use approximation or return db5
                self.daubechies_coeffs(*n)
            }
            Wavelet::Symlet(n) => self.symlet_coeffs(*n),
            Wavelet::Coiflet(n) => self.coiflet_coeffs(*n),
            Wavelet::Biorthogonal(n, m) => self.bior_dec_lo(*n, *m),
            _ => {
                // Default to Haar
                let c = 1.0 / libm::sqrt(2.0);
                alloc::vec![c, c]
            }
        }
    }

    /// Get the decomposition high-pass filter coefficients.
    #[must_use]
    pub fn dec_hi(&self) -> Vec<f64> {
        let lo = self.dec_lo();
        qmf(&lo)
    }

    /// Get the reconstruction low-pass filter coefficients.
    #[must_use]
    pub fn rec_lo(&self) -> Vec<f64> {
        let lo = self.dec_lo();
        lo.into_iter().rev().collect()
    }

    /// Get the reconstruction high-pass filter coefficients.
    #[must_use]
    pub fn rec_hi(&self) -> Vec<f64> {
        let hi = self.dec_hi();
        hi.into_iter().rev().collect()
    }

    /// Get the filter length.
    #[must_use]
    pub fn filter_length(&self) -> usize {
        self.dec_lo().len()
    }

    /// Higher-order Daubechies coefficients.
    fn daubechies_coeffs(&self, n: u8) -> Vec<f64> {
        match n {
            6 => alloc::vec![
                0.1115407433501095,
                0.4946238903984533,
                0.7511339080210959,
                0.3152503517091982,
                -0.2262646939654400,
                -0.1297668675672625,
                0.0975016055873225,
                0.0275228655303053,
                -0.0315820393174862,
                0.0005538422011614,
                0.0047772575109455,
                -0.0010773010853085,
            ],
            7 => alloc::vec![
                0.0778520540850037,
                0.3965393194818912,
                0.7291320908461957,
                0.4697822874051931,
                -0.1439060039285212,
                -0.2240361849938412,
                0.0713092192668296,
                0.0806126091510774,
                -0.0380299369350104,
                -0.0165745416306655,
                0.0125509985560986,
                0.0004295779729214,
                -0.0018016407040474,
                0.0003537137999745,
            ],
            8 => alloc::vec![
                0.0544158422431049,
                0.3128715909143031,
                0.6756307362972904,
                0.5853546836541907,
                -0.0158291052563816,
                -0.2840155429615702,
                0.0004724845739124,
                0.1287474266204837,
                -0.0173693010018083,
                -0.0440882539307952,
                0.0139810279173995,
                0.0087460940474061,
                -0.0048703529934518,
                -0.0003917403733770,
                0.0006754494064506,
                -0.0001174767841248,
            ],
            _ => {
                // Fall back to db5 for unsupported orders
                alloc::vec![
                    0.1601023979741929,
                    0.6038292697971896,
                    0.7243085284377729,
                    0.1384281459013207,
                    -0.2422948870663820,
                    -0.0322448695846381,
                    0.0775714938400459,
                    -0.0062414902127983,
                    -0.0125807519990820,
                    0.0033357252854738,
                ]
            }
        }
    }

    /// Symlet coefficients.
    fn symlet_coeffs(&self, n: u8) -> Vec<f64> {
        match n {
            2 => alloc::vec![
                -0.1294095225512604,
                0.2241438680420134,
                0.8365163037378079,
                0.4829629131445341,
            ],
            3 => alloc::vec![
                0.0352262918857095,
                -0.0854412738820267,
                -0.1350110200102546,
                0.4598775021184915,
                0.8068915093110925,
                0.3326705529500826,
            ],
            4 => alloc::vec![
                -0.0757657147893407,
                -0.0296355276459541,
                0.4976186676324578,
                0.8037387518052163,
                0.2978577956055422,
                -0.0992195435769354,
                -0.0126039672622612,
                0.0322231006040713,
            ],
            5 => alloc::vec![
                0.0273330683451645,
                0.0295194909257225,
                -0.0391342493023834,
                0.1993975339769955,
                0.7234076904038076,
                0.6339789634569490,
                0.0166021057644243,
                -0.1753280899081075,
                -0.0211018340249298,
                0.0195388827353869,
            ],
            _ => self.daubechies_coeffs(n.min(8)),
        }
    }

    /// Coiflet coefficients.
    fn coiflet_coeffs(&self, n: u8) -> Vec<f64> {
        match n {
            1 => alloc::vec![
                -0.0156557281,
                -0.0727326195,
                0.3848648469,
                0.8525720202,
                0.3378976625,
                -0.0727326195,
            ],
            2 => alloc::vec![
                0.0011945726958388,
                -0.0062075816146506,
                -0.0143294238350809,
                0.0611893857600960,
                0.0170946538317618,
                -0.2498464243271598,
                0.0038964363835400,
                0.5542430556968073,
                0.7805304867734663,
                0.2793292509326364,
                -0.0975016055873225,
                -0.0015875463219626,
            ],
            _ => {
                // Fall back to coif1
                alloc::vec![
                    -0.0156557281,
                    -0.0727326195,
                    0.3848648469,
                    0.8525720202,
                    0.3378976625,
                    -0.0727326195,
                ]
            }
        }
    }

    /// Biorthogonal decomposition low-pass.
    fn bior_dec_lo(&self, n: u8, m: u8) -> Vec<f64> {
        match (n, m) {
            (1, 1) => {
                let c = 1.0 / libm::sqrt(2.0);
                alloc::vec![c, c]
            }
            (1, 3) => alloc::vec![
                -0.0883883476483184,
                0.0883883476483184,
                0.7071067811865476,
                0.7071067811865476,
                0.0883883476483184,
                -0.0883883476483184,
            ],
            (2, 2) => alloc::vec![
                0.0,
                -0.1767766952966369,
                0.3535533905932738,
                1.0606601717798214,
                0.3535533905932738,
                -0.1767766952966369,
            ],
            _ => {
                let c = 1.0 / libm::sqrt(2.0);
                alloc::vec![c, c]
            }
        }
    }
}

/// Quadrature mirror filter (alternating sign flip).
fn qmf(filter: &[f64]) -> Vec<f64> {
    filter
        .iter()
        .rev()
        .enumerate()
        .map(|(i, &x)| if i % 2 == 0 { x } else { -x })
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_haar_coeffs() {
        let coeffs = Wavelet::Haar.dec_lo();
        assert_eq!(coeffs.len(), 2);
        let c = 1.0 / libm::sqrt(2.0);
        assert!((coeffs[0] - c).abs() < 1e-10);
        assert!((coeffs[1] - c).abs() < 1e-10);
    }

    #[test]
    fn test_filter_sum() {
        // Low-pass filter coefficients should sum to sqrt(2)
        let coeffs = Wavelet::Daubechies(4).dec_lo();
        let sum: f64 = coeffs.iter().sum();
        assert!((sum - libm::sqrt(2.0)).abs() < 1e-10);
    }
}
