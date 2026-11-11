
## Configuration File

> Inspired by NGINX. Each `server {}` block defines a virtual server.
> Config is the **truth source** — Router interprets it, Request is input, Response is output.

Each server {} block defines a distinct virtual host isolated by its network socket configuration.

### Workflow Clues

**Config** = truth source

**Router** = interpreter of config

**Request** = input

**Response** = output

### Minimum requirements 
A valid server needs at least a listen (port) and a root(to serv files) will be set by default if is not given, the rest has a fallback.

---

## 1. Hierarchy & Directives

**Config hierarchy:**

server {}
├── listen                # REQUIRED: port(s) to listen on (multiport)
├── root                  # OPTIONAL (default: ./www)
├── client_max_body_size  # OPTIONAL (default: 1MB)
├── error_page            # OPTIONAL (Empty by default. Explicit code-to-path map)
├── error_page_dir        # OPTIONAL (Default structural fallback: ./errors)
├── error_log             # OPTIONAL (default: error.log)
├── timeout               # OPTIONAL (default: 30, for CGI)
├── mime_type             # OPTIONAL (default: common types pre-loaded, dynamic map)
└── location {}           # OPTIONAL (Evaluated via prefix matching)
	├── root              # OPTIONAL (inherits from server)
	├── methods           # OPTIONAL (default: GET)
	├── index             # OPTIONAL (default: index.html)
	├── autoindex         # OPTIONAL (default: off) alias: autoidx
	├── cgi_ext           # OPTIONAL (CGI handler by extension)
	├── cgi_env           # OPTIONAL (env vars for CGI)
	└── return            # OPTIONAL (HTTP redirect, exclusive)

### Server block (required)
| Directive | Required | Default | Description |
|---|---|---|---|
| `listen` | ✅ | - | Interface:port to listen on, e.g. `listen 127.0.0.1:8080;` or `listen 8080;` (defaults to `0.0.0.0`). Repeatable for multiport/multi-interface |
| `server_name` | ❌ | - | Hostname for this server. One name per server block |
| `root` | ❌ | `./www` | Base directory for static files |
| `client_max_body_size` | ❌ | `1m` | Max request body. Supports `m`/`k` suffixes |
| `error_page` | ❌ | built-in | Custom error page: `error_page 404 /404.html` |
| `error_page_dir` | ❌ | `./errors` | Directory with `404.html`, `500.html` etc. |
| `error_log` | ❌ | `error.log` | Error log path |
| `timeout` | ❌ | `30` | CGI process timeout in seconds |
| `mime_type` | ❌ | common types | MIME type mapping: `mime_type .html text/html` |

### Location block (optional)
| Directive | Required | Default | Description |
|---|---|---|---|
| `methods` | ❌ | `GET` | Allowed HTTP methods (GET, POST, DELETE). If a route receives an unlisted method, the Router responds with 405 Method Not Allowed. |
| `root` | ❌ | inherited | Overrides server root for this route |
| `client_max_body_size` | ❌ | inherited from server | Overrides server limit for this route. Supports `m`/`k` suffixes |
| `index` | ❌ | `index.html` | Default file for directory requests |
| `autoindex` | ❌ | `off` | Enable directory listing. Alias: `autoidx` |
| `cgi_ext` | ❌ | - | CGI handler: `cgi_ext .py` |
| `cgi_env` | ❌ | - | CGI env vars: `cgi_env PATH=/usr/bin` |
| `return` | ❌ | - | HTTP redirect: `return 301 /new`. Exclusive directive |

---

## 2. Parsing & Automatic Fallback Rules

### Syntax validation Rules

