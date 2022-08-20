from osv.modules import api

api.require("libz")

default = api.run("/ffmpeg.so -formats")
video_details = api.run('/ffmpeg.so -i http://clips.vorwaerts-gmbh.de/VfE_html5.mp4')
video_subclip = api.run('/ffmpeg.so -i http://clips.vorwaerts-gmbh.de/VfE_html5.mp4 -ss 00:00:10 -codec copy -t 10 /tmp/output.mp4')
video_extract_png = api.run('/ffmpeg.so -i http://clips.vorwaerts-gmbh.de/VfE_html5.mp4 -r 1 -f image2 /tmp/image-%2d.png')
video_transcode = api.run('/ffmpeg.so -i http://clips.vorwaerts-gmbh.de/VfE_html5.mp4 -c:v libx265 -crf 28 -an -f mpegts tcp://192.168.122.1:12345')
