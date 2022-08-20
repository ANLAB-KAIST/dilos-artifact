# OSv GUI

This is the sourced code repository for OSv GUI.

# Running 

As the app currenly isn't served by the same server as the OSv public API, it relies on the API to allow [CORS](https://developer.mozilla.org/en-US/docs/Web/HTTP/Access_control_CORS) request for now. 
to do that, Access-Control-Allow-Origin headers need to be set.

They can be easily added to the httpserver module by modifying handler_base::set_headers() (found in httpserver/handlers.cc) and adding the Access-Control-Allow-Origin headers with the value "*" (or the address you will serve the application on) to the headers vector.

After modifying the httpserver, you need to build the app using make and serve the application using any http server with the "public" dir as the base dir.

It is important that all of the requests will be directed to the index.html file, as client side JS is doing the actual routing.

If PHP is installed, the builtin server can be easily used to serve the files as it directs all of the requests to the index file by default (php -S address:port).