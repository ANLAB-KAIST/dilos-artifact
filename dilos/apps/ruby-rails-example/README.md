# Ruby on Rails on OSv

This is a porting Ruby on Rails.

## Building

You need to install some packages before installed.

**Fedora**

```
yum install sqlite-devel
```

**Arch Linux**
```
pacman -S sqlite
```

And you also need suggest image when you build OSv

```
make image=ruby-rails-example
```

## Running OSv

This module has irb

You can launch this command:
```
./scripts/run.py -n
```

And you access this URL to try sample application:
```
http://<OSv's IP address>:3000/items/
```
