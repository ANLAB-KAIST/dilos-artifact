require 'webrick'

include WEBrick

s = HTTPServer.new( :Port => 8000 )

class HelloServlet < HTTPServlet::AbstractServlet
  def do_GET(req, res)
    res['Content-Type'] = "text/html"
    res.body = "<html><body>Hello, World!.</body></html>"
  end
end

s.mount("/hello", HelloServlet)

trap(:INT){ s.shutdown }

s.start
