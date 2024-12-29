# ReplayCoaches

ReplayCoaches uses a command-line tool written in Rust that parses Rocket League replay files into a format suitable for analysis by large language models (LLMs). It integrates with multiple AI providers, including Google, Anthropic, and OpenAI, to deliver comprehensive reports on team coordination and strategy.

See the [example output](./examples/CAE7013011EFA9DB5C4584B38DA4222F.feedback.md)
## Features

- **AI-Enhanced Analysis**: Utilizes AI services for in-depth insights into replay data.
- **Dynamic AI Selection**: Automatically detects and configures available AI services via environment variables (e.g., `OPENAI_API_KEY`, `ANTHROPIC_API_KEY`, `GEMINI_API_KEY`).
- **Unified Reporting**: Combines outputs from multiple AI agents into a single, comprehensive report.
- **Tactical Insights**: Highlights key plays, strategic opportunities, and areas for improvement.
- **Performance Metrics**: Extracts useful data points to track player progress over time.

## Installation

1. **Prerequisites**:
   - Install [Rattletrap](https://github.com/tfausak/rattletrap), a Rocket League replay parser.
   - Obtain API keys for the AI services you'd like to use (e.g., OpenAI, Anthropic, Google).

2. **Installation Steps**:
   - Clone this repository: `git clone https://github.com/scottleedavis/rattlebrain.git`
   - Navigate to the project directory: `cd rattlebrain`
   - Build the project: `cargo build --release`

## Usage

1. **Set Environment Variables**:
   - Export your API keys:
     ```bash
     export OPENAI_API_KEY=<your_openai_api_key>
     export ANTHROPIC_API_KEY=<your_anthropic_api_key>
     export GEMINI_API_KEY=<your_google_gemini_api_key>
     ```

2. **Analyze a Replay**:
   - Run the analysis command:
     ```bash
     ./rattlebrain analyze <replay_file>
     ```

3. **View the Report**:
   - After analysis, a comprehensive report will be generated, highlighting tactical insights and performance metrics.

## License

This project is licensed under the MIT License.
