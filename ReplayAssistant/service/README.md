# ReplayAssistant

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
3. Run Server
     ```bash
     source venv/bin/activate
     ./run.sh
     ```

## How It Works

1. **Parsing Replays**: 
   ReplayAssistant uses **Rattletrap** to decode Rocket League replay files into a structured csv files on player statistics, highlights, goals and raw frames.

2. **AI Assistant**: 
   - CSV files are uploaded to OpenAi, and use to create an assistant with instructions for how to use the data.

3. **Stream based textual feedback**:
   - Outputs from the OpenAI stream are displayed as they are received.

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
- **AI Service Providers**: OpenAI, for their advanced language models powering this tool.

---

## Contributing

Contributions are welcome! Please open an issue or submit a pull request if you’d like to improve ReplayAssistant.

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
