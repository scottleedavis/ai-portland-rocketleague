import React, { useRef, useEffect } from 'react';
import { sendChunkToApi, queryGeminiApi } from './api';

const VideoPlayer = () => {
  const videoRef = useRef(null);
  const mediaRecorder = useRef(null);

  useEffect(() => {
    const initStream = async () => {
      const stream = await navigator.mediaDevices.getDisplayMedia({ video: true, audio: true });
      videoRef.current.srcObject = stream;

      mediaRecorder.current = new MediaRecorder(stream, { mimeType: 'video/webm' });
      mediaRecorder.current.ondataavailable = async (event) => {
        try {
          const chunkBuffer = event.data;
          const uploadResult = await sendChunkToApi(chunkBuffer, 'session-id');
          const prompt = 'Analyze this chunk'; // Example prompt
          const geminiResponse = await queryGeminiApi(uploadResult, prompt);
          console.log('Gemini Response:', geminiResponse);
        } catch (error) {
          console.error('Error processing chunk:', error);
        }
      };

      mediaRecorder.current.start(500); // Send chunks every 500ms
    };

    initStream();
  }, []);

  return <video ref={videoRef} autoPlay />;
};

export default VideoPlayer;
