/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 *
 * This is a simple implementation of an rpm client for OSv, which relies
 * heavily upon the librpm, which is LGPL licensed. Aside from its simplicity,
 * before engaging in any rpm operation, it sets the logfile to an in-memory
 * file, so OSv can read the output.
 *
 * In the standard rpm tool, this is usually achieved through the -pipe switch,
 * which unfortunately, forks. The default option for when this is executed, is
 * to initialize the rpm subsystem, by installing the base rpm. Other functions
 * can be achieved by directly calling into the exported functions.
 *
 * TODO: if we used librpm's microoperations instead of high level functions like
 * "rpmcliQuery", we could probably have something a lot better integrated to OSv,
 * with more meaningful error codes and less dependency on string passing.
 */

#include <rpm/rpmlib.h>
#include <rpm/rpmcli.h>
#include <rpm/rpmlog.h>
#include <rpm/rpmps.h>
#include <rpm/rpmts.h>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <osv/version.hh>
#include <osv/printf.hh>
#include <system_error>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

static struct poptOption optionsTable[] = {

 { NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmQVSourcePoptTable, 0,
        "Query/Verify package selection options:",
        NULL },
 { NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmQueryPoptTable, 0,
	"Query options (with -q or --query):",
	NULL },
 { NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmInstallPoptTable, 0,
	"Install/Upgrade/Erase options:",
	NULL },
 { NULL, '\0', POPT_ARG_INCLUDE_TABLE, rpmcliAllPoptTable, 0,
	"Common options for all rpm modes and executables:",
	NULL },
   POPT_TABLEEND
};

// This buffer size is sufficient to (more than )handle the output of rpm
// -qa in a complete Fedora desktop. It should be enough for everybody (tm)
static size_t constexpr bufsize = 128 << 10;

class rpm_ts {
public:
    explicit rpm_ts(const std::vector<std::string>& args);
    ~rpm_ts();
    rpmts get() { return _ts; }
    const char **poptargs() { return poptGetArgs(_optCon); }
    std::string output() { return std::string(_buf); }
private:
    rpmts _ts;
    poptContext _optCon = nullptr;
    std::vector<char *> _argv;
    FILE *_fp;
    char *_buf;
};

rpm_ts::rpm_ts(const std::vector<std::string>& args)
{
    _ts = rpmtsCreate();
    rpmtsSetRootDir(_ts, "/");

    _buf = static_cast<char *>(malloc(bufsize));
    _fp = fmemopen(_buf, bufsize, "w+");
    assert(_fp); // will only fail if we passed the wrong parameters, of if malloc failed.

    rpmlogSetFile(_fp);
    setbuf(_fp, NULL);

    if (args.size()) {
        for (auto &s: args) {
            _argv.push_back(strdup(s.c_str()));
        }
        _optCon = rpmcliInit(_argv.size(), _argv.data(), optionsTable);
    }
}

rpm_ts::~rpm_ts()
{
    rpmtsFree(_ts);
    if (_optCon) {
        rpmcliFini(_optCon);
    }
    for (auto *s: _argv) {
        free(s);
    }
    fclose(_fp);
    free(_buf);
}

std::vector<std::string> osv_rpm_query(std::vector<std::string> &sw)
{
    std::vector<std::string> args;
    args.push_back("rpm.so");
    args.push_back("-qa");
    QVA_t qva = &rpmQVKArgs;

    rpm_ts ts(args);

    qva->qva_mode = 'q';

    int ret = rpmcliQuery(ts.get(), qva, (ARGV_const_t) ts.poptargs());
    free(qva->qva_queryFormat);

    auto str = ts.output();
    if (ret) {
        throw std::system_error(std::error_code(), str);
    }

    using namespace boost::algorithm;
    split(sw, str, is_any_of("\n"));
    if (sw.back() == "") {
        sw.pop_back();
    }

    return sw;
}

int osv_rpm_install(std::string name)
{
    std::vector<std::string> args;
    args.push_back("rpm.so");
    args.push_back("-i");
    args.push_back("--noscripts");
    args.push_back("--excludedocs");
    args.push_back("--notriggers");
    args.push_back("--ignoreos");
    args.push_back(name);
    struct rpmInstallArguments_s * ia = &rpmIArgs;

    rpm_ts ts(args);

    return rpmInstall(ts.get(), ia, (ARGV_t) ts.poptargs());
}

int main(int argc, char **argv)
{
    auto version = osv::version();
    size_t found = 0;

    while ((found = version.find("-", found)) != std::string::npos) {
        version[found] = '.';
    }

    auto rpath = osv::sprintf("/rpms/osv-%s-1.noarch.rpm", version);
    osv_rpm_install(rpath);
}
