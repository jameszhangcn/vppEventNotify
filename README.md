# How to add a event in VPP plugin
Note: this verify base on the VPP version 20.05

In Vpp, thers are 3 types of message exchanges:



	* Request/Reply
	The client sends a request message and the server replies with a
	single reply message. The convention is that the reply message is
	named as method_name + \_reply.
	
	* Dump/Detail
	The client sends a "bulk" request message to the server, and the
	server replies with a set of detail messages. These messages may be of
	different type. A dump/detail call must be enclosed in a control ping
	block (Otherwise the client will not know the end of the bulk
	transmission). The method name must end with method + "\_dump", the
	reply message should be named method + "\_details". The exception here
	is for the methods that return multiple message types
	(e.g. sw_interface_dump). The Dump/Detail methods are typically used
	for acquiring bulk information, like the complete FIB table.
	
	* Events
	The client can register for getting asynchronous notifications from
	the server. This is useful for getting interface state changes, and so
	on. The method name for requesting notifications is conventionally
	prefixed with "want_". E.g. "want_interface_events". Which
	notification types results from an event registration is defined in
	the service definition.


	### Services
	The service statement defines the relationship between messages.
	For request/response and dump/details messages it ties the
	request with the reply. For events, it specifies which events
	that can be received for a given want_* call.
	
	Example:
	```
	service {
	  rpc want_interface_events returns want_interface_events_reply
	    events sw_interface_event;
	};
	
	```
	

![image](./resources/eventworkflow.png)

For example, we will add a GTP-U error indication event, this event is reported by VPP, and the APP client will do some post handling.

![image](https://github.com/jameszhangcn/vppEventNotify/blob/main/resources/eventworkflow.png)


Here is the steps:

## 1 Add want_xxx in the .api file

## 2 registration

related code change:


```
marco
hash
```

## 3 User govpp as the client API

