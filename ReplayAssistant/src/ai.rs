use serde::Serialize;
use reqwest::{Client, multipart};
use std::{env, fs::File, io,io::Read,error::Error};

#[derive(Serialize)]
struct OpenAITool {
    r#type: String,
}

#[derive(Serialize)]
struct OpenAIToolResources {
    code_interpreter: OpenAIToolResourcesCodeInterpreter,
}

#[derive(Serialize)]
struct OpenAIToolResourcesCodeInterpreter {
    file_ids: Vec<String>,
}

#[derive(Serialize)]
struct CreateAssistantRequest {
    name: String,
    description: String,
    instructions: String,
    model: String,
    tools: Vec<OpenAITool>,
    tool_resources: OpenAIToolResources,
}

pub async fn create_assistant(match_guid: &str) -> io::Result<String> {

    let openai_key = env::var("OPENAI_API_KEY")
        .expect("Error: OPENAI_API_KEY environment variable is missing!");

    println!("Using OpenAI with key: {}****", &openai_key[0..8]);

    const INSTRUCTIONS: &str = "You are a world-class Rocket League team coach. You analyze data present in .csv files, understand trends, create data visualizations relevant to those trends, and share brief text summaries of observed trends. Your insights help improve team gameplay. Analyze team positioning, rotations, and overall synergy.";
    
    let player_stats_csv_path = format!("./output/{}.player_stats.json.csv", match_guid);
    let goals_csv_path = format!("./output/{}.goals.json.csv", match_guid);
    let highlights_csv_path = format!("./output/{}.highlights.json.csv", match_guid);
    let frames_csv_path = format!("./output/{}.replay.frames.json.csv", match_guid);
    let mut response = "".to_string();
    let mut files: Vec<String> = Vec::new(); 

    match upload_file(&player_stats_csv_path).await {
        Ok(file_id) => {
            println!("Uploaded player stats csv: {}",file_id);
            files.push(file_id);
        },
        Err(e) => eprintln!("Error uploading to OpenAI: {}", e),
    }
    match upload_file(&goals_csv_path).await {
        Ok(file_id) => {
            println!("Uploaded goals csv: {}",file_id);
            files.push(file_id);
        },
        Err(e) => eprintln!("Error uploading to OpenAI: {}", e),
    }
    match upload_file(&highlights_csv_path).await {
        Ok(file_id) => {
            println!("Uploaded highlights csv: {}",file_id);
            files.push(file_id);
        },
        Err(e) => eprintln!("Error uploading to OpenAI: {}", e),
    }
    match upload_file(&frames_csv_path).await {
        Ok(file_id) => {
            println!("Uploaded frames csv: {}",file_id);
            files.push(file_id);
        },
        Err(e) => eprintln!("Error uploading  to OpenAI: {}", e),
    }
    match generate_assistant(INSTRUCTIONS, &files).await {
        Ok(assistant_id) => {
            println!("Assistant created successfully: {}",assistant_id);
            response = assistant_id;
        },
        Err(e) => eprintln!("Error creating assistant: {}", e),
    }

    Ok(response)
}


async fn upload_file(file_path: &str) -> Result<String, Box<dyn std::error::Error>> {
    let client = Client::new();
    let api_key = env::var("OPENAI_API_KEY").expect("OPENAI_API_KEY must be set");
    let api_url = "https://api.openai.com/v1/files";

    let mut file = File::open(file_path)?;
    let mut buffer = Vec::new();
    file.read_to_end(&mut buffer)?;

    let form = multipart::Form::new()
        .part("file", multipart::Part::bytes(buffer).file_name(file_path.to_string()))
        .text("purpose", "assistants");

    let response = client
        .post(api_url)
        .bearer_auth(api_key)
        .multipart(form)
        .send()
        .await?;

    let json: serde_json::Value = response.json().await?;
    let file_id = json["id"]
    .as_str()
    .ok_or("Failed to get file ID")?;
    Ok(file_id.to_string())
}


async fn generate_assistant(instructions: &str, files: &[String]) -> Result<String, Box<dyn Error>>  {
    let client = Client::new();
    let api_key = env::var("OPENAI_API_KEY").expect("OPENAI_API_KEY must be set");
    let api_url = "https://api.openai.com/v1/assistants";

    let request_body = CreateAssistantRequest {
        name: "Rocket League Replay Coach".to_string(),
        description: "A rocket league analysis assistant".to_string(),
        instructions: instructions.to_string(),
        model: "gpt-4o".to_string(),
        tools: vec![OpenAITool {
            r#type: "code_interpreter".to_string(),
        }],
        tool_resources: OpenAIToolResources {
            code_interpreter: OpenAIToolResourcesCodeInterpreter {
                file_ids: files.to_vec(),
            },
        },
    };

    let response = client
        .post(api_url)
        .header("Authorization", format!("Bearer {}", api_key))
        .header("Content-Type", "application/json")
        .header("OpenAI-Beta", "assistants=v2")
        .json(&request_body)
        .send()
        .await?;

    if response.status().is_success() {
        let response_text = response.text().await?;
        // println!("Assistant created successfully: {}", response_text);
        let response_json: serde_json::Value = serde_json::from_str(&response_text)?;
        let assistant_id = response_json["id"].as_str().unwrap_or_default().to_string();
        Ok(assistant_id)
    } else {
        let status = response.status();
        let error_text = response.text().await?;
        Err(format!("Failed to create assistant ({}): {}", status, error_text).into())
    }
}


const PROMPT: &str = "Evaluate the replay on boost efficiency, aerial control, and shot accuracy using the csv files. The csv files are linked by a primary key column 'Frame'. Provide insights on situational awareness, risk/reward trade-offs, mechanical highlights. Also focus on team play, identifying dominant roles.";

pub async fn prompt_assistant(assistant_id: &str, prompt: &str) -> Result<String, Box<dyn Error>> {

    if assistant_id.is_empty() {
        return Err("Assistant ID is required.".into());
    }

    // Use the default prompt if none is provided
    let prompt_to_use = if prompt.is_empty() {
        PROMPT.to_string()
    } else {
        prompt.to_string()
        // TODO URL decode is needed 
    };

    println!("Assistant ID: {}", assistant_id);
    println!("Prompt: {}", prompt);


    Ok("reponse".to_string())
}
