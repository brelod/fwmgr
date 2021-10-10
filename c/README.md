# The idea:
Implement multi-threaded client-server application in C which allows a user to append / remove firewall rules.

Currently only 2 methods are supported: append and remove which either adds an ACCEPT rule to the FORWARD chain of the
server iptables or removes that rule. The executed commands would look like:
```
iptables -A FORWARD -s 1.2.3.4 -j ACCEPT    # client.py append 1.2.3.4
iptables -D FORWARD -s 1.2.3.4 -j ACCEPT    # client.py remove 1.2.3.4
```

# Environment:
The code has only been tested on Linux gentoo 5.4.38-gentoo-x86_64 so far.


# Compile the code:
Checkout the Makefile for setting variables like:
- THREADS - number of threads to use in the threadpool.
- QUEUE_SIZE - number of jobs which the threadpool is able to handle without refusing to answer new requests.

To compile the client and the server too use:
```
user@host:~/fwmgr/c # make all
gcc -ggdb -Wall -DLOGGING -DHOST='"127.0.0.1"' -DPORT=5555   -c -o client.o client.c
gcc -ggdb -Wall -DLOGGING   -c -o netpack.o netpack.c
gcc -ggdb -Wall   -c -o logging.o logging.c
gcc -lpthread  client.o netpack.o logging.o   -o client
gcc -ggdb -Wall -DLOGGING -DHOST='"127.0.0.1"' -DPORT=5555 -DTHREADS=1 -DQUEUE_SIZE=1   -c -o server.o server.c
gcc -ggdb -Wall -DLOGGING   -c -o runner.o runner.c
gcc -ggdb -Wall -DLOGGING   -c -o connection.o connection.c
gcc -ggdb -Wall -DLOGGING   -c -o threadpool.o threadpool.c
gcc -ggdb -Wall -DLOGGING   -c -o queue.o queue.c
gcc -lpthread  server.o runner.o connection.o threadpool.o queue.o netpack.o logging.o   -o server
```





# Examples: 
## On the server side:
Keep in mind that to be able to execute the iptables commands you have to run the server side code as a user
which has the permissions to modify the firewall rules. 
The code is trying to avoid generating too much logs, so by default it only shows the server setup / teardown and the 
executed iptables commands. To be more verbose you should enable the debug mode as you can see below.

