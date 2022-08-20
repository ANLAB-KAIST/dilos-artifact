require 'socket'
require 'openssl'

port = 443
host = 'www.google.com'

msg = "GET / HTTP/1.1\r\n" \
     + "Host: www.google.co.jp\r\n" \
     + "Connection: close\r\n" \
     + "\r\n"

context = OpenSSL::SSL::SSLContext.new
socket = TCPSocket.new host, port
ssl_client = OpenSSL::SSL::SSLSocket.new socket, context
ssl_client.connect
ssl_client.puts msg
ret = ssl_client.read

puts ret
