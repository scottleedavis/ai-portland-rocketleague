mod extract;
mod convert;
mod ai;

use std::env;
use std::fs;
use std::process;
use serde_json::Value;


use tokio;

#[tokio::main]
async fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() < 2 {
        println!("Usage: ReplayAssistant <command> [prompt]");
        println!("Commands:");
        println!(" prepare <path/some.replay> - Prepare replay data for ai");
        println!(" ai <assistant_id> [query] - Query AI for replay insights. (with default prompt)");
        return;
    }

    let command = &args[1];

    match command.as_str() {
        "prepare" => {
            if args.len() < 3 {
                println!("Usage: ReplayAssistant prepare <file.replay>");
                return;
            }
            let input = &args[2];

            println!("Starting prepare...");

            println!("Extracting replay data: {}", input);
            let match_guid = match extract::extract_replay(input) {
                Ok(match_guid) => {
                    println!("Extraction successful. Match GUID: {}", match_guid);
                    match_guid // Store the match_guid for Step 2
                }
                Err(e) => {
                    eprintln!("Error during extraction: {}", e);
                    process::exit(1);
                }
            };

            let replay_file = format!("./output/{}.replay.frames.json", match_guid);
            let player_statistics_file = format!("./output/{}.player_stats.json", match_guid);
            let goals_file = format!("./output/{}.goals.json", match_guid);
            let highlights_file = format!("./output/{}.highlights.json", match_guid);

            for file in [replay_file, player_statistics_file, goals_file, highlights_file].iter() {
                process_conversion(file);
            }
            delete_json_files("./output");

            println!("Creating assistant...");
            match ai::create_assistant(&match_guid).await {
                Ok(assistant_id) => println!("{}",assistant_id),
                Err(e) => eprintln!("Error querying AI: {}", e),
            }

        }
        "ai" => {
            // if args.len() < 3 {
            //     println!("Usage: ReplayAssistant ai <match_guid> [prompt]");
            //     return;
            // }

            // let match_guid = &args[2];
            // // Set focus to "all" if not provided, otherwise pass the provided value
            // let focus = if args.len() > 3 { &args[3] } else { "all" };

            // println!("Querying AI for insights...");
            // match query::query_ai(match_guid, focus).await {
            //     Ok(response) => {
            //         let feedback_file_path = format!("./output/{}.feedback.md", match_guid);

            //         // Save the AI response to the feedback file
            //         match fs::write(&feedback_file_path, &response) {
            //             Ok(_) => println!("Saved AI feedback: {}",feedback_file_path),
            //             Err(e) => eprintln!("Failed to save AI feedback: {}", e),
            //         }
            //     }
            //     Err(e) => eprintln!("Error querying AI: {}", e),
            // }
        }
        _ => {
            println!("Unknown command: {}", command);
            println!("Usage: ReplayAssistant <command> [options]");
        }
    }
}

fn process_conversion(file_path: &str) {
    println!("Converting replay data to CSV: {}", file_path);

    let file_content = match fs::read_to_string(file_path) {
        Ok(content) => content,
        Err(e) => {
            eprintln!("Error reading input file: {}", e);
            process::exit(1);
        }
    };

    let json_data: Value = match serde_json::from_str(&file_content) {
        Ok(data) => data,
        Err(e) => {
            eprintln!("Error parsing JSON: {}", e);
            process::exit(1);
        }
    };

    if let Err(e) = convert::convert_replay(json_data, file_path) {
        eprintln!("Error during conversion: {}", e);
        process::exit(1);
    }

    println!("Conversion completed successfully for file: {}", file_path);
}

fn delete_json_files(output_dir: &str) {
    match fs::read_dir(output_dir) {
        Ok(entries) => {
            for entry in entries {
                if let Ok(entry) = entry {
                    let path = entry.path();
                    if path.extension().and_then(|ext| ext.to_str()) == Some("json") {
                        if let Err(e) = fs::remove_file(&path) {
                            eprintln!("Failed to delete file {}: {}", path.display(), e);
                        } else {
                            println!("Deleted file: {}", path.display());
                        }
                    }
                }
            }
        }
        Err(e) => eprintln!("Failed to read directory {}: {}", output_dir, e),
    }
}