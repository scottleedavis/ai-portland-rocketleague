const baseUrl = "http://localhost:8000/api";

export const sendChunkToApi = async (chunk, sessionId) => {
  try {
    const response = await fetch(`${baseUrl}/stream-chunk`, {
      method: "POST",
      headers: { "Content-Type": "application/octet-stream", "Session-Id": sessionId },
      body: chunk,
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