- Order within a block is **not required** — hierarchy is what matters
- Directives are **case sensitive** and **lowercase**: `listen`, `root`, `methods`
- Methods are **UPPERCASE**: `GET`, `POST`, `DELETE`
- Paths, filenames and URLs are preserved as-is (not automatica lowerrcase)
- Each directive belongs to its context: server or location
- `client_max_body_size` is accepted in both server and location blocks. Location value overrides server value for that route; if not set, inherits from server.
- `autoidx` accepted as alias for `autoindex`
- Empty lines and indentation are skipped
- Comments start with `#` and are skipped
- Default `location /` is created automatically if not defined.
- Blocks are delimited by `{` and `}` on separate tokens
- Directives end with `;`
- Directives are keep in their specific context (global, server, location)
- Multiple values on same directive are space-separated: `methods GET POST;`
- Multiport uses repeated `listen`: one per line
- Comments start with `#` — rest of line is ignored
- Blank lines and indentation are ignored
- `server_name` accepts a single name per server (multiple server blocks for multiple names).
- `server_name` syntax: letters, digits, `-` and `.` between words, not at start or end. No consecutive `..`
- `timeout` `port` and `error code` receives only digits (seconds)
- Aliases accepted: `autoidx` = `autoindex`, `server_port` = `listen`, `on` = `true`, `off` = `false`
- Suffix markers k/K (kilobytes) and m/M (megabytes) inside `client_max_body_size` accepted and are automatically parsed and translated directly into absolute integer byte sizes inside the ServerConfig.
- *Implicit Root Context:* If a location block lacks a root directive, it automatically mirrors the parent server block's root folder.
- *Implicit Catch-All Route:* If no location / context is defined inside a server configuration, the parser automatically injects a default location / map initialized with GET permissions and index.html targets.
- Location Path Syntax: All location blocks must begin with / (validated in validateLocation), acting as a prefix for the request URI.
- The routs aren't restrict, they can be relative or absolute as NGINX.
- `listen` accepts `port`, or `interface:port` (e.g. `127.0.0.1:8080`). If no interface given, defaults to `0.0.0.0`
- Interface must be a valid IPv4 address: 4 octets, each 0-255, no leading zeros, digits only
- Port range accepted: 1024–65535 (well-known ports 0-1023 rejected)

