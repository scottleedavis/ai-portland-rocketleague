import express from 'express';
import ViteExpress from 'vite-express';
import multer from 'multer';
import { handleChunkUpload, queryGemini } from './upload.mjs';

const app = express();
app.use(express.json());

const upload = multer({
  storage: multer.memoryStorage(), // Store files in memory for processing
  limits: { fileSize: 50 * 1024 * 1024 }, // Set file size limit to 50MB
});

app.post('/api/stream-chunk', upload.single('chunk'), async (req, res) => {
  try {
    // Log received file
    console.log('Received file:', req.file);

    if (!req.file || !req.file.buffer) {
      throw new Error('Invalid file received.');
    }

    const chunkBuffer = req.file.buffer; // Access chunk data
    const sessionId = req.body.sessionId; // Retrieve sessionId from form data
    const uploadResult = await handleChunkUpload(chunkBuffer, sessionId);

    res.status(200).json(uploadResult);
  } catch (error) {
    console.error('Error processing chunk:', error);
    res.status(500).json({ error: error.message });
  }
});


app.post('/api/query-gemini', async (req, res) => {
  try {
    const { uploadResult, prompt } = req.body;
    const geminiResponse = await queryGemini(uploadResult, prompt);
    res.json(geminiResponse);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

const port = process.env.NODE_ENV === 'production' ? 8080 : 8000;

ViteExpress.listen(app, port, () => console.log('Server is listening...'));
