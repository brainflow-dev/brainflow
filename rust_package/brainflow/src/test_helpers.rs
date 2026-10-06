#[cfg(test)]
pub(crate) mod assertions {
    use regex::Regex;

    pub(crate) fn assert_regex_matches(regex: &str, value: &str) {
        let compiled_regex = Regex::new(regex).unwrap();
        assert!(compiled_regex.is_match(value), "Expected to match {}, got {}", regex, value);
    }
}

#[cfg(test)]
pub mod consts {
    // Releases use a version number; CI builds embed the full Git commit ID.
    pub(crate) const VERSION_PATTERN: &str = r"^(\d+\.\d+\.\d+|[0-9a-f]{40})$";

    #[test]
    fn version_pattern_accepts_release_and_commit_versions() {
        let pattern = regex::Regex::new(VERSION_PATTERN).unwrap();
        for version in ["0.0.1", "5.23.0", "e4173893c5a910b1ecb0f5eafde8ccc6b37bc6bf"] {
            assert!(pattern.is_match(version), "Rejected build version: {}", version);
        }
        for version in ["", "5.23", "not-a-version", "e4173893", "e4173893c5a910b1ecb0f5eafde8ccc6b37bc6bf0"] {
            assert!(!pattern.is_match(version), "Accepted invalid version: {}", version);
        }
    }
}
