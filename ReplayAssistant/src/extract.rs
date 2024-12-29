use std::process::Command;
use std::fs;
use std::io::{self};
use serde_json::{json, Value};
use std::path::Path;
use std::io::{Error, ErrorKind};


/// Parses a Rocket League replay file using the `rattletrap` CLI and writes the result to a CSV file.
pub fn extract_replay(input: &str) -> io::Result<String> {
    let rattletrap_name = "rattletrap";
    let rattletrap_path = Path::new(rattletrap_name);

    // Check if rattletrap exists in PATH
    let rattletrap_exists = Command::new("which")
        .arg(rattletrap_name)
        .output()
        .map(|output| output.status.success())
        .unwrap_or(false);

    // If not, download it locally
    if !rattletrap_exists && !rattletrap_path.exists() {
        println!("Rattletrap not found. Downloading...");
        let download_url = "https://github.com/tfausak/rattletrap/releases/download/14.1.0/rattletrap-14.1.0-linux-x64.tar.gz";
        let tar_file = "rattletrap-14.1.0-linux-x64.tar.gz";

        // Download the tar.gz
        let wget_status = Command::new("wget")
            .arg("-q")
            .arg(download_url)
            .status()?;

        if !wget_status.success() {
            return Err(Error::new(
                ErrorKind::Other,
                "Failed to download Rattletrap.",
            ));
        }

        // Extract the tar.gz
        let tar_status = Command::new("tar")
            .args(&["-xzf", tar_file])
            .status()?;

        if !tar_status.success() {
            return Err(Error::new(
                ErrorKind::Other,
                "Failed to extract Rattletrap.",
            ));
        }

        // Make the binary executable
        let chmod_status = Command::new("chmod")
            .args(&["+x", rattletrap_name])
            .status()?;

        if !chmod_status.success() {
            return Err(Error::new(
                ErrorKind::Other,
                "Failed to set executable permissions for Rattletrap.",
            ));
        }
        println!("Rattletrap downloaded.");

        // Cleanup tar.gz file
        fs::remove_file(tar_file)?;
    }

    // Ensure the binary is executable
    if !rattletrap_path.exists() && !rattletrap_exists {
        return Err(Error::new(
            ErrorKind::NotFound,
            "Rattletrap binary not found or failed to download.",
        ));
    }

    let filename = Path::new(&input)
        .file_name()
        .unwrap_or_else(|| Path::new(&input).as_os_str())
        .to_str()
        .unwrap_or(input);

    let json_output = format!("./output/{}.json", filename);

    // Run the rattletrap command
    let output_status = Command::new(if rattletrap_exists {
        rattletrap_name
    } else {
        "./rattletrap"
    })
    .arg("--compact")
    .arg("--input")
    .arg(input)
    .arg("--output")
    .arg(&json_output)
    .output();

    match output_status {
        Ok(output) => {
            if !output.status.success() {
                eprintln!(
                    "Failed to extract replay data. Error: {}",
                    String::from_utf8_lossy(&output.stderr)
                );
                return Err(io::Error::new(io::ErrorKind::Other, "Rattletrap failed"));
            }
        }
        Err(e) => {
            eprintln!("Failed to execute Rattletrap: {}", e);
            return Err(e);
        }
    }

    let json_data = fs::read_to_string(&json_output)?;
    let parsed_data: serde_json::Value = serde_json::from_str(&json_data)?;
    let match_guid = find_property(
        parsed_data.pointer("/header/body/properties/elements").unwrap_or(&Value::Null),
        "MatchGuid",
    )
    .and_then(|v| v.as_str().map(|s| s.to_string()))
    .unwrap_or_else(|| "unknown_match_guid".to_string());

    match parse_replay(parsed_data, match_guid.clone()) {
        Ok(_) => println!("Replay data parsed successfully."),
        Err(e) => eprintln!("Error parsing replay: {}", e),
    };

    fs::remove_file(&json_output).expect("Failed to delete output file");

    Ok(match_guid)
}


fn parse_replay(data: Value, match_guid: String) -> Result<(), Box<dyn std::error::Error>> {

    let output_dir = "output";
    fs::create_dir_all(output_dir)?;

    let header_map = parse_header(&data);
    save_to_file(&header_map, output_dir, &match_guid, "header")?;

    let goals = parse_goals(
        data.pointer("/header/body/properties/elements")
            .unwrap_or(&Value::Array(vec![])),
    );
    save_to_file(&Value::Array(goals), output_dir, &match_guid, "goals")?;


    let player_stats = parse_player_stats(
        data.pointer("/header/body/properties/elements")
            .unwrap_or(&Value::Array(vec![])),
    );
    save_to_file(&Value::Array(player_stats), output_dir, &match_guid, "player_stats")?;


    let highlights = parse_highlights(
        data.pointer("/header/body/properties/elements")
            .unwrap_or(&Value::Array(vec![])),
    );
    save_to_file(&Value::Array(highlights), output_dir, &match_guid, "highlights")?;

    let frames = parse_frames(
        data.pointer("/content/body/frames")
            .unwrap_or(&Value::Array(vec![])),
    );
    save_to_file(&Value::Array(frames), output_dir, &match_guid, "replay.frames")?;

    Ok(())
}

/// Parses the header into a structured JSON object.
fn parse_header(data: &Value) -> Value {
    let properties = data.pointer("/header/body/properties/elements").unwrap_or(&Value::Null);

    json!({
        "engine_version": data.pointer("/header/body/engine_version").unwrap_or(&Value::Null),
        "team_size": find_property(properties, "TeamSize").unwrap_or(Value::Null),
        "team_1_score": find_property(properties, "Team1Score").unwrap_or(Value::Null),
        "primary_player_team": find_property(properties, "PrimaryPlayerTeam").unwrap_or(Value::Null),
        "licensee_version": data.pointer("/header/body/licensee_version").unwrap_or(&Value::Null),
        "unfair_team_size": find_property(properties, "UnfairTeamSize").unwrap_or(Value::Null),
        "patch_version": data.pointer("/header/body/patch_version").unwrap_or(&Value::Null),
        "team_0_score": find_property(properties, "Team0Score").unwrap_or(Value::Null),
    })
}

