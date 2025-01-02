#!/bin/bash

# Function to fetch and delete assistants
delete_all_assistants() {
  # Get the list of assistants
  echo "Fetching list of assistants..."
  response=$(curl -s -X GET "https://api.openai.com/v1/assistants" \
    -H "Authorization: Bearer $OPENAI_API_KEY" \
    -H "OpenAI-Beta: assistants=v2")

echo $response
  # Check if the response contains assistants
  assistants=$(echo "$response" | jq -r '.data[] | .id')

  if [[ -z "$assistants" ]]; then
    echo "No assistants found to delete."
    exit 0
  fi

  # Iterate over each assistant and delete it
  echo "Deleting assistants..."
  for assistant_id in $assistants; do
    echo "Deleting assistant ID: $assistant_id"
    curl -s -X DELETE "https://api.openai.com/v1/assistants/$assistant_id" \
      -H "Authorization: Bearer $OPENAI_API_KEY"  \
      -H "OpenAI-Beta: assistants=v2"
  done

  echo "All assistants deleted."
}

# Check if jq is installed
if ! command -v jq &> /dev/null; then
  echo "Error: jq is not installed. Please install jq to use this script."
  exit 1
fi

# Run the function
delete_all_assistants
