# ai-portland-rocketleague
Materials/code for a 10 minute presentation for an [AI Portland podcast[(https://creators.spotify.com/pod/show/superchargedbyai/) on using AI for Game analysis and coaching

Work In Progress 

Parts
* [AI Dribble Coach](./AICoachBakkesPlugin/) - a [bakkesmod](https://github.com/bakkesmodorg/BakkesModSDK) plugin that uses Anthropic's Claude 3.5-Sonnet model providing live coaching feedback during tracked ground dribble practice in Rocket League. Using a custom Bakkesmod plugin, the AI evaluates ball and car positions in real time to deliver actionable insights on screen. ([recorded example](https://www.youtube.com/watch?v=vU-K88mYVAU) )
* [Rattlebrain](https://github.com/scottleedavis/rattlebrain) - a command line tool written in rust that parses Rocket League replays into a data fromat usable by an LLM, and queries multiple AI providers (Google, Anthropic, OpenAI)
* [Gemini 2.0 video stream analysis](https://aistudio.google.com/live) - Live replay analysis of the video stream and [CARL](https://lndrlndr.github.io/) statistics.  ([recorded example](https://www.youtube.com/watch?v=2OnrUEvSvAo))
