import { GoogleAIFileManager } from "@google/generative-ai/server";
import { GoogleGenerativeAI } from "@google/generative-ai";

const fileManager = new GoogleAIFileManager(process.env.GEMINI_API_KEY);
const genAI = new GoogleGenerativeAI(process.env.GEMINI_API_KEY);

/**
 * Polls the file status until it becomes ACTIVE.
 * @param {string} fileName - The name of the uploaded file.
 * @param {number} timeout - Maximum time (in ms) to wait for the file to become ACTIVE.
 * @param {number} interval - Polling interval (in ms).
 * @returns {Promise<Object>} Resolves with file metadata when ACTIVE or rejects on timeout.
 */
const waitForFileToBeActive = async (fileNameOrUri, timeout = 30000, interval = 2000) => {

    const fileName = fileNameOrUri.includes("files/")
        ? fileNameOrUri.split("files/")[1]
        : fileNameOrUri;
    const startTime = Date.now();

    while (Date.now() - startTime < timeout) {
        try {
            console.log(fileName);
            const fileMetadata = await fileManager.getFile(fileName);

            if (fileMetadata.state === "ACTIVE") {
                console.log("File is ACTIVE:", fileMetadata);
                return fileMetadata;
            }

            console.log(`File is in state: ${fileMetadata.state}, retrying...`);
        } catch (error) {
            console.warn("Error fetching file status. Retrying...", error);
        }

        await new Promise((resolve) => setTimeout(resolve, interval));
    }

    throw new Error(`File ${fileName} did not become ACTIVE within ${timeout}ms`);
};

/**
 * Queries Gemini for analysis using an uploaded file.
 * @param {Object} uploadResult - Metadata of the uploaded file.
 * @param {string} prompt - Prompt to query Gemini.
 * @returns {Promise<Object>} Gemini response.
 */
export const handleQueryGemini = async (uploadResult, prompt) => {
    try {
        console.log("Received uploadResult for querying Gemini:", uploadResult);

        // Wait for the file to become ACTIVE
        const activeFile = await waitForFileToBeActive(uploadResult.name);

        // Query Gemini with the active file
        const response = await genAI
            .getGenerativeModel({ model: "gemini-2.0-flash-exp" })
            .generateContent([
                { text: prompt },
                {
                    fileData: {
                        mimeType: activeFile.mimeType,
                        fileUri: activeFile.uri,
                    },
                },
            ]);

        console.log("Gemini response:", response);
        return response;
    } catch (error) {
        console.error("Error querying Gemini:", error);
        throw error;
    }
};
