// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//  * This file was modified from the original implementation *
//
//

#include "request_handler.hh"
#include "mime_types.hh"
#include "request.hh"
#include "reply.hh"
#include "common.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <osv/options.hh>

namespace http {

namespace server {

size_t request_handler::update_parameters(request& req)
{
    auto update_param = [&req](const std::string & s, size_t bg, size_t end) {
        if (bg == end) {
            return;
        }
        req.query_parameters.push_back(header());
        size_t eq;
        if ((eq = s.find('=', bg)) < end) {
            req.query_parameters.back().name = s.substr(bg, eq - bg);
            request_handler::url_decode(s.substr(eq + 1, end - eq - 1),
                                        req.query_parameters.back().value);
        } else {
            req.query_parameters.back().name = s.substr(bg, end - bg);
            req.query_parameters.back().value = "";
        }
    };
    auto parse_str = [&](const std::string & s, size_t bg) {
        size_t end;
        while ((end = s.find('&', bg)) != std::string::npos) {
            update_param(s, bg, end);
            bg = end +1;
        }
        update_param(s, bg, s.length());
    };

    if (req.is_form_post()) {
        parse_str(req.content, 0);
    }

    size_t par = req.uri.find('?');
    if (par != std::string::npos) {
        parse_str(req.uri, par+1);
    }
    return par;
}

request_handler::request_handler(httpserver::routes* routes, std::map<std::string,std::vector<std::string>>& _config)
    : routes(routes),
      config(_config)

{
    if (options::option_value_exists(_config, "access-allow")) {
        const auto s = options::extract_option_value(_config, "access-allow");

        std::string::size_type b = 0;
        do {
            auto e = s.find_first_of(',', b);
            auto d = s.substr(b, e - b);
            // maintaining "true" compatibility (reluctantly).
            // just in case, accept "false" as well. but in an actual list this makes less sense.
            if (d == "false") {
                allowed_domains.clear();
                break;
            }
            if (d.empty()) {
                break;
            }
            allowed_domains.emplace_back(d == "true" ? "*" : std::move(d));
            b = e + 1;
        } while (b != 0);

        std::sort(allowed_domains.begin(), allowed_domains.end());
    }
}

void request_handler::handle_request(request& req, reply& rep)
{
    // Decode url to path.
    std::string request_path;
    size_t param = update_parameters(req);
    if (!url_decode(req.uri, request_path, param))
    {
        rep = reply::stock_reply(reply::bad_request);
        return;
    }

    // Request path must be absolute and not contain "..".
    if (request_path.empty() || request_path[0] != '/'
            || request_path.find("..") != std::string::npos)
    {

        rep = reply::stock_reply(reply::bad_request);
        return;
    }

    // Do not handle the request if this is OPTIONS as client is requesting
    // capabilities of the server (see https://tools.ietf.org/html/rfc7231#section-4.3.7)
    if(httpserver::str2type(req.method) != httpserver::OPTIONS)
        routes->handle(request_path, req, rep);

    if (!allowed_domains.empty()) {
        auto origin = req.get_header("Origin");
        for (auto & s : allowed_domains) {
            if (s == "*" || s == origin) {
                rep.add_header("Access-Control-Allow-Origin", s);
                if (!req.get_header("Access-Control-Request-Method").empty()) {
                    rep.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, OPTIONS, DELETE");
                }
                const auto h = req.get_header("Access-Control-Request-Headers");
                if (!h.empty()) {
                    rep.add_header("Access-Control-Allow-Headers", h);
                }
                // allow caching CORS data. We won't be changing anything.
                rep.add_header("Access-Control-Max-Age", "1000");
            }
        }
    }
}

bool request_handler::url_decode(const std::string& in, std::string& out,
                                 size_t max)
{
    out.clear();
    out.reserve(in.size());
    if (in.size() < max) {
        max = in.size();
    }

    for (std::size_t i = 0; i < max; ++i)
    {
        if (in[i] == '%')
        {
            if (i + 3 <= in.size())
            {
                int value = 0;
                std::istringstream is(in.substr(i + 1, 2));
                if (is >> std::hex >> value)
                {
                    out += static_cast<char>(value);
                    i += 2;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else if (in[i] == '+')
        {
            out += ' ';
        }
        else
        {
            out += in[i];
        }
    }
    return true;
}

} // namespace server

} // namespace http
