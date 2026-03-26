#!/bin/bash
# Run integration tests in Docker container

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "Starting FreeSWITCH test environment..."

# Build test image
docker build -t freeswitch-mod-asr-tts-test -f "$SCRIPT_DIR/Dockerfile.test" "$SCRIPT_DIR"

# Create test config
mkdir -p "$PROJECT_ROOT/test/conf"
cat > "$PROJECT_ROOT/test/conf/mod_asr_tts.conf.xml" << 'EOF'
<configuration name="mod_asr_tts.conf" description="ASR/TTS Module Configuration">
  <settings>
    <param name="appkey" value="test_appkey"/>
    <param name="token" value="test_token"/>
    <param name="websocket-url" value="wss://nls-gateway.cn-shanghai.aliyuncs.com/ws/v1"/>
    <param name="sample-rate" value="16000"/>
    <param name="vad-threshold" value="-40"/>
    <param name="thread-pool-size" value="4"/>
  </settings>
</configuration>
EOF

# Run test container
docker run --rm -d --name freeswitch-test \
    -v "$PROJECT_ROOT/output:/data/mod" \
    -v "$PROJECT_ROOT/test/conf:/data/conf" \
    -p 5060:5060/udp \
    -p 8021:8021/tcp \
    freeswitch-mod-asr-tts-test

echo "FreeSWITCH started. Connect with: fs_cli -H localhost -P 8021 -p ClueCon"
echo "To load module: load mod_asr_tts"
echo "To stop: docker stop freeswitch-test"