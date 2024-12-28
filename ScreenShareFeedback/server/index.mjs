import express from 'express';
import ViteExpress from 'vite-express';
import multer from 'multer';
import { handleChunkUpload } from './upload.mjs';

const app = express();
app.use(express.json());

const upload = multer({ storage: multer.memoryStorage() });

app.post('/api/stream-chunk', upload.single('chunk'), async (req, res) => {
  try {
    const sessionId = req.body.sessionId;
    const chunkBuffer = req.file.buffer;

    const result = await handleChunkUpload(chunkBuffer, sessionId);
    res.json({ fileUri: result.uri });
  } catch (error) {
    console.error('Error in /api/stream-chunk:', error);
    res.status(500).json({ error: 'Failed to process chunk.' });
  }
});

// app.post('/api/stream-complete', async (req, res) => {
//   try {
//     const { fileUri, prompt } = req.body;
//     const result = await streamToGemini(fileUri, prompt);
//     res.json(result);
//   } catch (error) {
//     console.error('Error in /api/stream-complete:', error);
//     res.status(500).json({ error: 'Failed to complete stream.' });
//   }
// });

const port = process.env.NODE_ENV === 'production' ? 8080 : 8000;

ViteExpress.listen(app, port, () => console.log('Server is listening...'));