```
root@host:~/fwmgr/c # ./server -d
2021-10-10 16:18:11 | Thread-139835302561600 |    INFO | Starting server on 127.0.0.1:5555
2021-10-10 16:18:11 | Thread-139835302561600 |   DEBUG | Creating threadpool with 4 workers and job queue with size 256
2021-10-10 16:18:11 | Thread-139835302561600 |   DEBUG | Threadpool has been created
2021-10-10 16:18:11 | Thread-139835302561600 |   DEBUG | Starting threadpool
2021-10-10 16:18:11 | Thread-139835302561600 |   DEBUG | Threadpool has started
2021-10-10 16:18:11 | Thread-139835302561600 |    INFO | Server has started
2021-10-10 16:18:11 | Thread-139835302561600 |    INFO | Start listening
2021-10-10 16:18:18 | Thread-139835302561600 |   DEBUG | Starting job
2021-10-10 16:18:18 | Thread-139835302561600 |   DEBUG | Job has started
2021-10-10 16:18:18 | Thread-139835302557248 |   DEBUG | Connection from 127.0.0.1:44214
2021-10-10 16:18:18 | Thread-139835302557248 |   DEBUG | Request: 'method=invalid-method;ip=1.2.3.4'
2021-10-10 16:18:18 | Thread-139835302557248 |   DEBUG | Parsing request: 'method=invalid-method;ip=1.2.3.4'
2021-10-10 16:18:18 | Thread-139835302557248 |   DEBUG | Parsed request: method='invalid-method', ip='1.2.3.4'
2021-10-10 16:18:18 | Thread-139835302557248 |   DEBUG | Composing response code='1', reason='Invalid method: 'invalid-method''
2021-10-10 16:18:18 | Thread-139835302557248 |   DEBUG | Composed request: 'code=1;reason=Invalid method: 'invalid-method''
2021-10-10 16:18:18 | Thread-139835302557248 |   DEBUG | Response: 'code=1;reason=Invalid method: 'invalid-method''
2021-10-10 16:18:18 | Thread-139835302557248 |   DEBUG | Close connection to 127.0.0.1:44214
2021-10-10 16:18:23 | Thread-139835302561600 |   DEBUG | Starting job
2021-10-10 16:18:23 | Thread-139835302561600 |   DEBUG | Job has started
2021-10-10 16:18:23 | Thread-139835302557248 |   DEBUG | Connection from 127.0.0.1:44216
2021-10-10 16:18:23 | Thread-139835302557248 |   DEBUG | Request: 'method=append;ip=300.300.300.300'
2021-10-10 16:18:23 | Thread-139835302557248 |   DEBUG | Parsing request: 'method=append;ip=300.300.300.300'
2021-10-10 16:18:23 | Thread-139835302557248 |   DEBUG | Parsed request: method='append', ip='300.300.300.300'
2021-10-10 16:18:23 | Thread-139835302557248 |   ERROR | Invalid ip address '300.300.300.300'
2021-10-10 16:18:23 | Thread-139835302557248 |   DEBUG | Composing response code='1', reason='Invalid ip: '300.300.300.300''
2021-10-10 16:18:23 | Thread-139835302557248 |   DEBUG | Composed request: 'code=1;reason=Invalid ip: '300.300.300.300''
2021-10-10 16:18:23 | Thread-139835302557248 |   DEBUG | Response: 'code=1;reason=Invalid ip: '300.300.300.300''
2021-10-10 16:18:23 | Thread-139835302557248 |   DEBUG | Close connection to 127.0.0.1:44216
2021-10-10 16:18:27 | Thread-139835302561600 |   DEBUG | Starting job
2021-10-10 16:18:27 | Thread-139835302561600 |   DEBUG | Job has started
2021-10-10 16:18:27 | Thread-139835302557248 |   DEBUG | Connection from 127.0.0.1:44218
2021-10-10 16:18:27 | Thread-139835302557248 |   DEBUG | Request: 'method=append;ip=1.2.3.4'
2021-10-10 16:18:27 | Thread-139835302557248 |   DEBUG | Parsing request: 'method=append;ip=1.2.3.4'
2021-10-10 16:18:27 | Thread-139835302557248 |   DEBUG | Parsed request: method='append', ip='1.2.3.4'
2021-10-10 16:18:27 | Thread-139835302557248 |    INFO | Execute cmd: 'iptables -A FORWARD -s 1.2.3.4 -j ACCEPT'
2021-10-10 16:18:27 | Thread-139835302557248 |   DEBUG | Composing response code='0', reason='1.2.3.4 was successfully added'
2021-10-10 16:18:27 | Thread-139835302557248 |   DEBUG | Composed request: 'code=0;reason=1.2.3.4 was successfully added'
2021-10-10 16:18:27 | Thread-139835302557248 |   DEBUG | Response: 'code=0;reason=1.2.3.4 was successfully added'
2021-10-10 16:18:27 | Thread-139835302557248 |   DEBUG | Close connection to 127.0.0.1:44218
2021-10-10 16:18:31 | Thread-139835302561600 |   DEBUG | Starting job
2021-10-10 16:18:31 | Thread-139835302561600 |   DEBUG | Job has started
2021-10-10 16:18:31 | Thread-139835302557248 |   DEBUG | Connection from 127.0.0.1:44220
2021-10-10 16:18:31 | Thread-139835302557248 |   DEBUG | Request: 'method=remove;ip=1.2.3.4'
2021-10-10 16:18:31 | Thread-139835302557248 |   DEBUG | Parsing request: 'method=remove;ip=1.2.3.4'
2021-10-10 16:18:31 | Thread-139835302557248 |   DEBUG | Parsed request: method='remove', ip='1.2.3.4'
2021-10-10 16:18:31 | Thread-139835302557248 |    INFO | Execute cmd: 'iptables -D FORWARD -s 1.2.3.4 -j ACCEPT'
2021-10-10 16:18:31 | Thread-139835302557248 |   DEBUG | Composing response code='0', reason='1.2.3.4 was successfully removed'
2021-10-10 16:18:31 | Thread-139835302557248 |   DEBUG | Composed request: 'code=0;reason=1.2.3.4 was successfully removed'
2021-10-10 16:18:31 | Thread-139835302557248 |   DEBUG | Response: 'code=0;reason=1.2.3.4 was successfully removed'
2021-10-10 16:18:31 | Thread-139835302557248 |   DEBUG | Close connection to 127.0.0.1:44220
2021-10-10 16:18:32 | Thread-139835302561600 |   DEBUG | Starting job
2021-10-10 16:18:32 | Thread-139835302561600 |   DEBUG | Job has started
2021-10-10 16:18:32 | Thread-139835302557248 |   DEBUG | Connection from 127.0.0.1:44222
2021-10-10 16:18:32 | Thread-139835302557248 |   DEBUG | Request: 'method=remove;ip=1.2.3.4'
2021-10-10 16:18:32 | Thread-139835302557248 |   DEBUG | Parsing request: 'method=remove;ip=1.2.3.4'
2021-10-10 16:18:32 | Thread-139835302557248 |   DEBUG | Parsed request: method='remove', ip='1.2.3.4'
2021-10-10 16:18:32 | Thread-139835302557248 |    INFO | Execute cmd: 'iptables -D FORWARD -s 1.2.3.4 -j ACCEPT'
2021-10-10 16:18:32 | Thread-139835302557248 |   DEBUG | Composing response code='1', reason='iptables: Bad rule (does a matching rule exist in that chain?).'
2021-10-10 16:18:32 | Thread-139835302557248 |   DEBUG | Composed request: 'code=1;reason=iptables: Bad rule (does a matching rule exist in that chain?).'
2021-10-10 16:18:32 | Thread-139835302557248 |   DEBUG | Response: 'code=1;reason=iptables: Bad rule (does a matching rule exist in that chain?).'
2021-10-10 16:18:32 | Thread-139835302557248 |   DEBUG | Close connection to 127.0.0.1:44222
^C2021-10-10 16:18:36 | Thread-139835302561600 |    INFO | Stopping server
2021-10-10 16:18:36 | Thread-139835302561600 |   DEBUG | Stopping threadpool
2021-10-10 16:18:36 | Thread-139835302561600 |   DEBUG | Threadpool has stopped
2021-10-10 16:18:36 | Thread-139835302561600 |   DEBUG | Destroying threadpool
2021-10-10 16:18:36 | Thread-139835302561600 |   DEBUG | Threadpool has been destroyed
2021-10-10 16:18:36 | Thread-139835302561600 |    INFO | Server is stopped

```
## On the client side:
```
user@host:~/fwmgr/c$ ./client 
Usage: ./client <method> <ip>

user@host:~/fwmgr/c$ ./client invalid-method 1.2.3.4
Invalid method: 'invalid-method'

user@host:~/fwmgr/c$ ./client append 300.300.300.300
Invalid ip: '300.300.300.300'

user@host:~/fwmgr/c$ ./client append 1.2.3.4
1.2.3.4 was successfully added

user@host:~/fwmgr/c$ ./client remove 1.2.3.4
1.2.3.4 was successfully removed

user@host:~/fwmgr/c$ ./client remove 1.2.3.4
iptables: Bad rule (does a matching rule exist in that chain?).
```


