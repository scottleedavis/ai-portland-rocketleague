from flask import Flask, request, Response
import subprocess
import shutil
import os
import urllib.parse
import requests

app = Flask(__name__)

# Run 'whoami' and get the username
username = subprocess.check_output("whoami", shell=True).decode().strip().split("\\")[-1]

# Construct the path using the username
replays_path = f"/mnt/c/Users/{username}/OneDrive/Documents/My Games/Rocket League/TAGame/DemosEpic/"
replays_dest = "./replays"
proxy_dest_server = "https://zvdqtqcyvuw7.share.zrok.io"  # Replace with actual destination server address

if not os.path.exists(replays_dest):
    os.makedirs(replays_dest)

@app.route('/replays/<guid>', methods=['GET'])
def handle_replay(guid):

    print("Got request for ", guid)
    source_file = os.path.join(replays_path, guid + ".replay")
    destination_file = os.path.join(replays_dest, guid + ".replay")

    try:
        if not os.path.isfile(source_file):
            return f"Replay file for GUID {guid} not found at {source_file}.", 404

        shutil.copy(source_file, destination_file)
    except Exception as e:
        return f"Error copying file: {str(e)}", 500

    # Proxy the request to the destination server
    try:
        response = requests.get(f"{proxy_dest_server}/replays/{guid}")
        return Response(response.content, status=response.status_code, content_type=response.headers.get('Content-Type'))
    except requests.RequestException as e:
        return f"Error proxying request: {str(e)}", 500


@app.route('/messages/<thread_id>/', methods=['GET'])
def messages(thread_id):

    print("Got request for ", thread_id)
    try:
        response = requests.get(f"{proxy_dest_server}/messages/{thread_id}/")
        return Response(response.content, status=response.status_code, content_type=response.headers.get('Content-Type'))
    except requests.RequestException as e:
        return f"Error proxying request: {str(e)}", 500


@app.route('/query/<assistant_id>/<thread_id>/<query>', methods=['GET'])
def query(assistant_id, thread_id, query):

    decoded_query = urllib.parse.unquote(query)

    print("Got request for ", assistant_id, thread_id, decoded_query)
    try:
        response = requests.get(f"{proxy_dest_server}/query/{assistant_id}/{thread_id}/{urllib.parse.quote(decoded_query)}")
        return Response(response.content, status=response.status_code, content_type=response.headers.get('Content-Type'))
    except requests.RequestException as e:
        return f"Error proxying request: {str(e)}", 500


if __name__ == '__main__':
    app.run(host='localhost', port=5000)
