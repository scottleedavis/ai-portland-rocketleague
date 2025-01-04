#!/bin/bash

USER=$(whoami)  # Get the current username
# REPLAY_DIR="/mnt/c/Users/${USER}/OneDrive/Documents/My Games/Rocket League/TAGame/DemosEpic/"
REPLAY_DIR="/home/scott/workspace/ai-portland-rocketleague/ReplayAssistant/service/replays/"
SERVER_URL="http://localhost:5000/upload"

echo "Watching directory: ${REPLAY_DIR}"

# Use inotifywait to monitor the directory for new files
inotifywait -m "${REPLAY_DIR}" -e create -e close_write |
while read -r directory event file; do
    # Check if the file has the correct replay extension (.replay)
    if [[ "${file}" == *.replay ]]; then
        echo "New replay detected: ${file}"
        REPLAY_PATH="${directory}${file}"

        # Upload the new replay file to the server
        curl -X POST -F "file=@${REPLAY_PATH}" ${SERVER_URL}

        if [[ $? -eq 0 ]]; then
            echo "Successfully uploaded: ${file}"
        else
            echo "Failed to upload: ${file}"
        fi
    fi
done
