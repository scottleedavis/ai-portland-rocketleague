import { GoogleAIFileManager } from "@google/generative-ai/server";

const fileManager = new GoogleAIFileManager(process.env.GEMINI_API_KEY);


const doit = async () => {
    const files = await fileManager.listFiles();
    console.log(files);
}

doit();


