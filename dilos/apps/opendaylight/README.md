# OpenDaylight OSv

[OpenDaylight](http://www.opendaylight.org/) is an open platform for network programmability to enable SDN and create a solid foundation for NFV for networks at any size and scale.

## Using assets directory
If not using as part of an appliance that provides cloud-init, you can inject configuration and/or webapps by placing them in one of the following directories during build time.

| Host Directory | OSv Directory | Description |
| -------------- | ------------- | ----------- |
| ./assets/etc  | /opendaylight/etc| OpenDaylight configuration files. |
