import React, { useRef, useState } from "react";
import { sendChunkToApi, queryGemini } from "./api";

const VideoPlayer = () => {
  const [geminiResponse, setGeminiResponse] = useState(null);
  const [isRecording, setIsRecording] = useState(false);
  const mediaRecorder = useRef(null);
  const videoRef = useRef(null);
  const sessionId = useRef(Date.now().toString());

  const startRecording = async () => {
    try {
      console.log("Requesting screen share...");
      const stream = await navigator.mediaDevices.getDisplayMedia({ video: true });
      console.log("Screen share stream received:", stream);

      if (videoRef.current) {
        videoRef.current.srcObject = stream;
        videoRef.current.play();
      }

      mediaRecorder.current = new MediaRecorder(stream, {
        mimeType: "video/webm;codecs=vp8",
      });

      mediaRecorder.current.ondataavailable = async (event) => {
        console.log("Received data from MediaRecorder:", event.data);

        if (event.data && event.data.size > 0) {
          console.log("Sending chunk with size:", event.data.size);

          try {
            const uploadResult = await sendChunkToApi(event.data, sessionId.current);
            console.log("Chunk uploaded successfully:", uploadResult);

            // Query Gemini after chunk is uploaded
            const prompt = "Analyze this video stream for feedback.";
            const geminiResult = await queryGemini(uploadResult, prompt);
            setGeminiResponse(geminiResult);
            console.log("Gemini response:", geminiResult);
          } catch (error) {
            console.error("Error processing chunk:", error);
          }
        } else {
          console.warn("Received empty chunk from MediaRecorder.");
        }
      };

      mediaRecorder.current.start(1000); // Collect chunks every second
      setIsRecording(true);
      console.log("Recording started.");
    } catch (error) {
      console.error("Error starting screen share or recording:", error);
    }
  };

  const stopRecording = () => {
    if (mediaRecorder.current && mediaRecorder.current.state !== "inactive") {
      mediaRecorder.current.stop();
      setIsRecording(false);
      console.log("Recording stopped.");
    }
  };

  return (
    <div>
      <h1>Video Recorder</h1>
      <video ref={videoRef} style={{ width: "100%", height: "auto", backgroundColor: "black" }} autoPlay muted></video>
      <div>
        {!isRecording ? (
          <button onClick={startRecording}>Start Recording</button>
        ) : (
          <button onClick={stopRecording}>Stop Recording</button>
        )}
      </div>
      {geminiResponse && (
        <div>
          <h2>Gemini Response</h2>
          <pre>{JSON.stringify(geminiResponse, null, 2)}</pre>
        </div>
      )}
    </div>
  );
};

export default VideoPlayer;
