import fs from "fs";
import path from "path";
import os from "os";
import { GoogleAIFileManager } from "@google/generative-ai/server";

const fileManager = new GoogleAIFileManager(process.env.GEMINI_API_KEY);

export const handleChunkUpload = async (chunkBuffer, sessionId) => {
  const tempDir = path.join(os.tmpdir(), "stream_chunks");
  const fileName = `chunk-${sessionId}-${Date.now()}.webm`;
  const filePath = path.join(tempDir, fileName);

  try {
    if (!fs.existsSync(tempDir)) {
      fs.mkdirSync(tempDir, { recursive: true });
    }

    if (!chunkBuffer || !(chunkBuffer instanceof Buffer)) {
      throw new Error("Invalid chunk buffer received");
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

    // console.log("Upload result:", uploadResult);

    // Validate and format fileUri
    if (!uploadResult.file.uri || !uploadResult.file.uri.startsWith("https://")) {
      throw new Error(`Invalid or unsupported file URI: ${uploadResult.uri}`);
    }

    return uploadResult.file;
  } catch (error) {
    console.error("Error processing chunk:", error);
    if (fs.existsSync(filePath)) {
      fs.unlinkSync(filePath);
    }
    throw error;
  }
}

export const checkProgress = async fileId => {
  const fileName = fileId.includes("files/")
    ? fileId.split("files/")[1]
    : fileId;
  try {
    // console.log(fileName);
    const result = await fileManager.getFile(fileName);
    // console.log(result);
    return result
  } catch (error) {
    console.error(error)
    return {error}
  }
}
