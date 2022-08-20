Open source audio & video processing framework - https://www.ffmpeg.org/

The default command prints list of supported formats.

Examples:
- get details of video - ./scripts/run.py -e '/ffmpeg.so -i http://clips.vorwaerts-gmbh.de/VfE_html5.mp4
- extract 10 seconds of video starting at 10 seconds offset - ./scripts/run.py -e '/ffmpeg.so -i http://clips.vorwaerts-gmbh.de/VfE_html5.mp4 -ss 00:00:10 -codec copy -t 10 output.mp4'
- extract png image for each second of video - ./scripts/run.py -e '/ffmpeg.so -i http://clips.vorwaerts-gmbh.de/VfE_html5.mp4 -r 1 -f image2 image-%2d.png

For more examples read https://www.ostechnix.com/20-ffmpeg-commands-beginners/

There are number of test videos of different formats you can use from here - https://standaloneinstaller.com/blog/big-list-of-sample-videos-for-testers-124.html
