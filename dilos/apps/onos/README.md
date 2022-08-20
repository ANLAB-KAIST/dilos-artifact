# Open Network Operating System (ONOS) OSv

Open Network Operating System (ONOS) provides the control plane for a software-defined network (SDN), managing network components such as switches and links, and running a variety of applications which provide communication services to end hosts and neighboring networks.

Refer to http://onosproject.org/ for more information.

## Using assets directory
If not using as part of an appliance that provides cloud-init, you can inject configuration and/or webapps by placing them in one of the following directories during build time.

| Host Directory | OSv Directory | Description |
| -------------- | ------------- | ----------- |
| ./assets/etc  | /onos/apache-karaf-${KARAF_VERSION}/etc | ONOS configuration files. |

