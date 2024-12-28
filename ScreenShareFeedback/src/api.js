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
