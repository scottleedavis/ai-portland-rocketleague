// use rattlebrain::query::query_ai;
// use std::fs;

// #[test]
// fn test_query_output_file_exists() {
//     // Define the path to the query output file
//     let match_guid = "383F0B0411EFAC27082CAFA884251EFF".to_string();
//     let query_output_path = format!("output/{}.query.txt", match_guid);
//     let focus = "all".to_string();

//     // println!("Querying AI for insights...");
//     // match query_ai(match_guid, focus).await {
//     //     Ok(response) => {
//     //         println!("{}", &response);
//     //     }
//     //     Err(e) => eprintln!("Failed to save AI feedback: {}", e),
//     // }

//     // Check if the output file exists
//     assert!(
//         fs::metadata(query_output_path).is_ok(),
//         "The query output file does not exist at the expected location: {}",
//         query_output_path
//     );

//     // Cleanupfile
//     fs::remove_file(query_output_path).expect("Failed to remove query text file");
// }

// #[test]
// async fn test_query_output_file_exists() {
//     // Define the path to the query output file
//     let match_guid = "383F0B0411EFAC27082CAFA884251EFF".to_string();
//     let query_output_path = format!("output/{}.query.txt", match_guid);
//     let focus = "all".to_string();

//     println!("Querying AI for insights...");
//     match query::query_ai(match_guid, focus).await {
//         Ok(response) => {
//             println!("{}", &response);
//         }
//         Err(e) => eprintln!("Failed to save AI feedback: {}", e),
//     }

//     // Check if the output file exists
//     assert!(
//         fs::metadata(query_output_path).is_ok(),
//         "The query output file does not exist at the expected location: {}",
//         query_output_path
//     );

//     // Cleanupfile
//     fs::remove_file(query_output_path).expect("Failed to remove query text file");
// }



// #[cfg(test)]
// mod tests {
//     use super::*;
//     use tokio;

//     #[tokio::test]
//     async fn test_claude_send_prompt() {
//         let api_key = "test_api_key";
//         let base_url = "https://api.anthropic.com/v1/complete";
//         let claude_ai = ClaudeAI::new(api_key, base_url);

//         // Replace this with a real prompt and expected response for integration testing.
//         let prompt = "Hello, Claude!";
//         let response = claude_ai
//             .send_prompt(prompt, Some(100), Some(0.7))
//             .await;

//         match response {
//             Ok(completion) => {
//                 println!("Received response: {}", completion);
//                 assert!(!completion.is_empty());
//             }
//             Err(e) => panic!("Failed to send prompt: {}", e),
//         }
//     }
// }
