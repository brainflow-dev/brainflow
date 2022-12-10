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
    pub(crate) const VERSION_PATTERN: &str = r"^\d+\.\d+\.\d+$";
}