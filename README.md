*This project has been created as part of the 42 curriculum by albetanc, cbuzzini, duccello.*

## Description
**Webserv** is a custom HTTP/1.1 server written in **C++ 98**. The core of the project is a non-blocking I/O multiplexing engine that allows handling multiple concurrent connections without the need for multi-threading.

The goal is to replicate the basic behavior of NGINX, providing a resilient server capable of parsing complex configuration files, serving static content, and executing dynamic scripts.

---

### Key Features
*   **Event-Driven:** Uses a single `epoll` instance to monitor all I/O operations (read, write, send, recv) simultaneously.
*   **Non-blocking:** Every socket and pipe is set to non-blocking mode to ensure the server never hangs.
*   **Methods:** Supports HTTP methods `GET`, `POST`, and `DELETE`.
*   **Autoindex:** Dynamically generates HTML directory listings, when a directory is requested and no index file is present.
*   **Resilience:** Designed to stay operational under stress and properly manage client disconnections.
- **Config Parsing:** NGINX-style config with multiserver, multiport, locations, mime types, timeout, redirections, autoindex, CGI, and body size limits, among others.
- **Routing:** Prefix-match location routing with method validation and static file serving
- **Logging:** error logs per server with timestamp for debugging.
- **CGI:** Supports any type of CGI scripts, which are executed in a separate process and monitored with epoll.

### Prerequisites & Dependencies
To build and run this project, you need:
*   **OS:** Linux (for `epoll`).
*   **Compiler:** `c++` (supporting C++98).
*   **Build Tool:** `GNU Make`.
*   **Standard Headers only:** No external libraries are authorized except the standard C++ library and specific system functions.

---

## 🚀 Instructions

### Compilation
The project is compiled using `make`. It uses the following flags to ensure code quality and standard compliance:
`-Wall -Wextra -Werror -std=c++98 -MMD -MP -Wshadow`

```bash
make
```
### Execution
Launch the server by providing a configuration file:

```bash
./webserv [config.conf]
```
### Other make rules
Make and execute the program (defaults to config.conf):

```bash
make run
```
Make and execute the program while checking for all types of leaks (traces childrens and fds as well):

```bash
make leaks
```
or refrain from getting a detailed leak report when using external scripts:

```bash
make script
```
### Testing
You can verify the server using a web browser `http://localhost:8080` or via the command line:

```bash
curl -v http://localhost:8080

curl http://localhost:8080/index.html  # 200 OK

curl http://localhost:8080/doesnotexist.html # 404 Not Found

curl -X POST http://localhost:8080/          # 405 Method Not Allowed

curl http://localhost:8080/randompath        # 404 Not Found

```

#### Stress testing
Here you can find more details in how perform the [stress suite](./tests/README.md)

---
## ⚙️ Technical Architecture

To ensure a modular and clean codebase, the project is split into three core modules:

1. **Config**
Parses the Nginx-style config file and generates the objects that will guide the entire functioning of the server.

2. **Networking & Event Loop**
Manages the lifecycle of sockets and the epoll multiplexing loop, including CGI.

3. **HTTP Parsing**
Responsible for transforming raw byte buffers into structured Request objects, and later translate the output into valid HTTP responses.

👉 **Config Parsing with detailed specifications, directive hierarchies, and fallback rules can be found in [src/ConfigResponse/README.md](src/Config/README.md).**

## 🔄 Workflow and CI

Branching: main (stable) ← dev (integration) ← feature/*.

CI: Every PR to main is checked for:

1. No errors with mandatory flags.
2. No memory leaks (Valgrind).
3. No file descriptor leaks.

---

## Resources
### Protocol & Theory
*   **RFC 7230:** [HTTP/1.1 Message Syntax](https://datatracker.ietf.org/doc/html/rfc7230).
*   **Parsing Theory:** [Crafting Interpreters](https://craftinginterpreters.com/contents.html) - Used for the recursive descent configuration parser.
*   **Network Programming:** [Beej's Guide](https://beej.us/guide/bgnet/).
*   **Building a Webserv in Bash:** [YouTube Channel "You Suck at Programming"](https://www.youtube.com/watch?v=L967hYylZuc).
*   **Redirections:** [Baeldung](https://www.baeldung.com/cs/redirection-status-codes).

### Practical HTTP Testing & Analysis
To understand how real-world servers handle methods and query parameters, we analyzed the following implementations:
*   **GET Method Analysis:** Observed via [OpenSanctions Search](https://www.opensanctions.org/search/?q=) to study how parameters are encoded in the URI.
*   **POST Method Analysis:** Studied via [OpenSanctions Advanced Search](https://www.opensanctions.org/advancedsearch/) to understand complex request bodies and form data handling.

### AI Usage Disclosure
AI was used in
*   **Architecture Planning:** to brainstorm the directory structure and class relationships (e.g., the `Listener` vs `Client` class design).
*   **Documentation:** to identify what needed to be updated during the development.
*   **Theoretical questions:** to understand system behavior in specific cases for many different functions such as epoll, waitpid, etc, and to weigh different solutions for a problem (for example, in deciding which errors will result in closing the connection).
*   **Debugging:** describing specific unwanted behaviors to brainstorm possible causes in the program, with GitHub, or even with Bash in certain occasions.
