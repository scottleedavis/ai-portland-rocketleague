#!/bin/bash

# Function to fetch and delete files
delete_all_files() {
  # Get the list of files
  echo "Fetching list of uploaded files..."
  response=$(curl -s -X GET "https://api.openai.com/v1/files" \
    -H "Authorization: Bearer $OPENAI_API_KEY")
  
  # Check if the response contains files
  files=$(echo "$response" | jq -r '.data[] | .id')

  if [[ -z "$files" ]]; then
    echo "No files found to delete."
    exit 0
  fi

  # Iterate over each file and delete it
  echo "Deleting files..."
  for file_id in $files; do
    echo "Deleting file ID: $file_id"
    curl -s -X DELETE "https://api.openai.com/v1/files/$file_id" \
      -H "Authorization: Bearer $OPENAI_API_KEY"
  done

  echo "All files deleted."
}

# Check if jq is installed
if ! command -v jq &> /dev/null; then
  echo "Error: jq is not installed. Please install jq to use this script."
  exit 1
fi

# Run the function
delete_all_files
