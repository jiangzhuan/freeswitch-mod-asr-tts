#!/bin/bash
# Build mod_asr_tts in Docker container

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "Building mod_asr_tts in Docker..."

docker build -t freeswitch-mod-asr-tts-builder -f "$SCRIPT_DIR/Dockerfile.build" "$SCRIPT_DIR"

docker run --rm \
    -v "$PROJECT_ROOT/src:/build/src:ro" \
    -v "$PROJECT_ROOT/output:/build/output" \
    freeswitch-mod-asr-tts-builder \
    bash -c "cd /build/src && make clean && make && cp mod_asr_tts.so /build/output/"

if [ -f "$PROJECT_ROOT/output/mod_asr_tts.so" ]; then
    echo "Build successful: output/mod_asr_tts.so"
else
    echo "Build failed"
    exit 1
fi