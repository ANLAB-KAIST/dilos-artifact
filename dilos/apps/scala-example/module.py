#
# Copyright (C) 2014 Cloudius Systems, Ltd.
#
# This work is open source software, licensed under the terms of the
# BSD license as described in the LICENSE file in the top-level directory.
#

from osv.modules import api

api.require('scala')

classpath = ['/usr/share/scala/lib/scala-library.jar', 'HelloWorld.jar']

default = api.run_java(classpath = classpath,
                       args = [
                           "HelloWorld",
                       ],
                   )
