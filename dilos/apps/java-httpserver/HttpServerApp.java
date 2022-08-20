//
// Copyright (C) 2018 Waldek Kozaczuk
//
// This work is open source software, licensed under the terms of the
// BSD license as described in the LICENSE file in the top-level directory.
//
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;

import com.sun.net.httpserver.*;

public class HttpServerApp {
    public static void main(String... args) throws IOException {
        final HttpHandler handler = new HttpHandler() {
            public void handle(HttpExchange t) throws IOException {
                final InputStream is = t.getRequestBody();
                while (is.read() != -1) {} //Consume all data

                final String response = "This is the response";
                t.sendResponseHeaders(200, response.length());
                final OutputStream os = t.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        };

        final HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/", handler);
        server.setExecutor(null); // creates a default executor
        System.out.println("Listening on port 8000 ...");
        server.start();
    }
}
