/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef HANDLERS_HH_
#define HANDLERS_HH_

#include "request.hh"
#include "common.hh"
#include "reply.hh"

#include <unordered_map>
#include <functional>
#include "exception.hh"
#include <json/json_elements.hh>

namespace httpserver {

typedef const http::server::request& const_req;

/**
 * handlers holds the logic for serving an incoming request.
 * All handlers inherit from the base httpserver_handler and
 * implement the handle method.
 *
 * A few general purpose handler are included here.
 * directory_handler - map url path to disk path.
 *  This is useful for html server
 * file_handler - map a url path to a file
 * function_handler - uses a lambda expression to handle a reqeust
 */
#pragma GCC visibility push(default)
class handler_base {
public:
    /**
     * All handlers should implement this method.
     *  It fill the reply according to the request.
     * @param path the url path used in this call
     * @param params optional parameter object
     * @param req the original request
     * @param rep the reply
     */
    virtual void handle(const std::string& path, parameters* params,
                        const http::server::request& req, http::server::reply& rep) = 0;

    virtual ~handler_base() = default;

    /**
     * set headers must be called before returning the result.
     * @param rep the reply to set
     * @param mime is the mime content type of the message content
     */
    static void set_headers_explicit(http::server::reply& rep, const std::string& mime);

    /**
     * set headers must be called before returning the result.
     * @param rep the reply to set
     * @param type is the type of the message content and is equivalent to the
     *        file extension that would have been used if it was a file
     *        e.g. html, json, js
     */
    static void set_headers(http::server::reply& rep, const std::string& type);

    /**
     * call set_headers with "html" as content type
     */
    static void set_headers(http::server::reply& rep);

    /**
     * set the reply with a not found message.
     * by default a 404 error code will be used and the default message.
     * both can be override.
     * @param reply the reply message to update.
     * @param err_code the error code to use (default 404)
     * @param alternative_message alternative error message to use
     */
    virtual void reply400(http::server::reply& rep,
                          const std::string& alternative_message = ERROR_404_PAGE);

    /**
     * set the reply with a not found message.
     * by default a 500 error code will be used and the default message.
     * both can be override.
     * @param reply the reply message to update.
     * @param err_code the error code to use (default 500)
     * @param alternative_message alternative error message to use
     */
    virtual void reply500(http::server::reply& rep,
                          const std::string& alternative_message = ERROR_500_PAGE);

        /**
     * Add a mandatory parameter
     * @param param a parameter name
     * @return a reference to the handler
     */
    handler_base& mandatory(const std::string& param)
    {
        mandatory_param.push_back(param);
        return *this;
    }
    static const std::string ERROR_500_PAGE;

    static const std::string ERROR_404_PAGE;

    std::vector<std::string> mandatory_param;

};
#pragma GCC visibility pop

/**
 * This is a base class for file transformer.
 *
 * File transformer adds the ability to modify a file content before returning
 * the results.
 *
 * The transformer decides according to the file extension if transforming is
 * needed.
 */
class file_transformer {
public:
    /**
     * Any file transformer should implement this method.
     * @param content the content to transform
     * @param req the request
     * @param extension the file extension originating the content
     */
    virtual void transform(std::string& content,
                           const http::server::request& req,
                           const std::string& extension) = 0;

    virtual ~file_transformer() = default;
};

#pragma GCC visibility push(default)
/**
 * A base class for handlers that interact with files.
 * directory and file handlers both share some common logic
 * with regards to file handling.
 * they both needs to read a file from the disk, optionally transform it,
 * and return the result or page not found on error
 */
class file_interaction_handler : public handler_base {
public:
    file_interaction_handler(file_transformer* p = nullptr)
        : transformer(p)
    {

    }

    ~file_interaction_handler();

    /**
     * Allows setting a transformer to be used with the files returned.
     * @param t the file transformer to use
     * @return this
     */
    file_interaction_handler* set_transformer(file_transformer* t)
    {
        transformer = t;
        return this;
    }

    /**
     * if the url ends without a slash redirect
     * @param req the request
     * @param rep the reply
     * @return true on redirect
     */
    bool redirect_if_needed(const http::server::request& req,
                            http::server::reply& rep);

    /**
     * A helper method that returns the file extension.
     * @param file the file to check
     * @return the file extension
     */
    static std::string get_extension(const std::string& file);

protected:

    /**
     * read a file from the disk and return it in the replay.
     * @param file the full path to a file on the disk
     * @param req the reuest
     * @param rep the reply
     */
    void read(const std::string& file, const http::server::request& req,
              http::server::reply& rep);
    file_transformer* transformer;
};
#pragma GCC visibility pop

/**
 * The directory handler get a disk path in the
 * constructor.
 * and expect a path parameter in the handle method.
 * it would concatenate the two and return the file
 * e.g. if the path is /usr/mgmt/public in the path
 * parameter is index.html
 * handle will return the content of /usr/mgmt/public/index.html
 */
class directory_handler : public file_interaction_handler {
public:

    /**
     * The directory handler map a base path and a path parameter to a file
     * @param doc_root the root directory to search the file from.
     * For example if the root is '/usr/mgmt/public' and the path parameter
     * will be '/css/style.css' the file wil be /usr/mgmt/public/css/style.css'
     */
    explicit directory_handler(const std::string& doc_root,
                               file_transformer* transformer = nullptr);

    void handle(const std::string& path, parameters* parts,
                const http::server::request& req, http::server::reply& rep)
    override;

private:
    std::string doc_root;
};

/**
 * The file handler get a path to a file on the disk
 * in the constructor.
 * it will always return the content of the file.
 */
class file_handler : public file_interaction_handler {
public:

    /**
     * The file handler map a file to a url
     * @param file the full path to the file on the disk
     */
    explicit file_handler(const std::string& file,
                          file_transformer* transformer = nullptr,
                          bool force_path = true)
        : file_interaction_handler(transformer),
          file(file), force_path(force_path)
    {
    }

    void handle(const std::string& path, parameters* parts,
                const http::server::request& req, http::server::reply& rep)
    override;

private:
    std::string file;
    bool force_path;
};

/**
 * A request function is a lambda expression that gets only the request
 * as its parameter
 */
typedef std::function<std::string(const_req req)> request_function;

/**
 * A handle function is a lambda expression that gets request and reply
 */
typedef std::function<
std::string(const_req req,
            http::server::reply&)> handle_function;

/**
 * A json request function is a lambda expression that gets only the request
 * as its parameter and return a json response.
 * Using the json response is done implicitly.
 */
typedef std::function<json::json_return_type(const_req req)> json_request_function;

/**
 * The function handler get a lambda expression in the constructor.
 * it will call that expression to get the result
 * This is suited for very simple handlers
 *
 */
class function_handler : public handler_base {
public:

    function_handler(const handle_function & f_handle,
                     const std::string& type)
        : f_handle(f_handle)
        , type(type)
    {
    }

    function_handler(const request_function & _handle,
                     const std::string& type)
        : f_handle([_handle](const_req req, http::server::reply& rep) {
        return _handle(req);
    }),
    type(type)
    {
    }

    function_handler(const json_request_function& _handle) :
        f_handle([_handle](const_req req, http::server::reply& rep) {
            json::json_return_type res = _handle(req);
            return res.res;
            }),
            type("json")
    {

    }
    void handle(const std::string& path, parameters* parts,
                const http::server::request& req, http::server::reply& rep)
    override;

    handle_function f_handle;
    std::string type;
};

}

#endif /* HANDLERS_HH_ */
