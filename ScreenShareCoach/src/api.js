const baseUrl = "http://localhost:8000/api";

// const RECORD_FPS = 2;
// const RECORD_CHUNK_MS = 1000 / RECORD_FPS;

export const sendChunk = async (chunk, sessionId) => {
  // console.log("Sending chunk to API:", { size: chunk.size, type: chunk.type });

  const formData = new FormData();
  formData.append("chunk", chunk);
  formData.append("sessionId", sessionId);

  try {
    const response = await fetch(`${baseUrl}/stream-chunk`, {
      method: "POST",
      body: formData,
    });

    if (!response.ok) {
      throw new Error(`HTTP error! Status: ${response.status}`);
    }

    return await response.json();
  } catch (error) {
    console.error("Error sending chunk to API:", error);
    throw error;
  }
};

export const checkChunkProgress = async fileId => {

  const response = await fetch(`${baseUrl}/progress`, {
    method: 'POST',
    headers: {
      Accept: 'application/json',
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({ fileId })
  });
  const resp = await response.json();

  // console.log(resp.progress);

  if (resp.progress.state === 'ACTIVE') {
    // console.log('active found', resp.progress);
    // setIsLoadingVideo(false)
  } else if (resp.progress.state === 'FAILED') {
    // setVideoError(true);
    // console.log('chunk failed', resp.progress);
  } else if (resp.progress.state == 'PROCESSING') {
    // console.log('chunk processing', resp.progress);
    // setTimeout(() => checkChunkProgress(fileId), RECORD_CHUNK_MS);

  }
  return resp.progress;

}

export const queryGemini = async (fileMetadata, prompt) => {
  try {
    const response = await fetch(`${baseUrl}/query-gemini`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ fileMetadata, prompt }),
    });

    if (!response.ok) {
      throw new Error(`HTTP error! Status: ${response.status}`);
    }

    return await response.json();
  } catch (error) {
    console.error("Error querying Gemini:", error);
    throw error;
  }
};
