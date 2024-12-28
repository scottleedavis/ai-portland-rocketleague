//// File: src/VideoPlayer.jsx
import React, { useEffect, useRef } from 'react';
import { sendChunkToApi } from './api';

const VideoPlayer = () => {
  const videoRef = useRef(null);

  useEffect(() => {
    const startCapture = async () => {
      try {
        const stream = await navigator.mediaDevices.getDisplayMedia({
          video: true,
          audio: true,
        });
        const video = videoRef.current;
        video.srcObject = stream;

        const mediaRecorder = new MediaRecorder(stream, { mimeType: 'video/webm' });
        const chunks = [];

        mediaRecorder.ondataavailable = (event) => {
          if (event.data.size > 0) {
            chunks.push(event.data);
            sendChunkToApi(event.data);
          }
        };

        mediaRecorder.start(500); // Collect data every 500ms
      } catch (err) {
        console.error('Error starting capture:', err);
      }
    };

    startCapture();
  }, []);

  return <video ref={videoRef} autoPlay playsInline controls />;
};

export default VideoPlayer;