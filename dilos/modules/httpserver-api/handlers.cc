/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include "handlers.hh"
#include "mime_types.hh"

#include <sys/stat.h>
#include <fstream>

namespace httpserver {

using namespace std;

const std::string handler_base::ERROR_500_PAGE("<h1>Something went wrong</h1>");
const std::string handler_base::ERROR_404_PAGE(
    "<h1>We didn't find the page you were looking for</h1>");

#pragma GCC visibility push(default)
void handler_base::set_headers_explicit(http::server::reply& rep, const std::string& mime)
{
    const int contentLength = rep.content.size();
    rep.headers.resize(contentLength > 0 ? 2 : 1);
    rep.headers[0].name = "Content-Length";
    rep.headers[0].value = to_string(contentLength);
    // Add Content-Type only if there is some payload returned (see https://tools.ietf.org/html/rfc7231#section-3.1.1.5)
    if (contentLength > 0) {
         rep.headers[1].name = "Content-Type";
         rep.headers[1].value = mime;
    }
}

void handler_base::set_headers(http::server::reply& rep, const string& type)
{
    set_headers_explicit(rep, http::server::mime_types::extension_to_type(type));
}

void handler_base::set_headers(http::server::reply& rep)
{
    set_headers(rep, "html");
}

void handler_base::reply400(http::server::reply& rep, const std::string& alternative_message)
{
    rep = http::server::reply::stock_reply(http::server::reply::not_found,
                                           &alternative_message);
}

void handler_base::reply500(http::server::reply& rep, const std::string& alternative_message)
{
    rep = http::server::reply::stock_reply(http::server::reply::internal_server_error,
                                           &alternative_message);
}
#pragma GCC visibility pop

directory_handler::directory_handler(const string& doc_root,
                                     file_transformer* transformer)
    : file_interaction_handler(transformer),
      doc_root(doc_root)
{
}

void directory_handler::handle(const string& path, parameters* parts,
                               const http::server::request& req, http::server::reply& rep)
{
    string full_path = doc_root + (*parts)["path"];
    struct stat buf;
    stat(full_path.c_str(), &buf);
    if (S_ISDIR(buf.st_mode)) {
        if (redirect_if_needed(req, rep)) {
            return;
        }
        full_path += "/index.html";
    }
    read(full_path, req, rep);
}

#pragma GCC visibility push(default)
file_interaction_handler::~file_interaction_handler()
{
    delete transformer;
}

string file_interaction_handler::get_extension(const string& file)
{
    size_t last_slash_pos = file.find_last_of("/");
    size_t last_dot_pos = file.find_last_of(".");
    string extension;
    if (last_dot_pos != string::npos && last_dot_pos > last_slash_pos) {
        extension = file.substr(last_dot_pos + 1);
    }
    return extension;
}

void file_interaction_handler::read(const string& file,
                                    const http::server::request& req, http::server::reply& rep)
{
    ifstream is(file, ios::in | ios::binary);
    if (!is) {
        throw not_found_exception("Page not found");
    }

    string extension = get_extension(file);

    // Fill out the reply to be sent to the client.

    char buf[512];
    while (is.read(buf, sizeof(buf)).gcount() > 0)
        rep.content.append(buf, is.gcount());
    if (transformer != nullptr) {
        transformer->transform(rep.content, req, extension);
    }
    set_headers(rep, extension);
}

bool file_interaction_handler::redirect_if_needed(
    const http::server::request& req, http::server::reply& rep)
{
    if (req.uri.length() == 0 || req.uri.back() != '/') {
        rep = http::server::reply::stock_reply(
                  http::server::reply::moved_permanently,
                  nullptr);

        rep.headers.push_back(http::server::header());
        rep.headers.back().name = "Location";
        rep.headers.back().value = req.get_url() + "/";
        return true;
    }
    return false;
}
#pragma GCC visibility pop

void file_handler::handle(const string& path, parameters* parts,
                          const http::server::request& req, http::server::reply& rep)
{
    if (force_path && redirect_if_needed(req, rep)) {
        return;
    }
    read(file, req, rep);
}

void function_handler::handle(const string& path, parameters* parts,
                              const http::server::request& req, http::server::reply& rep)
{
    rep.content.append(f_handle(req, rep));
    set_headers(rep, type);
}

}
