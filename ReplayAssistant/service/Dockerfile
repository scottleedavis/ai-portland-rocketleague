# Stage 1: Build the Rust binary
FROM rust:1.82-slim AS builder

# Install necessary dependencies for building Rust with OpenSSL
RUN apt-get update && apt-get install -y --no-install-recommends \
    pkg-config \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy Rust project files and build the binary
COPY . .
RUN cargo build --release

# Stage 2: Build the Python Flask application
FROM python:3.10-slim

# Install system dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    bash \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

RUN pip install flask

# Copy application code and the Rust binary from the previous stage
COPY --from=builder /app/target/release/ReplayAssistant /app/ReplayAssistant
COPY . .

# Ensure scripts and binaries are executable
RUN chmod +x ReplayAssistant
RUN chmod +x ./*.sh

# Expose the port for Flask
EXPOSE 5000


# Set the entry point to run your Flask app
CMD ["bash", "run.sh"]
