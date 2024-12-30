mod extract;
mod convert;
mod query;
mod ai;

use std::env;
use std::fs;
use std::process;
use serde_json::Value;
use std::io::Write;

use tokio;

#[tokio::main]
async fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() < 2 {
        println!("Usage: ReplayAssistant <command> [options]");
        println!("Commands:");
        println!(" analysis <path/some.replay> - Analyze replay data. (runs extract->convert->plot->query)");
        println!(" query <match_guid> [focus] - Query AI for replay insights.");
        println!(" extract <path/some.replay> - Extract replay data to CSV.");
        println!(" convert <path/some.replay.json> - Convert replay JSON to processed data.");
        return;
    }

    let command = &args[1];

    match command.as_str() {
        "extract" => {
            if args.len() < 3 {
                println!("Usage: ReplayAssistant extract <input>");
                return;
            }
            let input = &args[2];
            println!("Extracting replay data...");
            match extract::extract_replay(input) {
                Ok(match_guid) => {
                    println!("Extract command completed successfully.");
                    println!("Match GUID: {}", match_guid);
                }
                Err(e) => eprintln!("Error extracting replay: {}", e),
            }
        }
        "convert" => {
            if args.len() < 3 {
                println!("Usage: ReplayAssistant convert <input>");
                return;
            }
            let input = &args[2];
            println!("Converting replay data...");

            // Read the input file
            let file_content = match fs::read_to_string(input) {
                Ok(content) => content,
                Err(e) => {
                    eprintln!("Error reading input file: {}", e);
                    process::exit(1);
                }
            };

            // Parse the JSON content
            let json_data: Value = match serde_json::from_str(&file_content) {
                Ok(data) => data,
                Err(e) => {
                    eprintln!("Error parsing JSON: {}", e);
                    process::exit(1);
                }
            };

            // Convert the replay
            match convert::convert_replay(json_data,input) {
                Ok(_) => println!("Convert command completed successfully."),
                Err(e) => eprintln!("Error converting replay: {}", e),
            }
        }
        "analysis" => {
            if args.len() < 3 {
                println!("Usage: ReplayAssistant analysis <file.replay>");
                return;
            }
            let input = &args[2];

            println!("Starting analysis...");

            // Step 1: Extract
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

            // Step 2: Convert 
            let replay_file = format!("./output/{}.replay.frames.json", match_guid);
            let player_statistics_file = format!("./output/{}.player_stats.json", match_guid);
            let goals_file = format!("./output/{}.goals.json", match_guid);
            let highlights_file = format!("./output/{}.highlights.json", match_guid);

            // PStep 3: Process each file
            for file in [replay_file, player_statistics_file, goals_file, highlights_file].iter() {
                process_conversion(file);
            }
            delete_json_files("./output");


            // Step 4: AI
            let focus =  "all".to_string();

            println!("Querying AI for insights...");
            match query::query_ai(&match_guid, &focus).await {
                Ok(response) => {
                    let feedback_file_path = format!("./output/{}.feedback.md", match_guid);

                    // Save the AI response to the feedback file
                    match fs::write(&feedback_file_path, &response) {
                        Ok(_) => {
                            println!("AI feedback saved to: {}", feedback_file_path);

                            // Append image links to the feedback file
                            let image_pattern = format!("./output/{}*.png", match_guid);
                            let image_paths = match glob::glob(&image_pattern) {
                                Ok(paths) => paths.filter_map(Result::ok).collect::<Vec<_>>(),
                                Err(e) => {
                                    eprintln!("Error finding images: {}", e);
                                    Vec::new()
                                }
                            };

                            if !image_paths.is_empty() {
                                let mut image_markdown = String::new();
                                for image_path in image_paths {
                                    let image_file_name = image_path.file_name().unwrap_or_default().to_string_lossy();
                                    image_markdown.push_str(&format!(
                                        "![{}]({})\n",
                                        match_guid,
                                        image_file_name
                                    ));
                                }

                            // Open the file in append mode and add the image markdown
                            let mut feedback_file = match fs::OpenOptions::new()
                                .append(true)
                                .open(&feedback_file_path)
                            {
                                Ok(file) => file,
                                Err(e) => {
                                    eprintln!("Failed to open feedback file for appending: {}", e);
                                    return;
                                }
                            };

                            if let Err(e) = feedback_file.write_all(image_markdown.as_bytes()) {
                                eprintln!("Failed to append images to feedback: {}", e);
                            } else {
                                println!("Images appended to feedback file.");
                            }
                            }
                        }
                        Err(e) => eprintln!("Failed to save AI feedback: {}", e),
                    }
                }
                Err(e) => eprintln!("Error querying AI: {}", e),
            }
        }
        "ai" => {
            if args.len() < 3 {
                println!("Usage: ReplayAssistant ai <match_guid> [focus]");
                return;
            }

            let match_guid = &args[2];
            // Set focus to "all" if not provided, otherwise pass the provided value
            let focus = if args.len() > 3 { &args[3] } else { "all" };

            println!("Querying AI for insights...");
            match query::query_ai(match_guid, focus).await {
                Ok(response) => {
                    let feedback_file_path = format!("./output/{}.feedback.md", match_guid);

                    // Save the AI response to the feedback file
                    match fs::write(&feedback_file_path, &response) {
                        Ok(_) => println!("Saved AI feedback: {}",feedback_file_path),
                        Err(e) => eprintln!("Failed to save AI feedback: {}", e),
                    }
                }
                Err(e) => eprintln!("Error querying AI: {}", e),
            }
        }
        _ => {
            println!("Unknown command: {}", command);
            println!("Usage: replayassist <command> [options]");
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