**Note (Relative vs. Absolute Paths):**  Because config paths are concatenated to search for actual files, it is recommended to define root without a trailing slash (e.g., root www;) to avoid the formation of double slashes (//) when combined with the location path.

#### Valid formatting examples
```nginx
# all equivalent:
listen 8080;
  listen    8080;		# indentation ignored
listen 8080 ;			# space before ; NOT valid - tokenizer reads "8080" then ";"... actually valid
```

#### Invalid
```nginx
listen					# missing port and ;
Listen 8080;			# uppercase directive - not accepted
methods get;			# lowercase method - not accepted
return 301 /new			# missing ;
location/path {			# missing space before {
```


### Automatic Fallback Rules

Here is shown how are defined the default directives that will apply when are not defined in the config file:

`listen` interface → default: `0.0.0.0` (all interfaces) if not specified

`client_max_body_size` → default: 1MB

`error_page` → default: static pages into error_page_dir (can be overrided)

`index` in location / → default: index.html

`error_log` → default: `error.log` in the curren directory

`autoindex` → default: off

`methods` in location → default: GET

`cgi_ext` → default: (upcoming)

`cgi_env` → default: (upcoming)

`return` (redirection) → default: (upcoming)


---

## Server Examples


### Minimum valid server
```nginx
server {
	listen 8080;
}
```

server {
	listen 127.0.0.1:8080;
}
---

### Full example

```nginx
server {

	listen 8080;
	listen 8081;

	root www;

	client_max_body_size 1m;

	error_log  logs/error.log;

	error_page_dir www/errors;
	timeout 30;

	error_page 404 www/errors/404.html;
	mime_type .html text/html;
	mime_type .css  text/css;
	mime_type .js   application/javascript;
	mime_type .png  image/png;
	mime_type .jpg  image/jpeg;

	# setting default index (static)
	location / {
		methods GET;
		index index.html;
		autoindex false;
	}

	# autoidx
	location /list {
		methods GET;
		autoindex true;
	}
	# to test delete on main server
	location /delete {
		methods DELETE;
	}
	# to test post on main server
	location /uploads {
		methods POST;
	}
}
```

---

## 3. Parser Architechture & Tokenization Workflow

### Architecture

```
Config
 ├── tokenize()
 ├── parseServerBlock()
 ├── parseLocationBlock()
 ├── validate()
 └── getServers()

```

### Out of the scope

- Complex production-grade NGINX blocks like events {}, http {}, upstream {}, include, or worker_connections are out of scope for this architecture and are entirely bypassed by the parser.

- Are not included child context (nested directives).

- wildcards on `server_name`, it's not a mandatory feature.

- Multiple index.html files under the directive `index`, it's not a mandatory feature.

- Multiple `server_name` per server, it's not a mandatory feature.

- `server_name` are not meant to work as a virtual hosts, because it's not a mandatory feature.

### Approach:
1. tokenize
2. Compare tokens
3. validate syntax
4. build structs
5. handle errors


### Tokenizer Example:
`listen 8080;`
Tokenizer:

`["listen", "8080", ";"]`

Parser: 
`currentServer.port = 8080;`

### Tokenizer Example (with interface):
`listen 127.0.0.1:8080;`
Tokenizer:
`["listen", "127.0.0.1:8080", ";"]`
Parser:
`currentServer.ports.push_back(ListenDirective{host: "127.0.0.1", port: 8080});`

### Tokenization Workflow

```
nextToken() → "server" → expect("{") → parseServer()
	→ nextToken() → "listen" → parseListen()
	→ nextToken() → "location" → parseLocation()
		→ nextToken() → "methods" → parseMethods()
		→ nextToken() → "}" → return LocationConfig
	→ nextToken() → "}" → validateServer() → return ServerConfig

```
```
	Config::Config(path)
	└── ConfigParser::parse()
		└── nextToken() == "server" → parseServer()
				└── nextToken() por cada directiva → parseServerDirective(token, s)
						├── "listen"   → parseListen(s)     → nextToken() reads "8080"
						├── "root"     → parseRoot(s)       → nextToken() reads "/var/www"
						└── "location" → parseLocation()    → nextToken() per each directive
											└── parseLocationDirective(token, l)

```

---

## 4. Error Pages & Logging System

### Error Pages
When errors occur during request processing (e.g., 404 Not Found, 405 Method Not Allowed, 413 Payload Too Large), the server prioritizes rendering static files via Response::buildError:

**Custom Target Check:** It checks if an explicit error page matching that specific status code was registered in the configuration (error_pages map).

**Directory Resolution:** If a custom file is missing but an error_page_dir path exists (defaulting to ./errors), it attempts to look up a static asset named exactly after the status code (e.g., ./errors/404.html).

**Hardcoded Fallback:** If both lookups fail or the designated error file is missing from the filesystem, the system automatically falls back to an internal hardcoded HTML template string embedded directly in the binary execution path.

### Persistent Logging
The logging subsystem acts independently of runtime volatile console outputs using the persistent.

- **Error Logs**: Every internal tracking failure, invalid configuration detection, or failed non-2xx transaction writes a trace out via `Logger::logError(statusCode, path)`. Error 2xx are not currently supported by the Router.

It logs the event with an automated structural timestamp layout directly to the defined file descriptor target (defaulting to error.log).

- **Append Enforcement**: If log files are already present on initialization, the `std::ofstream` handles data streams with append options (`std::ios_base::app`), keeping older log data intact across sequential reboots.

---

## 5. Testing Guide

### Start the server

```bash
make
./webserv config.conf
```

### Config parsing verification (add temporarily in Config.cpp)
```cpp
for (size_t i = 0; i < _servers.size(); i++) {
	std::cout << "=== Server " << i << " ===\n";
	for (size_t j = 0; j < _servers[i].ports.size(); j++)
		std::cout << "  port: " << _servers[i].ports[j].port << "\n";
	std::cout << "  root: " << _servers[i].root << "\n";
	for (size_t j = 0; j < _servers[i].locations.size(); j++)
		std::cout << "  location: " << _servers[i].locations[j].path << "\n";
}
```

### Functional tests (single server, port 8080)

| Command | Expected |error.log |
|---|---|---|---|
| `curl http://localhost:8080/index.html` | `200 OK` | ❌ |
| `curl http://localhost:8080/doesnotexist.html` | `404 Not Found` | ✅ ERROR 404 |
| `curl -X POST http://localhost:8080/` | `405 Method Not Allowed` | ✅ ERROR 405 |
| `curl http://localhost:8080/randompath` | `404 Not Found` | ✅ ERROR 404 |

### Multiserver config test (pending Owner A)

```nginx
# SERVER 1: basic GET
server {
	listen 9080;
	root ./www/site1;
	location / { methods GET; index index.html; }
}

# SERVER 2: multiport + POST
server {
	listen 9090;
	listen 9091;
	root ./www/site2;
	client_max_body_size 5m;
	location / { methods GET; index index.html; }
	location /upload { methods POST; }
}

# SERVER 3: DELETE + autoindex
server {
	listen 7080;
	root ./www/site3;
	client_max_body_size 512k;
	location / { methods GET; autoindex true; }
	location /delete { methods DELETE; }
}
```

| Test | Command | Expected |
|---|---|---|
| Server 1 GET | `curl http://localhost:9080/index.html` | `200 OK` |
| Server 2 multiport | `curl http://localhost:9091/index.html` | `200 OK` |
| Server 2 POST | `curl -X POST -d "data" http://localhost:9090/upload` | `200 OK` |
| Server 2 body limit | `dd if=/dev/zero bs=1M count=6 \| curl -X POST --data-binary @- http://localhost:9090/upload` | `413` |
| Server 3 DELETE | `curl -X DELETE http://localhost:7080/delete` | `200 OK` |
| Server 3 autoindex | `curl http://localhost:7080/` | directory listing |

All the other test were included in the CI.

### Memory check
```bash
make leaks
# Expected: definitely lost: 0, possibly lost: 0
# still reachable is normal (ofstream buffers, libstdc++ internals)
```