use rattlebrain::extract;
use std::process::{self, Command};
use std::{fs, path::Path};
use jsonschema::{JSONSchema, Draft};
use serde_json::Value;
use std::fs::File;
use std::io::Write;
use tokio::sync::OnceCell;

static SETUP_ONCE: OnceCell<()> = OnceCell::const_new();

async fn ensure_rattletrap() -> &'static str {
    SETUP_ONCE
        .get_or_init(|| async {
            let rattletrap_name = "rattletrap";

            if Command::new("which")
                .arg(rattletrap_name)
                .output()
                .map(|output| output.status.success())
                .unwrap_or(false)
            {
                return; // Rattletrap is available, no need to download
            }

            println!("Rattletrap not found. Downloading...");
            let download_url = "https://github.com/tfausak/rattletrap/releases/download/14.1.0/rattletrap-14.1.0-linux-x64.tar.gz";
            let tar_file = "rattletrap-14.1.0-linux-x64.tar.gz";

            // Download the tar.gz
            let wget_status = Command::new("wget")
                .arg("-q")
                .arg(download_url)
                .status()
                .expect("Failed to execute wget");

            if !wget_status.success() {
                eprintln!("Error downloading Rattletrap");
                process::exit(1);
            }

            // Extract the tar.gz
            let tar_status = Command::new("tar")
                .args(&["-xzf", tar_file])
                .status()
                .expect("Failed to execute tar");

            if !tar_status.success() {
                eprintln!("Error extracting Rattletrap");
                process::exit(1);
            }

            // Make the binary executable
            let chmod_status = Command::new("chmod")
                .args(&["+x", rattletrap_name])
                .status()
                .expect("Failed to execute chmod");

            if !chmod_status.success() {
                eprintln!("Error making Rattletrap executable");
                process::exit(1);
            }

            println!("Rattletrap downloaded and set up.");

            // Cleanup tar.gz file
            fs::remove_file(tar_file).expect("Failed to remove tar file");
        })
        .await;

    "./rattletrap"
}

#[tokio::test]
async fn test_extract_replay_json_schema_validation() {
    let rattletrap_path = ensure_rattletrap().await;
    let input_replay = "./tests/valid.replay";
    let output_json = "./output/output_schema_test.json";

    // Run `rattletrap` to extract replay data to JSON
    let output_status = Command::new(rattletrap_path)
        .arg("--compact")
        .arg("--input")
        .arg(input_replay)
        .arg("--output")
        .arg(output_json)
        .output()
        .expect("Failed to execute Rattletrap");

    assert!(
        output_status.status.success(),
        "Rattletrap failed with error: {}",
        String::from_utf8_lossy(&output_status.stderr)
    );

    // Load the JSON schema
    let schema_str = fs::read_to_string("replay_schema.json")
        .expect("Failed to read replay_schema.json");
    let schema_json: Value = serde_json::from_str(&schema_str)
        .expect("Invalid JSON schema");

    // Compile the JSON schema
    let compiled_schema = JSONSchema::options()
        .with_draft(Draft::Draft7)
        .compile(&schema_json)
        .expect("Invalid JSON schema");

    // Load the extracted JSON data
    let data_str = fs::read_to_string(output_json)
        .expect("Failed to read output.json");
    let data_json: Value = serde_json::from_str(&data_str)
        .expect("Invalid JSON data");

    // Validate the JSON data against the schema
    if let Err(errors) = compiled_schema.validate(&data_json) {
        for error in errors {
            println!("Validation error: {}", error);
        }
        panic!("JSON data does not conform to the schema");
    }

    // Clean up
    fs::remove_file(output_json).expect("Failed to remove output_schema_test.json");
}

#[tokio::test]
async fn test_extract_invalid_replay() {
    let _ = ensure_rattletrap().await;
    let invalid_replay_path = "tests/invalid.replay";

    // Create a dummy invalid replay file
    let mut file = File::create(invalid_replay_path).unwrap();
    file.write_all(b"invalid replay data").unwrap();

    // Call `extract_replay`
    let result = extract::extract_replay(invalid_replay_path);
    assert!(result.is_err(), "Extract replay should fail for invalid data");

    // Clean up
    fs::remove_file(invalid_replay_path).unwrap();
}

#[tokio::test]
async fn test_extract_replay_creates_files() {
    let _ = ensure_rattletrap().await;
    let input_replay = "./tests/valid.replay";
    let output_dir = "./output";
    let match_guid = "383F0B0411EFAC27082CAFA884251EFF";
    let frames_file = format!("{}/{}.replay.frames.json", output_dir, match_guid);

    // Call `extract_replay`
    match extract::extract_replay(input_replay) {
        Ok(_) => println!("Extract command completed successfully."),
        Err(e) => panic!("Error extracting replay: {}", e),
    }

    // Ensure the frames file exists
    assert!(
        Path::new(&frames_file).exists(),
        "Frames file does not exist"
    );

    // Cleanup: Remove all output files after the test
    if Path::new(&frames_file).exists() {
        fs::remove_file(&frames_file).expect("Failed to delete frames file");
    }
}
