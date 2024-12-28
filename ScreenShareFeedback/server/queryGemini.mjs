import { GoogleGenerativeAI } from "@google/generative-ai";

const key = process.env.GEMINI_API_KEY;
const genAI = new GoogleGenerativeAI(key);

export const handleQueryGemini = async (fileMetadata, prompt) => {
    try {
        const request = [
            { text: prompt },
            {
                fileData: {
                    mimeType: fileMetadata.mimeType,
                    fileUri: fileMetadata.name,
                },
            },
        ];

        const result = await genAI.getGenerativeModel({ model: "gemini-2.0-flash-exp" }).generateContent(request);

        if (result.error) {
            throw new Error(result.error);
        }

        return result.response;
    } catch (error) {
        console.error("Error querying Gemini:", error);
        throw error;
    }
};
