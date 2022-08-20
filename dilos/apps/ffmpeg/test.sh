#!/bin/bash

COMMAND=${1:-'default'}
THIS_DIR=$(readlink -f $(dirname $0))
CMDLINE=$($THIS_DIR/../cmdline.sh $THIS_DIR $COMMAND)

LINE='  E rawvideo'
case $COMMAND in
	video_details)
		LINE='At least one output file must be specified' ;;
	video_subclip)
		LINE='video:' ;;
	video_extract_png)
		LINE='video:' ;;
	video_transcode)
		LINE='encoded ' ;;
esac

if [ "$COMMAND" == "video_transcode" ]; then
rm -f /tmp/test.mp4
ffmpeg -i tcp://0.0.0.0:12345?listen -c copy /tmp/test.mp4 &
PID=$!
fi

$THIS_DIR/../../scripts/tests/test_app.py -e "$CMDLINE" --line "$LINE"

if [ "$PID" != "video_transcode" ] && [ -d "/proc/$PID" ]; then
kill -9 $PID
fi
