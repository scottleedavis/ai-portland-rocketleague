from flask import Flask, request
import subprocess
import shutil
import os

app = Flask(__name__)

replays_path = "/mnt/c/Users/gamez/OneDrive/Documents/My Games/Rocket League/TAGame/DemosEpic/"
replays_dest = "./replays" 

if not os.path.exists(replays_dest):
    os.makedirs(replays_dest)

@app.route('/replays/<guid>', methods=['GET'])
def handle_replay(guid):

    print("Got request for ",guid)
    source_file = os.path.join(replays_path, guid + ".replay")
    destination_file = os.path.join(replays_dest, guid + ".replay")

    try:
        if not os.path.isfile(source_file):
            return f"Replay file for GUID {guid} not found at {source_file}.", 404
        
        shutil.copy(source_file, destination_file)
    except Exception as e:
        return f"Error copying file: {str(e)}", 500

    command = ["./ReplayAssistant", "prepare", destination_file]
    
    try:
        env = os.environ.copy()
        
        print("Running ReplayAssistant...")
        result = subprocess.run(command, check=True, env=env, capture_output=True, text=True)
        print(result)
        
        # Split the output into lines and get the last one (which should be the assistant_id)
        output_lines = result.stdout.splitlines()
        assistant_id = output_lines[-1] if output_lines else "Unknown"
        print("ReplayAssistant ID ",assistant_id)
        return f"{assistant_id}", 200

    except subprocess.CalledProcessError as e:
        return f"Command failed with error: {e}", 500


@app.route('/query/<guid>/<query>', methods=['GET'])
def handle_query(guid,query):
    # # Construct the source and destination file paths
    # source_file = os.path.join(replays_path, guid + ".replay")
    # destination_file = os.path.join(replays_dest, guid + ".replay")

    # # Copy the replay file to the ./replays directory
    # try:
    #     # Check if the source file exists
    #     if not os.path.isfile(source_file):
    #         return f"Replay file for GUID {guid} not found at {source_file}.", 404
        
    #     shutil.copy(source_file, destination_file)
    # except Exception as e:
    #     return f"Error copying file: {str(e)}", 500

    # # Construct the command with the replay file in the ./replays directory
    # command = ["./ReplayAssistant", "ai", prompt]
    
    # # Run the command
    # try:
    #     env = os.environ.copy()
    #     subprocess.run(command, check=True, env=env) 
    # except subprocess.CalledProcessError as e:
    #     return f"Command failed with error: {e}", 500
    
    return f"Received request for replay with GUID: {guid} and query: {query}", 200


if __name__ == '__main__':
    app.run(host='localhost', port=5000)
