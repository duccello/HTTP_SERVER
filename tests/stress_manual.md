# Stress Manual Tests (Concurrency & Multi-Server)

These tests must be executed manually because they require multiple terminals and simultaneous execution of `siege`.

They validate multi-listener behavior, concurrency isolation, and global server stability.

**Note:** every test with `siege` needs a limit with `-r` or `-t` to avoid infinit loops and get the final `siege` report.

## Test 1 — Same server, two ports simultaneously

### Goal
Verify that two ports belonging to the same server instance handle load independently.

### Execution
Open **two terminals**:

_Terminal 1:_
```bash
siege -b -t30S http://localhost:8080/
```

_Terminal 2:_

```
siege -b -t30S http://localhost:8081/
```

_Expected result:_
- Both ports respond correctly
- No cross-interference between ports
- No crashes or timeouts

## Test 2 — Different servers, simultaneous load

### Goal

Verify isolation between different server blocks.

### Execution

Open two terminals:

_Terminal 1:_
```
siege -b -t30S http://localhost:9090/
```
_Terminal 2:_

```
siege -b -t30S http://localhost:8081/
```
_Expected result:_

- Both servers respond independently
- No shared-state issues
- Stable performance under concurrent load


### Test 3 — All ports simultaneously

#### Goal

Verify full system stability under distributed load across all configured servers.

#### Execution

Open multiple terminals (one per port):

```
siege -b -t30S http://localhost:8080/
```
```
siege -b -t30S http://localhost:8081/
```
```
siege -b -t30S http://localhost:9080/
```
```
siege -b -t30S http://localhost:9090/
```
```
siege -b -t30S http://localhost:9081/
```
```
siege -b -t30S http://localhost:7080/
```
```
siege -b -t30S http://localhost:9889/
```

_Expected result:_
- All servers remain responsive
- No port starvation
- No crashes or blocked event loop


### Test 4 — Full system high concurrency stress

#### Goal

Validate maximum concurrency handling across all servers.

#### Execution

Open multiple terminals, one per port:

```
siege -b -c100 -t30S http://localhost:8080/
```
```
siege -b -c100 -t30S http://localhost:8081/
```
```
siege -b -c100 -t30S http://localhost:9080/
```
```
siege -b -c100 -t30S http://localhost:9090/
```
```
siege -b -c100 -t30S http://localhost:9081/
```
```
siege -b -c100 -t30S http://localhost:7080/
```
```
siege -b -c100 -t30S http://localhost:9889/
```

_Expected result_
- Availability ≥ 99.5%
- No connection resets
- No request drops under concurrent load
- No server starvation or epoll blocking issues

---

### Failures usually indicate:
- blocking I/O
- incorrect socket management
- shared global state bugs
- incorrect file descriptor handling