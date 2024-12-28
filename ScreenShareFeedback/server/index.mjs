import express from 'express';
import ViteExpress from 'vite-express';
import multer from 'multer';
import { handleChunkUpload, queryGemini } from './upload.mjs';

const app = express();
app.use(express.json());

const upload = multer({ dest: '/tmp/' });

app.post('/api/stream-chunk', upload.single('chunk'), async (req, res) => {
  try {
    const chunkBuffer = req.file.buffer;
    const sessionId = req.body.sessionId || 'default';
    const uploadResult = await handleChunkUpload(chunkBuffer, sessionId);
    res.json({ uploadResult });
  } catch (error) {
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