fn parse_goals(elements: &Value) -> Vec<Value> {

    // Create a longer-lived empty vector
    let empty_vec = vec![];
    let goals_property = elements
        .as_array()
        .unwrap_or(&empty_vec) // Use the longer-lived empty vector here
        .iter()
        .find(|item| item.get(0).and_then(|v| v.as_str()) == Some("Goals"));

    let goals_array = goals_property
        .and_then(|item| item.get(1)) // Access the second element in the "Goals" property
        .and_then(|details| details.get("value")) // Access the "value" field
        .and_then(|value| value.get("array")); // Access the "array" field

    let parsed_goals = goals_array
        .and_then(|array| array.as_array()) // Ensure it's an array
        .map(|array| {
            array
                .iter()
                .filter_map(|goal| {
                    goal.get("elements")
                        .and_then(|elements| elements.as_array())
                        .map(|fields| {
                            let mut goal_map = serde_json::Map::new();
                            for field in fields {
                                if let Some(key) = field.get(0).and_then(|v| v.as_str()) {
                                    if let Some(value) = field.get(1).and_then(|v| v.get("value")) {
                                        if let Some(int_value) = value.get("int") {
                                            goal_map.insert(key.to_string(), int_value.clone());
                                        } else if let Some(str_value) = value.get("str") {
                                            goal_map.insert(key.to_string(), str_value.clone());
                                        }
                                    }
                                }
                            }
                            Value::Object(goal_map)
                        })
                })
                .collect()
        })
        .unwrap_or_default(); // Default to an empty vector if parsing fails

    parsed_goals
}

fn parse_player_stats(elements: &Value) -> Vec<Value> {
    let empty_vec = vec![]; // Create a longer-lived empty vector
    let player_stats_property = elements
        .as_array()
        .unwrap_or(&empty_vec) // Use the longer-lived empty vector
        .iter()
        .find(|item| item.get(0).and_then(|v| v.as_str()) == Some("PlayerStats"));

    let player_stats_array = player_stats_property
        .and_then(|item| item.get(1))
        .and_then(|details| details.get("value"))
        .and_then(|value| value.get("array"));

    player_stats_array
        .and_then(|array| array.as_array())
        .map(|array| {
            array
                .iter()
                .map(|entry| {
                    let mut map = serde_json::Map::new();
                    if let Some(elements) = entry.get("elements").and_then(|v| v.as_array()) {
                        for key in &["Name", "Platform", "Team", "Score", "Goals", "Assists", "Saves", "Shots", "bBot"] {
                            if let Some(value) = elements.iter().find_map(|field| {
                                field.get(0).and_then(|k| {
                                    if k.as_str() == Some(key) {
                                        field.get(1).and_then(|v| v.get("value"))
                                    } else {
                                        None
                                    }
                                })
                            }) {
                                map.insert((*key).to_string(), value.clone());
                            }
                        }
                    }
                    Value::Object(map)
                })
                .collect()
        })
        .unwrap_or_default()
}

fn parse_highlights(elements: &Value) -> Vec<Value> {
    let empty_vec = vec![]; // Create a longer-lived empty vector
    let highlights_property = elements
        .as_array()
        .unwrap_or(&empty_vec) // Use the longer-lived empty vector
        .iter()
        .find(|item| item.get(0).and_then(|v| v.as_str()) == Some("HighLights"));

    let highlights_array = highlights_property
        .and_then(|item| item.get(1))
        .and_then(|details| details.get("value"))
        .and_then(|value| value.get("array"));

    highlights_array
        .and_then(|array| array.as_array())
        .map(|array| {
            array
                .iter()
                .map(|entry| {
                    let mut map = serde_json::Map::new();
                    if let Some(elements) = entry.get("elements").and_then(|v| v.as_array()) {
                        for key in &["frame", "CarName", "BallName", "GoalActorName"] {
                            if let Some(value) = elements.iter().find_map(|field| {
                                field.get(0).and_then(|k| {
                                    if k.as_str() == Some(key) {
                                        field.get(1).and_then(|v| v.get("value"))
                                    } else {
                                        None
                                    }
                                })
                            }) {
                                map.insert((*key).to_string(), value.clone());
                            }
                        }
                    }
                    Value::Object(map)
                })
                .collect()
        })
        .unwrap_or_default()
}

/// Saves frames to a JSON file and returns them.
pub fn parse_frames(frames: &Value) -> Vec<Value> {
    // Return the frames as a Vec<Value>
    frames.as_array().cloned().unwrap_or_default()
}



fn find_property(array: &Value, key: &str) -> Option<Value> {
    array
        .as_array()
        .and_then(|elements| {
            elements.iter().find_map(|e| {
                if e.get(0)?.as_str()? == key {
                    let value = e.get(1)?.get("value")?;
                    value.get("int").or_else(|| value.get("str")).cloned()
                } else {
                    None
                }
            })
        })
}

/// Helper function to save a JSON object to a file.
fn save_to_file(
    data: &Value,
    output_dir: &str,
    match_guid: &str,
    section: &str,
) -> Result<(), Box<dyn std::error::Error>> {
    let file_path = format!("{}/{}.{}.json", output_dir, match_guid, section);
    fs::write(&file_path, serde_json::to_string_pretty(data)?)?;
    println!("Saved: {}", file_path);
    Ok(())
}

