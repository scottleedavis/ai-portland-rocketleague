import { GoogleAIFileManager } from '@google/generative-ai/server';
import fs from 'fs';
import path from 'path';
import os from 'os';

const key = process.env.VITE_GEMINI_API_KEY;
const fileManager = new GoogleAIFileManager(key);

/**
 * Handles chunk upload for streaming data to Google Gemini.
 * Writes chunk to a temporary file before uploading.
 *
 * @param {Buffer} chunkBuffer - The raw chunk buffer
 * @param {string} sessionId - Unique identifier for the upload session
 * @returns {Object} - Response from the file upload
 */
export const handleChunkUpload = async (chunkBuffer, sessionId) => {
  const tempDir = path.join(os.tmpdir(), 'stream_chunks');
  const fileName = `chunk-${sessionId}-${Date.now()}.webm`;
  const filePath = path.join(tempDir, fileName);

  try {
    // Ensure the temporary directory exists
    if (!fs.existsSync(tempDir)) {
      fs.mkdirSync(tempDir, { recursive: true });
    }

    // Write the chunk to a temporary file
    fs.writeFileSync(filePath, chunkBuffer);

    // Upload the temporary file
    const uploadResult = await fileManager.uploadFile(filePath, {
      displayName: fileName,
      mimeType: 'video/webm',
    });

    // Cleanup: Remove the temporary file
    fs.unlinkSync(filePath);

    if (!uploadResult || uploadResult.error) {
      console.error('Chunk upload failed:', uploadResult.error || 'Unknown error');
      throw new Error(uploadResult.error || 'Chunk upload failed.');
    }

    return uploadResult.file;
  } catch (error) {
    console.error('Error processing chunk:', error);
    // Cleanup: Ensure the file is removed in case of error
    if (fs.existsSync(filePath)) {
      fs.unlinkSync(filePath);
    }
    throw error;
  }
};
