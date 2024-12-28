//// File: src/api.js
export const sendChunkToApi = async (chunk) => {
  const formData = new FormData();
  formData.append('chunk', chunk);

  try {
    const response = await fetch('http://localhost:8000/api/stream-chunk', {
      method: 'POST',
      body: formData,
    });
    const result = await response.json();
    console.log('Chunk uploaded:', result);
  } catch (err) {
    console.error('Error uploading chunk:', err);
  }
};