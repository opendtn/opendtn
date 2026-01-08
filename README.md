# opendtn

Implementation of DTN accourding to:

1. RFC 9171 Bundle Protocol Version 7 
2. RFC 9172 Bundle Protocol Security (BPSec)
3. RFC 9173 Default Security Contexts for Bundle Protocol Security (BPSec)

This implementation is in line with the given RFCs and implements a software stack usefull for DTN bundle operations. 

## Dependencies

This library is an C bases implementation of the bundle protocol and bpsec with an external dependency to **openssl**. On Linux based installations **libsystemd** will be used for logging. No other dependencies are required. 

## Prerequisites

As a first step the development requirements needs to be installed. 
These are the opendtn development requirements in addition with the 
opendtn environment itself. To setup your environment you may run

For Ubuntu use:
```
./scripts/prepare_environment.sh debian ubuntu
```

This will install the following tools:
  - libssl-dev
  - libsystemd-dev
  - git
  - gcc
  - make
  - pkg-config
  - valgrind
  - vim
  - dpkg-dev
  - clang
  - clangd
  - gperf
  - tcpdump
  - htop

## Services

Currently 3 services are implemented.

### dtn_file_node

This service will create a file sender and a file listener, which are connected via DTN in a secured manner (bpsec may be enabled based on configuration). Using a commandline tool, the sender may be enabled to send a file to the receiver node. 

For more information about this service have a look at:
```
src/service/dtn_file_node/README
```

### dtn_tunnel

This service will create a DTN Tunnel for UDP endpoints. The UDP endpoints may be used to send any type of data, which is encypsulated at one Tunnel node and send to the counterpart Tunnel node, which forwards the data to its UDP receiver.

For more information about this service have a look at:
```
src/service/dtn_tunnel/README
```

### dtn_web_node

This service is a webnode, where some DTN packages may be generated and send to a receiver node. It is usefull for flag and setting testing for DTN packages. 

For more information about this service have a look at:
```
src/service/dtn_web_node/README
```