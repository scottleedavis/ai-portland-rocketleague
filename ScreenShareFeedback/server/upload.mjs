import { GoogleAIFileManager } from '@google/generative-ai/server';
import { GoogleGenerativeAI } from '@google/generative-ai';

import fs from 'fs';
import path from 'path';
import os from 'os';


const key = process.env.VITE_GEMINI_API_KEY;
const fileManager = new GoogleAIFileManager(key);
const genAI = new GoogleGenerativeAI(key);

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
    if (!chunkBuffer || !Buffer.isBuffer(chunkBuffer)) {
      throw new TypeError('The "chunkBuffer" argument must be a Buffer.');
    }

    if (!fs.existsSync(tempDir)) {
      fs.mkdirSync(tempDir, { recursive: true });
    }

    fs.writeFileSync(filePath, chunkBuffer);

    const uploadResult = await fileManager.uploadFile(filePath, {
      displayName: fileName,
      mimeType: 'video/webm',
    });

    fs.unlinkSync(filePath);

    if (!uploadResult || uploadResult.error) {
      console.error('Chunk upload failed:', uploadResult.error || 'Unknown error');
      throw new Error(uploadResult.error || 'Chunk upload failed.');
    }

    return uploadResult.file;
  } catch (error) {
    if (fs.existsSync(filePath)) {
      fs.unlinkSync(filePath);
    }
    console.error('Error processing chunk:', error);
    throw error;
  }
};

// export const handleChunkUpload = async (chunkBuffer, sessionId) => {
//   const tempDir = path.join(os.tmpdir(), 'stream_chunks');
//   const fileName = `chunk-${sessionId}-${Date.now()}.webm`;
//   const filePath = path.join(tempDir, fileName);

//   try {
//     // Ensure the temporary directory exists
//     if (!fs.existsSync(tempDir)) {
//       fs.mkdirSync(tempDir, { recursive: true });
//     }

//     // Write the chunk to a temporary file
//     fs.writeFileSync(filePath, chunkBuffer);

//     // Upload the temporary file
//     const uploadResult = await fileManager.uploadFile(filePath, {
//       displayName: fileName,
//       mimeType: 'video/webm',
//     });

//     // Cleanup: Remove the temporary file
//     fs.unlinkSync(filePath);

//     if (!uploadResult || uploadResult.error) {
//       console.error('Chunk upload failed:', uploadResult.error || 'Unknown error');
//       throw new Error(uploadResult.error || 'Chunk upload failed.');
//     }

//     return uploadResult.file;
//   } catch (error) {
//     console.error('Error processing chunk:', error);
//     // Cleanup: Ensure the file is removed in case of error
//     if (fs.existsSync(filePath)) {
//       fs.unlinkSync(filePath);
//     }
//     throw error;
//   }
// };

/**
 * Queries Gemini with uploaded file data and user prompt.
 *
 * @param {Object} uploadResult - File metadata returned from the upload
 * @param {string} prompt - User's query to Gemini
 * @returns {Object} - Response from Gemini
 */
export const queryGemini = async (uploadResult, prompt) => {
  try {
    const req = [
      { text: prompt },
      {
        fileData: {
          mimeType: uploadResult.mimeType,
          fileUri: uploadResult.uri,
        },
      },
    ];

    const result = await genAI.getGenerativeModel({ model: 'gemini-2.0-flash-exp' }).generateContent(req);

    if (result.error) {
      console.error('Error querying Gemini:', result.error);
      throw new Error(result.error);
    }

    return {
      text: result.response.text(),
      candidates: result.response.candidates,
      feedback: result.response.promptFeedback,
    };
  } catch (error) {
    console.error('Error querying Gemini:', error);
    throw error;
  }
};
