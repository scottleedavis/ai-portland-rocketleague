import { GoogleAIFileManager } from "@google/generative-ai/server";
import { GoogleGenerativeAI } from "@google/generative-ai";

/**
 * Queries Gemini for analysis using an uploaded file.
 * @param {Object} uploadResult - Metadata of the uploaded file.
 * @param {string} prompt - Prompt to query Gemini.
 * @returns {Promise<Object>} Gemini response.
 */
export const handleQueryGemini = async (uploadResult, prompt) => {
    try {
        const fileManager = new GoogleAIFileManager(process.env.GEMINI_API_KEY);
        const genAI = new GoogleGenerativeAI(process.env.GEMINI_API_KEY);

        console.log("Received uploadResult for querying Gemini:", uploadResult);

        // Wait for the file to become ACTIVE
        // const activeFile = await waitForFileToBeActive(uploadResult.name);

        // Query Gemini with the active file
        const response = await genAI
            .getGenerativeModel({ model: "gemini-2.0-flash-exp" })
            .generateContent([
                { text: prompt },
                {
                    fileData: {
                        mimeType: uploadResult.mimeType,
                        fileUri: uploadResult.uri,
                    },
                },
            ]);

        console.log("Gemini response:", response);

        const deleteFile = uploadResult.name.includes("files/")
            ? uploadResult.name.split("files/")[1]
            : uploadResult.name;
        fileManager.deleteFile(deleteFile);

        return response;
    } catch (error) {
        console.error("Error querying Gemini:", error);
        throw error;
    }
};
