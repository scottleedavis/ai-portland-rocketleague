import React, { useRef, useState, useEffect } from 'react';
import { sendChunkToApi, queryGemini } from './api';

const VideoPlayer = () => {
  const videoRef = useRef(null);
  const mediaRecorder = useRef(null);
  const [isRecording, setIsRecording] = useState(false);
  const [geminiResponse, setGeminiResponse] = useState(null);
  const sessionId = useRef(Date.now().toString());

  useEffect(() => {
    const initScreenShare = async () => {
      try {
        console.log('Requesting screen share...');
        const stream = await navigator.mediaDevices.getDisplayMedia({
          video: true,
        });

        console.log('Screen share stream received:', stream);
        if (videoRef.current) {
          videoRef.current.srcObject = stream;
        }

        mediaRecorder.current = new MediaRecorder(stream, {
          mimeType: 'video/webm;codecs=vp8',
        });

        console.log('MediaRecorder initialized:', mediaRecorder.current);

        mediaRecorder.current.ondataavailable = async (event) => {
          console.log('Data available from MediaRecorder:', event.data);
          if (event.data && event.data.size > 0) {
            console.log('Sending chunk with size:', event.data.size);
            try {
              const uploadResult = await sendChunkToApi(event.data, sessionId.current);
              console.log('Chunk uploaded successfully:', uploadResult);

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

        mediaRecorder.current.onerror = (e) => {
          console.error('MediaRecorder error:', e);
        };
      } catch (error) {
        console.error('Error initializing screen share:', error);
      }
    };

    initScreenShare();
  }, []);

  const startRecording = () => {
    if (mediaRecorder.current && mediaRecorder.current.state !== 'recording') {
      console.log('Starting MediaRecorder...');
      mediaRecorder.current.start(1000); // Emit data every second
      setIsRecording(true);
    }
  };

  const stopRecording = () => {
    if (mediaRecorder.current && mediaRecorder.current.state === 'recording') {
      console.log('Stopping MediaRecorder...');
      mediaRecorder.current.stop();
      setIsRecording(false);
    }
  };

  return (
    <div>
      <video ref={videoRef} autoPlay playsInline></video>
      <div>
        <button onClick={startRecording} disabled={isRecording}>
          Start Recording
        </button>
        <button onClick={stopRecording} disabled={!isRecording}>
          Stop Recording
        </button>
      </div>
      {geminiResponse && <pre>{JSON.stringify(geminiResponse, null, 2)}</pre>}
    </div>
  );
};

export default VideoPlayer;
