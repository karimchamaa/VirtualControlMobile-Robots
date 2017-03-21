raspivid -o - -t 0 -hf -n -w 640 -h 480 -fps 30 | cvlc -vvv stream:///dev/stdin --sout '#rtp{sdp=rtsp://:8554/x}' :demux=h264

