# TODOs before presentation

1) ReplayCoaches: use file upload for csv data (test openai first)
2) ScreenShareCoach:  try openai live streaming for same screenshare test.  (waiting on gemini fix.  can fall back to gemini 2.0 ai studio)
3) DribbleCoach: evaluate air dribble addition to dribble coach


## Notes

To achieve CSV file upload and querying functionality using Rust with OpenAI's API, you can use an HTTP client like `reqwest` to make direct API calls.

Here’s how you can implement it:

---

### **1. Add Dependencies**
In your `Cargo.toml`, add the required dependencies:
```toml
[dependencies]
reqwest = { version = "0.11", features = ["json", "blocking"] }
tokio = { version = "1", features = ["full"] }
serde = { version = "1.0", features = ["derive"] }
serde_json = "1.0"
```

---

### **2. Upload the CSV**
Create a function to upload the CSV file to OpenAI's file storage:

```rust
use reqwest::blocking::Client;
use std::fs::File;
use std::io::Read;

fn upload_csv(api_key: &str, file_path: &str) -> Result<String, Box<dyn std::error::Error>> {
    let client = Client::new();

    // Read the CSV file
    let mut file = File::open(file_path)?;
    let mut file_contents = Vec::new();
    file.read_to_end(&mut file_contents)?;

    // Make the request
    let response = client
        .post("https://api.openai.com/v1/files")
        .bearer_auth(api_key)
        .multipart(
            reqwest::blocking::multipart::Form::new()
                .file("file", file_path)? // Attach the file
                .text("purpose", "answers"), // Specify the purpose
        )
        .send()?;

    // Parse the response
    let json: serde_json::Value = response.json()?;
    let file_id = json["id"].as_str().ok_or("Failed to get file ID")?;

    Ok(file_id.to_string())
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let api_key = "your-api-key";
    let file_path = "data.csv";

    let file_id = upload_csv(api_key, file_path)?;
    println!("File uploaded successfully! File ID: {}", file_id);

    Ok(())
}
```

---

### **3. Query the Uploaded File**
Create a function to query the uploaded file using its `file_id`:

```rust
fn query_csv(
    api_key: &str,
    file_id: &str,
    question: &str,
) -> Result<String, Box<dyn std::error::Error>> {
    let client = Client::new();

    // Define the request body
    let body = serde_json::json!({
        "model": "davinci",
        "question": question,
        "file": file_id,
        "examples_context": "The dataset contains sales information.",
        "examples": [
            ["What is the highest sale?", "The highest sale is $5000."],
            ["How many sales were made?", "There were 200 sales."]
        ],
        "max_rerank": 10,
        "max_tokens": 100
    });

    // Make the request
    let response = client
        .post("https://api.openai.com/v1/answers")
        .bearer_auth(api_key)
        .json(&body)
        .send()?;

    // Parse the response
    let json: serde_json::Value = response.json()?;
    let answer = json["answers"]
        .as_array()
        .and_then(|arr| arr.get(0))
        .and_then(|val| val.as_str())
        .ok_or("Failed to get an answer")?;

    Ok(answer.to_string())
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let api_key = "your-api-key";
    let file_id = "your-file-id";
    let question = "What is the total revenue in the CSV?";

    let answer = query_csv(api_key, file_id, question)?;
    println!("Answer: {}", answer);

    Ok(())
}
```

---

### **4. Workflow**
1. Upload your CSV using `upload_csv`. Note the returned `file_id`.
2. Query the file using `query_csv`, passing the `file_id` and your question.

---

### **5. Additional Notes**
- **Authentication**: Use the `Authorization: Bearer YOUR_API_KEY` header for all API calls.
- **Error Handling**: Add more robust error handling for production.
- **Rate Limits**: Be mindful of OpenAI's API rate limits.
- **Alternative**: If you don’t want to upload files, parse the CSV in Rust and pass its contents directly to the GPT model in the query payload.

Let me know if you need help tailoring this code further!