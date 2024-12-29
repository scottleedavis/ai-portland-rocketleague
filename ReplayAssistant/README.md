# ReplayAssistant

**ReplayAssistant** is a command-line tool, written in Rust, designed to bring the power of AI providers to your **Rocket League** replay analysis. By leveraging **[Rattletrap](https://github.com/tfausak/rattletrap)** for parsing replay files and connecting to one or more AI services (e.g., OpenAI, Anthropic, Google), ReplayAssistant extracts tactical insights, performance metrics, and actionable feedback to help you elevate your gameplay.


## Usage
[Download](https://github.com/scottleedavis/ReplayAssistant/releases) and Run the latest release of **ReplayAssistant** with a replay file
```bash
./ReplayAssistant analyze <replay_file>
```

---

## Features

- **AI-Enhanced Analysis**: Analyze Rocket League replay files with the help of AI services for in-depth insights.
- **Dynamic AI Selection**: Automatically detects and configures available AI services via environment variables (e.g., `OPENAI_API_KEY`, `ANTHROPIC_API_KEY`, `GEMINI_API_KEY`).
- **Unified Reporting**: Combines outputs from multiple AI agents into a single, comprehensive report.
- **Tactical Insights**: Highlights key plays, strategic opportunities, and areas for improvement.
- **Performance Metrics**: Extracts useful data points to track your progress over time.
- **Lightweight and Fast**: Built with Rust for performance and efficiency.

---

## Getting Started

### Prerequisites
1. Install **Rattletrap**:
   - ReplayAssistant uses [Rattletrap](https://github.com/tfausak/rattletrap/releases) to parse replay files, and is automatically downloaded if not detected in $PATH.  

2. Obtain API Keys:
   - Get API keys for the AI services you'd like to use (e.g., OpenAI, Claude).

3. Set Environment Variables:
   - Add API keys to AI Services in environment variables (at least 1):
     ```bash
     export OPENAI_API_KEY=<your_openai_api_key>
     export ANTHROPIC_API_KEY=<your_anthropic_api_key>
     export GEMINI_API_KEY=<your_google_gemini_api_key>
     ```
4. Run ReplayAssistant:
     ```bash
      ./ReplayAssistant analyze ./examples/363cf8d9-8d7f-4c55-95a8-b97edbab0449.replay 
     ```

## How It Works

1. **Parsing Replays**: 
   ReplayAssistant uses **Rattletrap** to decode Rocket League replay files into a structured format.

2. **AI Integration**: 
   - Depending on the environment variables detected, ReplayAssistant connects to one or more AI services.
   - Each AI service processes the replay data, generating unique feedback and insights.

3. **Unified Report**:
   - Outputs from all AI services are combined into a single, readable report, highlighting tactical analysis, performance metrics, and key observations.

---

### Installation
Clone the repository and build **ReplayAssistant**:
```bash
git clone https://github.com/scottleedavis/ReplayAssistant.git
cd ReplayAssistant
```
#### Building

```bash
cargo build --release
```

#### Testing

```bash
cargo test
```

---

## Acknowledgments

- **[Rattletrap](https://github.com/tfausak/rattletrap)**: ReplayAssistant wouldn’t be possible without this fantastic replay parser. Kudos to the creators and maintainers for providing such a robust tool!
- **AI Service Providers**: OpenAI, Anthropic, and Google for their advanced language models powering this tool.

---

## Contributing

Contributions are welcome! Please open an issue or submit a pull request if you’d like to improve ReplayAssistant.

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
