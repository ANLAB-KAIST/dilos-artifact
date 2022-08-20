require 'socket'

gs = TCPServer.open(8000)
socks = [gs]
addr = gs.addr
addr.shift
printf("server is on %s\n", addr.join(":"))

while true
  nsock = select(socks)
  next if nsock == nil
  for s in nsock[0]
    if s == gs
      socks.push(s.accept)
      print(s, " is accepted\n")
    else
      if s.eof?
        print(s, " is gone\n")
        s.close
        socks.delete(s)
      else
        str = s.gets
        s.write(str)
      end
    end
  end
end
