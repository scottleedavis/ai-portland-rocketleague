// use serde::{Deserialize, Serialize};
// use std::error::Error;
// use reqwest::{Client, multipart};
// use std::{env, fs::File};
// use std::io::Read;

// #[derive(Serialize)]
// struct ChatRequest {
//     messages: Vec<Message>,
// }

// #[derive(Serialize)]
// struct Message {
//     role: String,
//     content: String,
// }

// #[derive(Serialize)]
// struct OpenAITool {
//     r#type: String,
// }

// #[derive(Serialize)]
// struct OpenAIToolResources {
//     code_interpreter: OpenAIToolResourcesCodeInterpreter,
// }

// #[derive(Serialize)]
// struct OpenAIToolResourcesCodeInterpreter {
//     file_ids: Vec<String>,
// }

// #[derive(Serialize)]
// struct CreateAssistantRequest {
//     name: String,
//     description: String,
//     instructions: String,
//     model: String,
//     tools: Vec<OpenAITool>,
//     tool_resources: OpenAIToolResources,
// }

// #[derive(Serialize)]
// struct OpenAIRequest {
//     name: String,
//     description: String,
//     model: String,
//     tools: Vec<OpenAITool>,
//     tool_resources: OpenAIToolResources,
// }

// pub async fn upload_file(file_path: &str) -> Result<String, Box<dyn std::error::Error>> {
//     let client = Client::new();
//     let api_key = env::var("OPENAI_API_KEY").expect("OPENAI_API_KEY must be set");
//     let api_url = "https://api.openai.com/v1/files";

//     // Read the file contents
//     let mut file = File::open(file_path)?;
//     let mut buffer = Vec::new();
//     file.read_to_end(&mut buffer)?;

//     // Create the multipart form
//     let form = multipart::Form::new()
//         .part("file", multipart::Part::bytes(buffer).file_name(file_path.to_string()))
//         .text("purpose", "assistants");

//     // Send the request to OpenAI's file endpoint
//     let response = client
//         .post(api_url)
//         .bearer_auth(api_key)
//         .multipart(form)
//         .send()
//         .await?;

//     // Parse the response JSON to extract the file ID
//     let json: serde_json::Value = response.json().await?;
//     // println!("Response JSON: {}", json.to_string());
//     let file_id = json["id"]
//     .as_str()
//     .ok_or("Failed to get file ID")?;
//     Ok(file_id.to_string())
// }

// pub async fn create_assistant(instructions: &str, files: &[String]) -> Result<String, Box<dyn Error>>  {
//     let client = Client::new();
//     let api_key = env::var("OPENAI_API_KEY").expect("OPENAI_API_KEY must be set");
//     let api_url = "https://api.openai.com/v1/assistants";

//     // Prepare the request body
//     let request_body = CreateAssistantRequest {
//         name: "Rocket League Replay Coach".to_string(),
//         description: "A rocket league analysis assistant".to_string(),
//         instructions: instructions.to_string(),
//         model: "gpt-4o".to_string(),
//         tools: vec![OpenAITool {
//             r#type: "code_interpreter".to_string(),
//         }],
//         tool_resources: OpenAIToolResources {
//             code_interpreter: OpenAIToolResourcesCodeInterpreter {
//                 file_ids: files.to_vec(),
//             },
//         },
//     };

//     // Send the POST request
//     let response = client
//         .post(api_url)
//         .header("Authorization", format!("Bearer {}", api_key))
//         .header("Content-Type", "application/json")
//         .header("OpenAI-Beta", "assistants=v2")
//         .json(&request_body)
//         .send()
//         .await?;

//     // Handle the response
//     if response.status().is_success() {
//         let response_text = response.text().await?;
//         // println!("Assistant created successfully: {}", response_text);

//         // Parse the response to extract the assistant ID (optional)
//         let response_json: serde_json::Value = serde_json::from_str(&response_text)?;
//         let assistant_id = response_json["id"].as_str().unwrap_or_default().to_string();
//         Ok(assistant_id)
//     } else {
//         let status = response.status();
//         let error_text = response.text().await?;
//         Err(format!("Failed to create assistant ({}): {}", status, error_text).into())
//     }
// }

// // pub async fn create_assistant(description: &str, files: &[String]) -> Result<String, Box<dyn Error>> {
// //     let client = Client::new();
// //     let api_key = env::var("OPENAI_API_KEY").expect("OPENAI_API_KEY must be set");
// //     // needs header "OpenAI-Beta: assistants=v2" \
// // }


// pub async fn query_openai_with_assistant(prompt: &str, assistant_id: &str) -> Result<String, Box<dyn Error>> {
//     // Get the OpenAI API key from the environment
//     let api_key = env::var("OPENAI_API_KEY").expect("OPENAI_API_KEY must be set");
//     let api_url = format!("https://api.openai.com/v1/assistants/{}/messages", assistant_id);

//     // Create the OpenAI request payload
//     let request_body = ChatRequest {
//         messages: vec![
//             Message {
//                 role: "user".to_string(),
//                 content: prompt.to_string(),
//             },
//         ],
//     };

//     // Send the POST request
//     let client = Client::new();
//     let response = client
//         .post(&api_url)
//         .header("Authorization", format!("Bearer {}", api_key))
//         .header("Content-Type", "application/json")
//         .header("OpenAI-Beta", "assistants=v2")
//         .json(&request_body)
//         .send()
//         .await?;

//     // Handle the response
//     if response.status().is_success() {
//         let response_text = response.text().await?;
//         println!("OpenAI Assistant Response: {}", response_text);
//         Ok(response_text)
//     } else {
//         let status = response.status();
//         let error_text = response.text().await?;
//         Err(format!("OpenAI Assistant API Error ({}): {}", status, error_text).into())
//     }
// }


// pub async fn query_openai(_prompt: &str) -> Result<String, Box<dyn Error>> {

//     // Get the OpenAI API key from the environment
//     // let api_key = env::var("OPENAI_API_KEY").expect("OPENAI_API_KEY must be set");
//     // let api_url = "https://api.openai.com/v1/chat/completions";

//     // // Create the OpenAI request payload
//     // let request_body = OpenAIRequest {
//     //     name: "Rocket League Replay Coach".to_string(),
//     //     description: prompt.to_string(),
//     //     model: "gpt-4o".to_string(),
//     //     tools: vec![OpenAITool {
//     //         r#type: "code_interpreter".to_string(),
//     //     }],
//     //     tool_resources: OpenAIToolResources {
//     //         code_interpreter: OpenAIToolResourcesCodeInterpreter {
//     //             file_ids: files.to_vec()
//     //         },
//     //     },
//     // };

//     Ok("foo".to_string())
//     // // Send the request to OpenAI
//     // let client = Client::new();
//     // let response = client
//     //     .post(api_url)
//     //     .header("Authorization", format!("Bearer {}", api_key))
//     //     .json(&request_body)
//     //     .send()
//     //     .await?;

//     // // Handle the response
//     // if response.status().is_success() {
//     //     let openai_response: OpenAIResponse = response.json().await?;
//     //     if let Some(choice) = openai_response.choices.first() {
//     //         Ok(choice.message.content.clone())
//     //     } else {
//     //         Err("No response content from OpenAI.".into())
//     //     }
//     // } else {
//     //     let status = response.status();
//     //     let error_text = response.text().await?;
//     //     Err(format!("OpenAI API Error ({}): {}", status, error_text).into())
//     // }
// }
