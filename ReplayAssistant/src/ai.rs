use serde::{Deserialize,Serialize};
use serde_json::{json,Value};
use reqwest::{Client, multipart};
use std::{env, fs::File, io,io::Read,error::Error};

#[derive(Serialize,Deserialize)]
struct OpenAITool {
    r#type: String,
}

#[derive(Debug,Serialize,Deserialize)]
struct OpenAIToolResources {
    code_interpreter: Option<OpenAIToolResourcesCodeInterpreter>,
}

#[derive(Debug,Serialize,Deserialize)]
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

#[derive(Debug,Serialize,Clone)]
struct UserMessage {
    role: String,
    content: String,
}

#[derive(Debug,Serialize)]
struct CreateMessage {
    messages: Vec<UserMessage>,
}

#[derive(Debug,Serialize)]
struct CreateThreadAndRunRequest {
    assistant_id: String,
    thread: CreateMessage,
}

const INSTRUCTIONS: &str = "You are a world-class Rocket League team coach. You analyze data present in .csv files, understand trends, create data visualizations relevant to those trends, and share brief text summaries of observed trends. Your insights help improve team gameplay. Analyze team positioning, rotations, and overall synergy.";
    
const PROMPT: &str = "Evaluate the replay on boost efficiency, aerial control, and shot accuracy using the csv files. The csv files are linked by a primary key column 'Frame'. Provide insights on situational awareness, risk/reward trade-offs, mechanical highlights. Also focus on team play, identifying dominant roles.";

pub async fn create_assistant(match_guid: &str) -> io::Result<String> {

    let openai_key = env::var("OPENAI_API_KEY")
        .expect("Error: OPENAI_API_KEY environment variable is missing!");

    println!("Using OpenAI with key: {}****", &openai_key[0..8]);


    let player_stats_csv_path = format!("./output/{}.player_stats.json.csv", match_guid);
    let goals_csv_path = format!("./output/{}.goals.json.csv", match_guid);
    let highlights_csv_path = format!("./output/{}.highlights.json.csv", match_guid);
    let frames_csv_path = format!("./output/{}.replay.frames.json.csv", match_guid);
    let mut response = "".to_string();
    let mut files: Vec<String> = Vec::new(); 

    match upload_file(&player_stats_csv_path).await {
        Ok(file_id) => {
            println!("Uploaded player stats: {}",file_id);
            files.push(file_id);
        },
        Err(e) => eprintln!("Error uploading to OpenAI: {}", e),
    }
    match upload_file(&goals_csv_path).await {
        Ok(file_id) => {
            println!("Uploaded goals: {}",file_id);
            files.push(file_id);
        },
        Err(e) => eprintln!("Error uploading to OpenAI: {}", e),
    }
    match upload_file(&highlights_csv_path).await {
        Ok(file_id) => {
            println!("Uploaded highlights: {}",file_id);
            files.push(file_id);
        },
        Err(e) => eprintln!("Error uploading to OpenAI: {}", e),
    }
    match upload_file(&frames_csv_path).await {
        Ok(file_id) => {
            println!("Uploaded frames: {}",file_id);
            files.push(file_id);
        },
        Err(e) => eprintln!("Error uploading  to OpenAI: {}", e),
    }
    match generate_assistant(INSTRUCTIONS, &files).await {
        Ok(thread_id) => {
            println!("Assistant created successfully on thread: {}",thread_id);
            response = thread_id;
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
            code_interpreter: Some(OpenAIToolResourcesCodeInterpreter {
                file_ids: files.to_vec(),
            }),
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
        let id = create_thread_and_run(&assistant_id, PROMPT).await?;
        Ok(id)
    } else {
        let status = response.status();
        let error_text = response.text().await?;
        Err(format!("Failed to create assistant ({}): {}", status, error_text).into())
    }
}

async fn create_thread_and_run(assistant_id: &str, prompt: &str) -> Result<String, Box<dyn Error>> {
    let client = Client::new();
    let api_key = env::var("OPENAI_API_KEY").expect("OPENAI_API_KEY must be set");
    let api_url = "https://api.openai.com/v1/threads/runs";

    let request_body = CreateThreadAndRunRequest {
        assistant_id: assistant_id.to_string(),
        thread: CreateMessage {
            messages: vec![UserMessage {
                role: "user".to_string(),
                content: prompt.to_string(),
            }]
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
        let response_json: serde_json::Value = response.json().await?;
        // println!("{:?}", response_json);
        Ok(response_json["thread_id"].as_str().unwrap_or_default().to_string())
    } else {
        let status = response.status();
        let error_text = response.text().await?;
        Err(format!("Failed to create run ({}): {}", status, error_text).into())
    }
}

pub async fn create_message(thread_id: &str, prompt: &str) -> Result<String, Box<dyn Error>>  {
    let client = Client::new();
    let api_key = env::var("OPENAI_API_KEY").expect("OPENAI_API_KEY must be set");
    let api_url = format!("https://api.openai.com/v1/threads/{}/messages", thread_id);

    let body = json!({ "role": "user", "content": prompt });

    let response = client
        .post(&api_url)
        .header("Authorization", format!("Bearer {}", api_key))
        .header("Content-Type", "application/json")
        .header("OpenAI-Beta", "assistants=v2")
        .json(&body)
        .send()
        .await?;

    let response_json: serde_json::Value = response.json().await?;
    println!("{:?}",response_json);
    Ok(response_json["content"].as_str().unwrap_or_default().to_string())
}

pub async fn get_messages(thread_id: &str) -> Result<Vec<String>, Box<dyn Error>>  {
    let client = Client::new();
    let api_key = env::var("OPENAI_API_KEY").expect("OPENAI_API_KEY must be set");
    let api_url = format!("https://api.openai.com/v1/threads/{}/messages", thread_id);

    let response = client
        .get(&api_url)
        .header("Authorization", format!("Bearer {}", api_key))
        .header("Content-Type", "application/json")
        .header("OpenAI-Beta", "assistants=v2")
        .send()
        .await?;

    let response_json: serde_json::Value = response.json().await?;
    let responses = parse_text_values(response_json.clone());
    // println!("{:?}",responses);
    Ok(responses)
}

fn parse_text_values(response_json: Value) -> Vec<String> {
    let mut text_values = Vec::new();

    if let Some(data_array) = response_json.get("data").and_then(|data| data.as_array()) {
        for entry in data_array {
            let role = entry.get("role").and_then(|role| role.as_str()).unwrap_or("unknown");
            if let Some(content_array) = entry.get("content").and_then(|content| content.as_array()) {
                for content_item in content_array {
                    if let Some(text_value) = content_item
                        .get("text")
                        .and_then(|text| text.get("value"))
                        .and_then(|value| value.as_str())
                    {
                        text_values.push(role.to_owned()+": "+text_value);
                    }
                }
            }
        }
    }
    text_values.reverse();
    return text_values;
}
