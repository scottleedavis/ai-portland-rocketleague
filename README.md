# ai-portland-rocketleague
Materials/code for a [10 minute presentation](https://www.meetup.com/ai-portland/events/303283141/?eventOrigin=group_upcoming_events) for an [AI Portland](https://creators.spotify.com/pod/show/superchargedbyai/) on using AI for Game analysis and self-coaching


## Overview

Rocket League is a fast-paced game requiring quick reflexes, precise control, and strategic team play. This project explores three primary approaches to game and AI integration:

### 1. [DribbleCoach](./DribbleCoach/README.md) **: Live Mechanics Feedback**
![DribbleCoach.png](DribbleCoach.png)
   - **Description**:  Textual feedback of dribble mechanics using Anthropic's Claude during freeplay.
     - Identifies the mechanical skill of ground dribbling.
     - Offers simple suggestions on optimal timing, positioning, and ball control.
   - **ToDo**:
     - Provide air dribbling and flicks feedback as well.

### 2. [ReplayAssistant](./ReplayAssistant/README.md) **: Interactive Feedback on replay data**
![ReplayAssistantPrepare.png](ReplayAssistantPrepare.png)
![ReplayAssistantPrompt.png](ReplayAssistantPrompt.png)
   - **Description**: Extracts replay data and creates an OpenAI assistant available on the current replay.
   - available commands

prepare an OpenAI Assistant for the current replay
```bash
replay_prepare
```

free form prompt
```bash
replay_prompt tell me how many times I hit the ball towards my own goal.l
```

default prompt 
```bash
replay_prompt 
```
equivalent to
```bash
replay_prompt Evaluate the replay on boost efficiency, aerial control, and shot accuracy using the csv files.  The csv files are linked by a primary key column 'Frame'. Provide insights on situational awareness, risk/reward trade-offs, mechanical highlights.  Also focus on team play, indentifying dominant roles.
```
   - **ToDo**:
     - Finish replay_prompt
     - Provide ai response replay overlays

## Objectives

- Improve player mechanics and strategic understanding.
- Foster better teamwork and communication within teams.
- Deliver actionable, easy-to-understand insights to players of all skill levels.
- Advance the use of AI in gaming to create a more immersive and educational experience.

## License

This project is licensed under the [Apache 2 License](LICENSE).
