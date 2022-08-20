require 'twitter'

#set your twitter api key
client = Twitter::REST::Client.new do |config|
  config.consumer_key = ''
  config.consumer_secret = ''
  config.access_token = ''
  config.access_token_secret = ''
end

client.update( "post form CRuby on OSv" )
