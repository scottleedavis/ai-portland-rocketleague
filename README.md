# ai-portland-rocketleague
Materials/code for a [10 minute presentation](https://www.meetup.com/ai-portland/events/303283141/?eventOrigin=group_upcoming_events) for an [AI Portland](https://creators.spotify.com/pod/show/superchargedbyai/) on using AI for Game analysis and self-coaching



## Overview

Rocket League is a fast-paced game requiring quick reflexes, precise control, and strategic team play. This project explores three primary approaches to game and AI integration:

### 1. **Live Mechanics Coaching Feedback**
   - **Description**: Real-time analysis of player mechanics during matches.
   - [AI Dribble Coach](./AICoachBakkesPlugin/) - a [bakkesmod](https://github.com/bakkesmodorg/BakkesModSDK) plugin that uses Anthropic's Claude 3.5-Sonnet model providing live coaching feedback during tracked ground dribble practice in Rocket League. Using a custom Bakkesmod plugin, the AI evaluates ball and car positions in real time to deliver actionable insights on screen. ([recorded example](https://www.youtube.com/watch?v=vU-K88mYVAU) )
   - **Features**:
     - Identifies the mechanical skill of ground dribbling.
     - Offers suggestions on optimal timing, positioning, and ball control.
     - Provides immediate actionable advice to help players adjust their mechanics during freeplay.
   - **Goal**:
     - Provide actionable feedback for air dribbling and flicks as well.

### 2. **Overall Strategy in Team Play Dynamics**
   - **Description**: Analysis of team coordination and strategy.
   - [Rattlebrain](https://github.com/scottleedavis/rattlebrain) - a command line tool written in rust that parses Rocket League replays into a data fromat usable by an LLM, and queries multiple AI providers (Google, Anthropic, OpenAI)
   - **Features**:
     - Evaluates player roles (e.g., striker, defender) and their adherence to team strategy.
     - Highlights gaps in rotations and missed opportunities for passing plays.
     - Suggests adjustments for better synergy and positioning.
   - **Goal**: Enhance team play cohesion and effectiveness.

### 3. **Individual Live Replay Analysis with Video and Audio Feedback**
   - **Description**: Post-game analysis tailored for individual players.
   - [Gemini 2.0 video stream analysis](https://aistudio.google.com/live) - Live replay analysis of the video stream and [CARL](https://lndrlndr.github.io/) statistics.  ([recorded example](https://www.youtube.com/watch?v=2OnrUEvSvAo))
   - **Features**:
     - Breaks down key moments in the replay, analyzing decisions and outcomes.
     - Includes video overlays and audio commentary to explain insights.
     - Provides a personalized improvement plan based on the analysis.
   - **Goal**: Help players understand their strengths and areas for improvement through detailed, context-rich feedback.

## Objectives

- Improve player mechanics and strategic understanding.
- Foster better teamwork and communication within teams.
- Deliver actionable, easy-to-understand insights to players of all skill levels.
- Advance the use of AI in gaming to create a more immersive and educational experience.

## License

This project is licensed under the [Apache 2 License](LICENSE).
