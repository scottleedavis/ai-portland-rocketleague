# ScreenShareCoach

ScreenShareCoach is a live coaching tool designed to provide real-time analysis and feedback during Rocket League gameplay. This tool combines video stream analysis with gameplay statistics (e.g., CARL metrics) to deliver actionable insights and personalized improvement plans.

## Features

- **Real-Time Replay Analysis**: Analyzes live gameplay and replay footage to provide context-rich feedback.
- **Video and Audio Overlays**: Highlights key moments, decision points, and areas for improvement via video and audio commentary.
- **Personalized Coaching**: Creates improvement plans tailored to individual gameplay style and strategy.
- **AI Integration**: Utilizes advanced AI models to extract meaningful insights from video and statistical data.

## Installation

1. **Prerequisites**:
   - Ensure you have a reliable screen recording or streaming setup.
   - Install any dependencies for video and audio analysis tools (specific details coming soon).

2. **Installation Steps**:
   - Clone the repository:
     ```bash
     git clone https://github.com/scottleedavis/ai-portland-rocketleague.git
     ```
   - Navigate to the `ScreenShareCoach` directory:
     ```bash
     cd ai-portland-rocketleague/ScreenShareCoach
     npm install
     ```

## Usage

1. **Set Up a Game Session**:
   - Begin playing Rocket League or review replay footage.
   - Stream or screen-share your gameplay for live analysis.
  ```bash
  export GEMINI_API_KEY=<your_google_gemini_api_key>
  npm run dev
  ```

2. **Start ScreenShareCoach**:
   - Launch the tool and point it to your video feed or replay files.
   - Observe real-time overlays with insights and feedback.

3. **Receive Feedback**:
   - Review video highlights and listen to audio commentary for suggestions on improving mechanics, strategy, and decision-making.

## Future Goals

- Integrate advanced AI models for enhanced contextual analysis.
- Expand compatibility with popular streaming platforms like Twitch and YouTube.
- Include team-based analysis for cooperative gameplay.

## Contribution

We welcome contributions to improve the functionality and user experience of ScreenShareCoach. Feel free to fork the repository and submit pull requests.

## License

This project is licensed under the Apache 2 License.
