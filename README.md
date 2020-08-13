# WAF extension on Envoy proxy

This repository is forked from
[`envoyproxy/envoy-wasm`](https://github.com/envoyproxy/envoy-wasm), and the
example WASM
extension in the envoy-wasm repository is modified to work as a Web Application
Firewall(WAF) that
can detect SQL injection. The rules for detection are aligned with ModSecurity
rules
[942100](https://github.com/coreruleset/coreruleset/blob/v3.3/dev/rules/REQUEST-942-APPLICATION-ATTACK-SQLI.conf#L45)
and
[942101](https://github.com/coreruleset/coreruleset/blob/v3.3/dev/rules/REQUEST-942-APPLICATION-ATTACK-SQLI.conf#L1458),
and SQL injection is detected with methods from
[libinjection](https://github.com/client9/libinjection).

## Deployment

From the root of this repository, build the WASM module with:

```
bazel build //:envoy_filter_http_wasm_example.wasm
```

The WASM binary being built will be at
`bazel-bin/envoy_filter_http_wasm_example.wasm`.
We will mount the WASM module onto the docker image of Istio proxy.
```
docker pull istio/proxyv2:1.7.0-beta.2
```
Then run the image with WASM configured:
```
docker run \
-v ~/WAF-wasm/envoy-config.yaml:/etc/envoy-config.yaml \
-v ~/WAF-wasm/bazel-bin/envoy_filter_http_wasm_example.wasm:/etc/envoy_filter_http_wasm_example.wasm \
-p 8000:8000 \
--entrypoint /usr/local/bin/envoy \
istio/proxyv2:1.7.0-beta.2 -l trace --concurrency 1 -c /etc/envoy-config.yaml
```
(This does not quite work yet. It seems that the envoy proxy is running without the wasm module.)

In a separate terminal, curl at `localhost:8000` to interact with the running proxy. For example, if you type the following command, you will receive a response with HTTP code 200 Okay, indicating that the request has passed SQL injection detection.
``` curl -d "hello world" -v localhost:8000```
If you instead put something suspicious in the body, for example, enter the
following command:
``` curl -d "val=-1%27+and+1%3D1%0D%0A" -v localhost:8000```
You will receive a response with HTTP code 403 Forbidden. The body of the http
request above has the parameter `val` with the value `-1' and 1=1` in URL
encoding.

## Configuration
The rules for SQL injection detection can be configured from YAML files. An
example of configuration can be found in `examples/wasm/envoy-config.yaml`.
Configuration are passsed through the field `config.config.configuration.value`
in the yaml file in JSON syntax as below:

```
{
  “query_param”: {
# detect sqli on all parameters but “foo”
    “Content-Type”: “application/x-www-form-urlencoded”,
      “exclude”: [“foo”]
  },
    “header”: {
# detect sqli on “bar”, “Referrer”, and “User-Agent”
      “include”: [“bar”]
    }
}
```
There are three parts that can be configured for now: query parameters(`query_param`), cookies(`cookie`, not shown above), and
    headers(`header`). Configuration for all three
    parts are optional. If nothing is passed in a
    field, a default configuration based on
    ModSecurity rule 942100 will apply. ModSecurity
    rule 942101 requires SQL injection detection on
    path of request. Configuration for path will be
    updated later.

### Query Parameters
    The "Content-Type" field is required in query
    parameters configuration, Currently, the WASM
    module only supports SQL injection detection
    for the content type
    "application/x-www-form-urlencoded" (it has the
        syntax `param=value&param2=value2`). If the
    incoming http request has a different content
    type, detection on its body will be skipped.

    In default setting, all query parameter names
    and values will be checked for SQL injection.
    To change this setting, you can either add an
    `include` or an `exclude` field. Both take a
    list of parameter names. If `include` is
    present, only the parameters in the list will
    be checked. If `exclude` is present, all but
    the parameters in the list will be checked.
    `include` and `exclude` are not expected to be
    present at the same time.

### Headers
    In default setting, the `Referrer` and
    `User-Agent` headers will be checked for SQL
    injection. The `include` and `exclude` fields
    work similarly as above, except that `Referrer`
    and `User-Agent` will always be checked unless
    explicitly enlisted in `exlude`.

### Cookies
    In default setting, all cookie names will be
    checked. `include` and `exclude` work exactly
    the same as for query parameters.


# Envoy WebAssembly Filter (Original README in `envoyproxy/envoy-wasm`)
In this example, we show how a WebAssembly(WASM) filter can be used with the Envoy
proxy. The Envoy proxy [configuration](./envoy.yaml) includes a Webassembly filter
as documented [here](https://www.envoyproxy.io/docs/envoy/latest/).
<!--TODO(bianpengyuan): change to the url of Wasm filter once the doc is ready.-->

## Quick Start

1. `docker-compose build`
2. `docker-compose up`
3. `curl -v localhost:18000`

Curl output should include our headers:

```
# <b> curl -v localhost:8000</b>
* Rebuilt URL to: localhost:18000/
*   Trying 127.0.0.1...
* TCP_NODELAY set
* Connected to localhost (127.0.0.1) port 18000 (#0)
> GET / HTTP/1.1
> Host: localhost:18000
> User-Agent: curl/7.58.0
> Accept: */*
> 
< HTTP/1.1 200 OK
< content-length: 13
< content-type: text/plain
< location: envoy-wasm
< date: Tue, 09 Jul 2019 00:47:14 GMT
< server: envoy
< x-envoy-upstream-service-time: 0
< newheader: newheadervalue
< 
example body
* Connection #0 to host localhost left intact
```

## Build WASM Module

Now you want to make changes to the C++ filter ([envoy_filter_http_wasm_example.cc](envoy_filter_http_wasm_example.cc))
and build the WASM module ([envoy_filter_http_wasm_example.wasm](envoy_filter_http_wasm_example.wasm)).

1. Build WASM module
   ```shell
   bazel build //examples/wasm:envoy_filter_http_wasm_example.wasm
   ```

## Build the Envoy WASM Image

<!--TODO(incfly): remove this once we upstream WASM to envoyproxy main repo.-->

For Envoy WASM runtime developers, if you want to make changes, please

1. Follow [instructions](https://github.com/envoyproxy/envoy-wasm/blob/master/WASM.md).
2. Modify `docker-compose.yaml` to mount your own Envoy.
