use crate::ai::openai;

use std::fs;
use std::env;
use std::io;
use base64::{Engine as _};
use std::io::Write;
use csv::{ReaderBuilder, Writer};
use std::error::Error;
use std::collections::HashMap;

pub async fn query_ai(match_guid: &str, focus: &str) -> io::Result<String> {
    // Define file paths based on the match_guid
    let player_stats_csv_path = format!("./output/{}.player_stats.json.csv", match_guid);
    let goals_csv_path = format!("./output/{}.goals.json.csv", match_guid);
    let highlights_csv_path = format!("./output/{}.highlights.json.csv", match_guid);
    let frames_csv_path = format!("./output/{}.replay.frames.json.csv", match_guid);

    // Combine all selected templates
    let instructions = "You are a world-class Rocket League team coach. You analyze data present in .csv files, understand trends, create data visualizations relevant to those trends, and share brief text summaries of observed trends. Your insights help improve team gameplay. Analyze team positioning, rotations, and overall synergy.".to_string();

    let query = "Evaluate the replay on boost efficiency, aerial control, and shot accuracy using the csv files.  Each file is a csv file, which are linked by a column 'Frame'. Provide insights on situational awareness, risk/reward trade-offs, mechanical highlights, focused on team play, indentifying 1st,2nd,3rd 'main' roles.".to_string();


    let mut responses = Vec::new();
    let header_response = "# Rattlebrain Replay Analysis\n\n\n".to_string();
    responses.push(header_response);

    // OpenAI
    if let Ok(openai_key) = env::var("OPENAI_API_KEY") {
        println!("Using OpenAI with key: {}****", &openai_key[0..8]);
        let mut files: Vec<String> = Vec::new(); 
    
        match openai::upload_file(&player_stats_csv_path).await {
            Ok(file_id) => {
                println!("Uploaded player stats csv: {}",file_id);
                files.push(file_id);
            },
            Err(e) => eprintln!("Error uploading to OpenAI: {}", e),
        }
        match openai::upload_file(&goals_csv_path).await {
            Ok(file_id) => {
                println!("Uploaded goals csv: {}",file_id);
                files.push(file_id);
            },
            Err(e) => eprintln!("Error uploading to OpenAI: {}", e),
        }
        match openai::upload_file(&highlights_csv_path).await {
            Ok(file_id) => {
                println!("Uploaded highlights csv: {}",file_id);
                files.push(file_id);
            },
            Err(e) => eprintln!("Error uploading to OpenAI: {}", e),
        }
        match openai::upload_file(&frames_csv_path).await {
            Ok(file_id) => {
                println!("Uploaded frames csv: {}",file_id);
                files.push(file_id);
            },
            Err(e) => eprintln!("Error uploading  to OpenAI: {}", e),
        }
        match openai::create_assistant(&instructions, &files).await {
            Ok(assistant_id) => {
                println!("Assistant created successfully:{}",assistant_id);
                match openai::query_openai_with_assistant(&query, &assistant_id).await {
                    Ok(response) => {
                        // responses.push(format!("## OpenAI response\n\n {}", response));
                        println!("{}",response)
                    },
                    Err(e) => eprintln!("Error querying OpenAI: {}", e),
                }
            },
            Err(e) => eprintln!("Error creating assistant: {}", e),
        }
        // match openai::query_openai(&query, &files).await {
        //     Ok(response) => responses.push(format!("## OpenAI response\n\n {}", response)),
        //     Err(e) => eprintln!("Error querying OpenAI: {}", e),
        // }
    }

    // Handle case where no providers are configured
    if responses.is_empty() {
        return Err(io::Error::new(
            io::ErrorKind::NotFound,
            "No AI providers configured. Please set one or more of the following: OPENAI_API_KEY, ANTHROPIC_API_KEY, GEMINI_API_KEY, COPILOT_API_KEY.",
        ));
    }
    // Combine all responses
    let combined_response = responses.join("\n\n");
    Ok(combined_response)
}

