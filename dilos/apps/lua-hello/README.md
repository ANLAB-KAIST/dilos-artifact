# Lua on OSv

Lua is a powerful and fast programming language that is easy to learn and use and to embed into your application.
Lua is designed to be a lightweight embeddable scripting language. It is used for all sorts of applications, from games to web applications and image processing.
You can read more at [https://www.lua.org](https://www.lua.org).

## Building
This example will build a hello world in Lua based on the following code (hello.lua):
```
-- prints Hello World
print("Hello World")
-- exit from Lua interpreter (will shutdown OSv)
os.exit()
```

From OSv root directory execute the following command:
```
./scripts/build image=lua-hello
```

## Customisation
In order to change or add a Lua script, please follow the steps below:
1. Place your scripts inside <OSv root directory>/apps/lua-hello

2. Declare your scripts inside the file usr.manifest like this
```
/usr/lib/<script.lua>: ${MODULE_DIR}/<script.lua>
```
This tells OSv build script to place <script.lua> from current module directory
(in this case <OSv root directory>/apps/lua-hello) inside OSv image at the path /usr/lib/<script.lua>

3. Replace the script to be executed when OSv starts inside module.py:
```
default = api.run("/usr/lib/liblua.so /usr/lib/<script.lua>")
```

4. Clean OSv image and rebuild it
