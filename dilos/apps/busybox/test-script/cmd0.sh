echo "/*--------------------------------------*/"
echo test-echo-string
echo env-AAA=$AAA
export AAA=aaa
echo env-AAA=$AAA
echo env-PATH=$PATH
echo "/*--------------------------------------*/"
echo "TEST ls /tools"
ls -al /tools
echo "/*--------------------------------------*/"
echo "TEST /usr/lib/ls /etc"
/usr/lib/ls -al /etc
echo "/*--------------------------------------*/"
echo -n 'before-and-and ' && echo -n 'after-and-and ' && echo 'OK'
if [ "$AAA" == "aaa" ]; then echo "OK if: $AAA==aaa"; fi
[[ "$AAA" == "bbb" ]] || echo "OK [[ ]]: $AAA!=bbb"
echo "/*--------------------------------------*/"
echo "before-missing-commnand"
no_such_command
echo "OK after-missing-commnand"
echo "/*--------------------------------------*/"
echo "before-syntax-error"
if [ true ]; echo blah; fi
# execution stops here in bash too
echo "OK after-syntax-error"
echo "/*--------------------------------------*/"
