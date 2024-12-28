import React, { useEffect, useRef, useState } from 'react';
import { sendChunkToApi, queryGemini } from './api';

const VideoPlayer = () => {
  const videoRef = useRef(null);
  const mediaRecorder = useRef(null);
  const [sessionId, setSessionId] = useState('');
  const [geminiResponse, setGeminiResponse] = useState(null);

  useEffect(() => {
    const initializeStream = async () => {
      try {
        const stream = await navigator.mediaDevices.getDisplayMedia({
          video: true,
          audio: true,
        });

        if (videoRef.current) {
          videoRef.current.srcObject = stream;
        }

        mediaRecorder.current = new MediaRecorder(stream, { mimeType: 'video/webm' });

        mediaRecorder.current.ondataavailable = async (event) => {
          if (event.data && event.data.size > 0) {
            console.log('Sending chunk with size:', event.data.size);
            try {
              const uploadResult = await sendChunkToApi(event.data, sessionId);
              console.log('Chunk uploaded successfully:', uploadResult);

              // Query Gemini after a chunk is uploaded
              const prompt = 'Analyze this video stream for feedback';
              const geminiResult = await queryGemini(uploadResult, prompt);
              setGeminiResponse(geminiResult);
              console.log('Gemini response:', geminiResult);
            } catch (error) {
              console.error('Error processing chunk:', error);
            }
          } else {
            console.warn('Received empty chunk from MediaRecorder.');
          }
        };
      } catch (error) {
        console.error('Error initializing stream:', error);
      }
    };

    initializeStream();

    return () => {
      if (mediaRecorder.current) {
        mediaRecorder.current.ondataavailable = null;
      }
    };
  }, [sessionId]);

  const startRecording = () => {
    if (mediaRecorder.current && mediaRecorder.current.state !== 'recording') {
      setSessionId(Date.now().toString());
      mediaRecorder.current.start(500); // Emit chunks every 500ms
      console.log('Recording started');
    } else {
      console.warn('MediaRecorder not initialized or already recording');
    }
  };

  const stopRecording = () => {
    if (mediaRecorder.current && mediaRecorder.current.state === 'recording') {
      mediaRecorder.current.stop();
      console.log('Recording stopped');
    } else {
      console.warn('MediaRecorder is not recording');
    }
  };

  return (
    <div>
      <video ref={videoRef} autoPlay muted style={{ width: '100%', border: '1px solid black' }}></video>
      <div>
        <button onClick={startRecording}>Start Recording</button>
        <button onClick={stopRecording}>Stop Recording</button>
      </div>
      {geminiResponse && (
        <div>
          <h2>Gemini Analysis:</h2>
          <pre>{JSON.stringify(geminiResponse, null, 2)}</pre>
        </div>
      )}
    </div>
  );
};

export default VideoPlayer;
