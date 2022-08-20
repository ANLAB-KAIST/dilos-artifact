Couple of XWindow apps like xclock, etc to demonstrate it works on OSv.

Please make sure that XWindows manager on your host is configured to accept TCP
based communication (should see '-listen tcp' when listing Xorg process).

It also needs to be able to accept cookie based authentication (aka xauth).
Please see https://zweije.home.xs4all.nl/xauth-6.html for details.
