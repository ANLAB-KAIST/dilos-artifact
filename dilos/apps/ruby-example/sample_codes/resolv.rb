require 'resolv'

puts Resolv.getaddress("www.ruby-lang.org")
puts Resolv.getname("210.251.121.214").to_s
puts Resolv::DNS.new.getresources("www.ruby-lang.org", Resolv::DNS::Resource::IN::A).collect {|r| r.address}
puts Resolv::DNS.new.getresources("ruby-lang.org", Resolv::DNS::Resource::IN::MX).collect {|r| [r.exchange.to_s, r.preference]}
