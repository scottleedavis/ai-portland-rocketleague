# ReplayAssistant

## How It Works

1. **Parsing Replays**: 
   ReplayAssistant parses Rocket League replay files into a structured csv files on player statistics, highlights, goals and raw frames.

2. **AI Assistant**: 
   - CSV files are uploaded to OpenAi, and use to create an assistant with instructions for how to use the data.

3. **Stream based textual feedback**:
   - Outputs from the OpenAI stream are displayed in a console window during a replay as they are received.

## Plugin Installation : [plugin/](plugin/)

1. **Prerequisites**:
   - Ensure [BakkesMod](https://bakkesmod.com/) is installed.
   - Obtain access to OpenAI's API.  `OPENAI_API_KEY=<youkey>`

2. **Installation Steps**:
   - Download the ReplayAssistant plugin files from this repository.
   - Place the plugin files into the BakkesMod plugins directory.
   - Launch Rocket League and BakkesMod to load the plugin.

## Usage

- Start a replay with the plugin enabled.
- Once in a replay you'd like an assistant with, click the replay assistant button.
- interact by typing in free form questions into the thread/prompt.

## Future Goals

- Improve markdown rendering quality with images and fonts.

# ReplayAssistant Service [service/](service/)

**ReplayAssistant** is a command-line tool, written in Rust, parses binary replay files into csv files, and connects them to an OpenAI Assistant with instructions. ReplayAssistant extracts tactical insights, performance metrics, and actionable feedback designed to elevate gameplay.

---

## Getting Started

### Prerequisites

1. Obtain an API key for the Open AI

2. Set Environment:
     ```bash
     export OPENAI_API_KEY=<your_openai_api_key>
     python -m venv venv
     ```
3. Build Server
     ```bash
     ./build.sh
     ```
     or alternatively
     Build Docker
     ```bash
     docker build -t replay-assistant-service .
     ```
4. Run Server
     ```bash
     source venv/bin/activate
     ./run.sh
     ```
     or alternatively
   Run Docker 
   ```bash
   docker run -p 5000:5000 -e OPENAI_API_KEY=your_openai_api_key replay-assistant-service
   ```
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
# cross build --release --target x86_64-apple-darwin
# cross build --release --target aarch64-apple-darwin
# cross build --release --target x86_64-pc-windows-gnu
```

#### Testing

```bash
cargo test
```

---

## Acknowledgments

- **[Rattletrap](https://github.com/tfausak/rattletrap)**: ReplayAssistant wouldn’t be possible without this fantastic replay parser. Kudos to the creators and maintainers for providing such a robust tool!
- **AI Service Providers**: Anthropic and OpenAI, for their advanced language models powering these tools.

---

## Contributing

Contributions are welcome! Please open an issue or submit a pull request if you’d like to improve ReplayAssistant.

---

