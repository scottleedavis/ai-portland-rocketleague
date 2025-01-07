from flask import Flask, request
import subprocess
import shutil
import os
import urllib.parse
import time

app = Flask(__name__)
replays_dest = "./replays" 

if not os.path.exists(replays_dest):
    os.makedirs(replays_dest)

@app.route('/replays/<guid>', methods=['GET'])
def handle_replay(guid):

    print("Got request for ",guid)
    destination_file = os.path.join(replays_dest, guid + ".replay")

    time.sleep(1)
    try:
        if not os.path.isfile(destination_file):
            return f"Replay file for GUID {guid} not found at {destination_file}.", 404
    except Exception as e:
        return f"Error accessing file: {str(e)}", 500

    command = ["./ReplayAssistant", "prepare", destination_file]
    
    try:
        env = os.environ.copy()
        
        print("Running ReplayAssistant prepare...")
        result = subprocess.run(command, check=True, env=env, capture_output=True, text=True)
        print(result)
        
        # Split the output into lines and get the last one (which should be the assistant_id)
        output_lines = result.stdout.splitlines()
        thread_id = output_lines[-1] if output_lines else "Unknown"
        print("ReplayAssistant Thread ID ",thread_id)
        return f"{thread_id}", 200

    except subprocess.CalledProcessError as e:
        return f"Command failed with error: {e}", 500



@app.route('/messages/<thread_id>/', methods=['GET'])
def messages(thread_id):

    print("Got request for ",thread_id)
    command = ["./ReplayAssistant", "messages", thread_id]
    
    try:
        env = os.environ.copy()
        
        print("Running ReplayAssistant messages...")
        result = subprocess.run(command, check=True, env=env, capture_output=True, text=True)
        print("ReplayAssistant Thread Messages ",result.stdout)
        return f"{result.stdout}", 200

    except subprocess.CalledProcessError as e:
        return f"Command failed with error: {e}", 500


@app.route('/query/<assistant_id>/<thread_id>/<query>', methods=['GET'])
def query(assistant_id,thread_id, query):

    decoded_query = urllib.parse.unquote(query)

    print("Got request for ",assistant_id,thread_id, decoded_query)
    command = ["./ReplayAssistant", "prompt", assistant_id,thread_id, decoded_query]
    
    try:
        env = os.environ.copy()
        
        print("Running ReplayAssistant prompt...")
        result = subprocess.run(command, check=True, env=env, capture_output=True, text=True)
        print("ReplayAssistant Thread Messages ",result)
        return f"{result}", 200

    except subprocess.CalledProcessError as e:
        return f"Command failed with error: {e}", 500
    

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
