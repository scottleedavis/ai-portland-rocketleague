#[cfg(test)]
mod main_tests {
    use std::process::Command;

    #[test]
    fn test_no_arguments() {
        let output = Command::new("cargo")
            .args(&["run", "--"])
            .output()
            .expect("Failed to execute process");

        let stdout = String::from_utf8(output.stdout).unwrap();
        assert!(stdout.contains("Usage: rattlebrain <command> [options]"));
    }

    #[test]
    fn test_extract_command() {
        let output = Command::new("cargo")
            .args(&["run", "--", "extract", "input.replay", "output.csv"])
            .output()
            .expect("Failed to execute process");

        let stdout = String::from_utf8(output.stdout).unwrap();
        assert!(stdout.contains("Extracting replay data..."));
    }

    #[test]
    fn test_unknown_command() {
        let output = Command::new("cargo")
            .args(&["run", "--", "unknown"])
            .output()
            .expect("Failed to execute process");

        let stdout = String::from_utf8(output.stdout).unwrap();
        assert!(stdout.contains("Unknown command: unknown"));
    }
}
