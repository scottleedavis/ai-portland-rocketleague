#!/bin/bash


echo "source venv/bin/activate reminder"
python server.py

./delete_all_files.sh
./delete_all_assistants.sh
