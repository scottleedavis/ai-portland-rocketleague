//// File: server/index.mjs
import express from 'express';
import multer from 'multer';
import { handleChunkUpload } from './upload.mjs';
import ViteExpress from 'vite-express'

const app = express();
const upload = multer();

app.use(express.json());

app.post('/api/stream-chunk', upload.single('chunk'), async (req, res) => {
  try {
    const chunk = req.file;
    const result = await handleChunkUpload(chunk);
    res.json(result);
  } catch (err) {
    console.error(err);
    res.status(500).send({ error: 'Failed to process chunk.' });
  }
});

const port = process.env.NODE_ENV === 'production' ? 8080 : 8000

ViteExpress.listen(app, port, () => console.log('Server is listening...'))
