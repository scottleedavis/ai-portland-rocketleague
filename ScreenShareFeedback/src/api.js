const API_BASE_URL = 'http://localhost:8000';

export const sendChunkToApi = async (chunk, sessionId) => {
  if (!chunk || chunk.size === 0) {
    console.warn('Cannot send empty or invalid chunk');
    return;
  }

  const formData = new FormData();
  formData.append('chunk', chunk);
  formData.append('sessionId', sessionId);

  try {
    const response = await fetch(`${API_BASE_URL}/api/stream-chunk`, {
      method: 'POST',
      body: formData,
    });

    if (!response.ok) {
      throw new Error(`HTTP error! Status: ${response.status}`);
    }

    const data = await response.json();
    console.log('Chunk uploaded:', data);
    return data;
  } catch (error) {
    console.error('Error sending chunk to API:', error);
    throw error;
  }
};

export const queryGemini = async (uploadResult, prompt) => {
  if (!uploadResult || !uploadResult.uri) {
    throw new Error('Invalid upload result for querying Gemini');
  }

  const requestPayload = {
    uploadResult,
    prompt,
  };

  try {
    const response = await fetch(`${API_BASE_URL}/api/query-gemini`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(requestPayload),
    });

    if (!response.ok) {
      throw new Error(`HTTP error! Status: ${response.status}`);
    }

    const data = await response.json();
    console.log('Gemini query response:', data);
    return data;
  } catch (error) {
    console.error('Error querying Gemini:', error);
    throw error;
  }
};
