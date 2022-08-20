from osv.modules import api

default = api.run("--env=MONO_DISABLE_SHARED_AREA=true /run_mono hello.exe")
