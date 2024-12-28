import fs from "fs";
import path from "path";
import os from "os";
import { GoogleAIFileManager } from "@google/generative-ai/server";

const key = process.env.GEMINI_API_KEY;
const fileManager = new GoogleAIFileManager(key);

export const handleChunkUpload = async (chunkBuffer, sessionId) => {
  const tempDir = path.join(os.tmpdir(), "stream_chunks");
  const fileName = `chunk-${sessionId}-${Date.now()}.webm`;
  const filePath = path.join(tempDir, fileName);

  try {
    if (!fs.existsSync(tempDir)) {
      fs.mkdirSync(tempDir, { recursive: true });
    }

    fs.writeFileSync(filePath, chunkBuffer);

    const uploadResult = await fileManager.uploadFile(filePath, {
      displayName: fileName,
      mimeType: "video/webm",
    });

    fs.unlinkSync(filePath);

    if (!uploadResult || uploadResult.error) {
      throw new Error(uploadResult.error || "Chunk upload failed.");
    }

    return uploadResult.file;
  } catch (error) {
    console.error("Error processing chunk:", error);
    if (fs.existsSync(filePath)) {
      fs.unlinkSync(filePath);
    }
    throw error;
  }
};
