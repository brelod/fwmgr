# The idea:
I implemented a multi-threaded server application which expects a client to send JSON data to it via TCP.
To keep being responsive under high load every client connection has it's own thread which is responsible for the
command execution.

The payload of each request should look like the following dictionary:
```
{
    "method": <method to execute>
    "host": <host to set firewall rules for>
}
```
Currently only 2 methods are supported: append and remove which either adds an ACCEPT rule to the FORWARD chain of the
server iptables or removes that rule. The executed commands would look like:
```
iptables -A FORWARD -s 1.2.3.4 -j ACCEPT    # client.py append 1.2.3.4
iptables -D FORWARD -s 1.2.3.4 -j ACCEPT    # client.py remove 1.2.3.4
```

# Depencies:
No dependencies are needed to execute the scripts apart from the standard library but keep in mind that the code
was only tested with python 3.9

# Executing unittests:
```
user@host:~/fwmgr/python$ python ./test_server.py -v
test_init_addr (__main__.TestConnection) ... ok
test_init_socket (__main__.TestConnection) ... ok
test_run (__main__.TestConnection) ... ok
test_dump (__main__.TestReponse) ... ok
test_init_code (__main__.TestReponse) ... ok
test_init_msg (__main__.TestReponse) ... ok
test_repr (__main__.TestReponse) ... ok
test__run_non_zero_return_code (__main__.TestRunner) ... ok
test__run_return_value (__main__.TestRunner) ... ok
test_append_error (__main__.TestRunner) ... ok
test_append_success (__main__.TestRunner) ... ok
test_execute_with_error (__main__.TestRunner) ... ok
test_execute_without_error (__main__.TestRunner) ... ok
test_init_data (__main__.TestRunner) ... ok
test_methods (__main__.TestRunner) ... ok
test_parse_with_json_error (__main__.TestRunner) ... ok
test_parse_without_error (__main__.TestRunner) ... ok
test_parse_without_host (__main__.TestRunner) ... ok
test_parse_without_method (__main__.TestRunner) ... ok
test_remove_error_general (__main__.TestRunner) ... ok
test_remove_error_rule_not_exist (__main__.TestRunner) ... ok
test_remove_success (__main__.TestRunner) ... ok
test_repr (__main__.TestRunner) ... ok
test_validate_address_invalid (__main__.TestRunner) ... ok
test_validate_address_missing (__main__.TestRunner) ... ok
test_validate_method_invalid (__main__.TestRunner) ... ok
test_validate_method_missing (__main__.TestRunner) ... ok

----------------------------------------------------------------------
Ran 27 tests in 0.010s

OK

```

# Examples: 
## On the server side:
Keep in mind that to be able to execute the iptables commands you have to run the server side code as a user
which has the permissions to modify the firewal rules. 
The code is trying to avoid generating to much logs, so by default it only shows the server setup / teardown and the 
executed iptables commands. To be more verbose you should enable the debug mode as you can see below.

```
root@host:~/fwmgr/python # python server.py --debug
2021-10-06 20:50:34,781 | MainThread |    INFO | Starting server on localhost:5555
2021-10-06 20:50:34,781 | MainThread |    INFO | Server has been started
2021-10-06 20:50:34,781 | MainThread |    INFO | Listening for new connections
2021-10-06 20:50:43,237 |   Thread-1 |   DEBUG | Connection received from 127.0.0.1:50632
2021-10-06 20:50:43,237 |   Thread-1 |   DEBUG | Request: b'{"method": "invalid-method", "host": "1.2.3.4"}'
2021-10-06 20:50:43,238 |   Thread-1 |   DEBUG | Response: b'{"code": 1, "msg": "Method \\"invalid-method\\" does not exist"}'
2021-10-06 20:50:43,238 |   Thread-1 |   DEBUG | Connection closed to 127.0.0.1:50632
2021-10-06 20:50:49,964 |   Thread-2 |   DEBUG | Connection received from 127.0.0.1:50634
2021-10-06 20:50:49,964 |   Thread-2 |   DEBUG | Request: b'{"method": "append", "host": "invalid-host"}'
2021-10-06 20:50:49,964 |   Thread-2 |   DEBUG | Response: b'{"code": 2, "msg": "Invalid host address \\"invalid-host\\""}'
2021-10-06 20:50:49,964 |   Thread-2 |   DEBUG | Connection closed to 127.0.0.1:50634
2021-10-06 20:50:58,871 |   Thread-3 |   DEBUG | Connection received from 127.0.0.1:50636
2021-10-06 20:50:58,872 |   Thread-3 |   DEBUG | Request: b'{"method": "append", "host": "1.2.3.4"}'
2021-10-06 20:50:58,872 |   Thread-3 |    INFO | Execute command: iptables -A FORWARD -s 1.2.3.4 -j ACCEPT
2021-10-06 20:50:58,876 |   Thread-3 |   DEBUG | Response: b'{"code": 0, "msg": "Host 1.2.3.4 has been successfully appended"}'
2021-10-06 20:50:58,876 |   Thread-3 |   DEBUG | Connection closed to 127.0.0.1:50636
2021-10-06 20:51:05,501 |   Thread-4 |   DEBUG | Connection received from 127.0.0.1:50640
2021-10-06 20:51:05,501 |   Thread-4 |   DEBUG | Request: b'{"method": "remove", "host": "1.2.3.4"}'
2021-10-06 20:51:05,501 |   Thread-4 |    INFO | Execute command: iptables -D FORWARD -s 1.2.3.4 -j ACCEPT
2021-10-06 20:51:05,505 |   Thread-4 |   DEBUG | Response: b'{"code": 0, "msg": "Host 1.2.3.4 has been successfully removed"}'
2021-10-06 20:51:05,506 |   Thread-4 |   DEBUG | Connection closed to 127.0.0.1:50640
2021-10-06 20:51:10,910 |   Thread-5 |   DEBUG | Connection received from 127.0.0.1:50642
2021-10-06 20:51:10,911 |   Thread-5 |   DEBUG | Request: b'{"method": "remove", "host": "1.2.3.4"}'
2021-10-06 20:51:10,911 |   Thread-5 |    INFO | Execute command: iptables -D FORWARD -s 1.2.3.4 -j ACCEPT
2021-10-06 20:51:10,914 |   Thread-5 | WARNING | Return-code: 1; stdout: b''; stderr: b'iptables: Bad rule (does a matching rule exist in that chain?).\n'
2021-10-06 20:51:10,916 |   Thread-5 |   DEBUG | Response: b'{"code": 4, "msg": "No matching rule presents"}'
2021-10-06 20:51:10,916 |   Thread-5 |   DEBUG | Connection closed to 127.0.0.1:50642
^C2021-10-06 20:51:15,705 | MainThread |    INFO | Stop listening for new connection because of KeyboardInterrupt
2021-10-06 20:51:15,705 | MainThread |    INFO | Server has been stopped

```
## On the client side:
```
user@host:~/fwmgr/python$ python ./client.py 
./client.py <method> <host>

Methods:
    - append
    - remove

Hosts:
    - Any valid ip address


user@host:~/fwmgr/python$ python ./client.py invalid-method 1.2.3.4
Method "invalid-method" does not exist

user@host:~/fwmgr/python$ python ./client.py append invalid-host
Invalid host address "invalid-host"

user@host:~/fwmgr/python$ python ./client.py append 1.2.3.4
Host 1.2.3.4 has been successfully appended

user@host:~/fwmgr/python$ python ./client.py remove 1.2.3.4
Host 1.2.3.4 has been successfully removed

user@host:~/fwmgr/python$ python ./client.py remove 1.2.3.4
No matching rule presents
```

