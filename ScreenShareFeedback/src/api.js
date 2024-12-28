export async function sendChunkToApi(chunkBuffer, sessionId) {
  try {
    const formData = new FormData();
    formData.append('chunk', chunkBuffer, 'capture.webm');
    formData.append('sessionId', sessionId);

    const response = await fetch('/api/stream-chunk', {
      method: 'POST',
      body: formData,
    });

    if (!response.ok) {
      const error = await response.json();
      throw new Error(error.error || 'Failed to upload chunk');
    }

    const result = await response.json();
    console.log('Chunk uploaded:', result);
    return result;
  } catch (error) {
    console.error('Error uploading chunk:', error);
  }
}

export const queryGeminiApi = async (uploadResult, prompt) => {
  try {
    const response = await fetch('/api/query-gemini', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ uploadResult, prompt }),
    });

    if (!response.ok) throw new Error('Failed to query Gemini.');

    const data = await response.json();
    return data;
  } catch (error) {
    console.error('Error querying Gemini:', error);
    throw error;
  }
};
