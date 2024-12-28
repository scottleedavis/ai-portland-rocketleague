/// File: server/upload.mjs
import { GoogleAIFileManager } from '@google/generative-ai/server';

const key = process.env.VITE_GEMINI_API_KEY;
const fileManager = new GoogleAIFileManager(key);

export const handleChunkUpload = async (chunk) => {
  const uploadResult = await fileManager.uploadFile(chunk.buffer, {
    displayName: 'chunk.webm',
    mimeType: 'video/webm',
  });
  return uploadResult.file;
};