# Ideas to improve:
- Make the host, port configurable from cli (server and client too)
- Make the logging verbosity / prefix configurable from the cli
- Make the threads / queue size dynamically changing based on the load and capabilities of the server
- Define error codes as a common lib for client and server
- Do not dump command output directly to the client --> rewrite the most important things on the server
and show the original message only in the server logs.
- Unittests
- Implement Popen() - like in Python which you can use to obtain 3 fd and communicate with the subprocess

## Based on review:
### General:
- Use c99 standard: (gcc -std=c99)
- Generate header dependencies with gcc (find out how) - https://makefiletutorial.com/#makefile-cookbook
- #pragma once instead of current header guard
- ggc options: -MT <target> -MMD -MP
- Use aligned variable on the stack (32/64bit)
- Use more types instead of struct....
- hex: align to byte and use capitals (eg: 0x0F)
- Learn about restrict c99
- Use const pointers (eg in netpack) if the data can not be passed via a register (eg: big struct)

### Queue:
- queue_create() -- allocate memory in 1 shot.
- simplify return of isempty(), isfull()
- use static inline functions isempty() isfull() peek()

### Threadpool:
- Pin the workers to specific CPUs

### Logging:
- log_level_names --> const
- macro indentation
- prefix string configuration -- eg: don't include thread name
- log_set() -- should be atomic (https://en.cppreference.com/w/c/atomic)

### Connections:
- con_handler() -- naming should be improved

### Runner:
- runner_process() -- naming should be improved
- cmd concatenation - strcat has re-read the whole string over and over again

### Server:
- operate() -- queue overflow -- timeout macro
