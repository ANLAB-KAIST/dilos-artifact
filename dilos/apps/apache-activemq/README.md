# Apache ActiveMQ OSv

Apache ActiveMQ is a high performance Apache 2.0 licensed
Message Broker and JMS 1.1 implementation.

Refer to http://activemq.apache.org/ for more information.

## Using assets directory
If not using as part of an appliance that provides cloud-init, you can inject configuration and/or webapps by placing them in one of the following directories during build time.

| Host Directory | OSv Directory | Description |
| -------------- | ------------- | ----------- |
| ./assets/conf  | /activemq/conf| ActiveMQ configuration files. |
| ./assets/webapps | /activemq/webapps | ActiveMQ web-applications to deploy. |
