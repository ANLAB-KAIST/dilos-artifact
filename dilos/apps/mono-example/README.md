This demonstrates running Mono C# apps on OSv.
Mono is a Cross platform, open source .NET framework that allows
running C# apps on Linux. For more details please
read here - https://www.mono-project.com/

To build this app you need to install Mono development framework (mono-devel)
for your Linux distribution. For details please read
https://www.mono-project.com/download/stable/#download-link.

By default the mono apps is run using tiny mono-exec app. Alternatively you
can use Makefile.FromHost (change Makefile symlink) to build image executing original mono
executable and run it like so:

```
./scripts/run.py -e '--env=MONO_DISABLE_SHARED_AREA=true /mono-sgen hello.exe'
```

Please note that in both cases you need to pass `MONO_DISABLE_SHARED_AREA=true`
variable to disable usage of Posix shared memory objects accessed via shm_open()
that is not supported on OSv at this time.
