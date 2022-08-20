require 'webrick'
require 'webrick/https'

server = WEBrick::HTTPServer.new(
  :BindAddress => '127.0.0.1',
  :Port => 8000,
  :DocumentRoot => './',
  :SSLEnable => true,
  :SSLCertName => [ [ 'CN', WEBrick::Utils::getservername ] ])
Signal.trap('INT') { server.shutdown }
server.start
