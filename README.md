# WAF extension on Envoy proxy

This repository is forked from [`envoyproxy/envoy-wasm`](https://github.com/envoyproxy/envoy-wasm), and the example WASM extension in the envoy-wasm repository is modified to work as a Web Application Firewall(WAF) that can detect SQL injection. The rules for detection are aligned with ModSecurity rules [942100](https://github.com/coreruleset/coreruleset/blob/v3.3/dev/rules/REQUEST-942-APPLICATION-ATTACK-SQLI.conf#L45) and [942101](https://github.com/coreruleset/coreruleset/blob/v3.3/dev/rules/REQUEST-942-APPLICATION-ATTACK-SQLI.conf#L1458), and SQL injection is detected with methods from [libinjection](https://github.com/client9/libinjection).

## Build

We will build the WASM module with `proxy-wasm-cpp-sdk` on docker.
We first need to build the image of the SDK from its repository:
```
git clone git@github.com:proxy-wasm/proxy-wasm-cpp-sdk.git
cd proxy-wasm-cpp-sdk
docker build -t wasmsdk:v2 -f Dockerfile-sdk .
```
Then from the root of this repository, build the WASM module with:
```
docker run -v $PWD:/work -w /work wasmsdk:v2 /build_wasm.sh
```
After the compilation completes, you should find a `WAF_wasm.wasm` file in the repository.

## Deploy
We can mount the WASM module onto the docker image of Istio proxy to run it.
Pull the following image of Istio proxy:
```
docker pull istio/proxyv2:1.7.0-beta.2
```
Then run the proxy with WASM configured:
```
docker run \
-v ~/WAF-wasm/envoy.yaml:/etc/envoy.yaml \
-v ~/WAF-wasm/WAF_wasm.wasm:/etc/WAF_wasm.wasm \
-p 8000:8000 \
--entrypoint /usr/local/bin/envoy \
istio/proxyv2:1.7.0-beta.2 -l trace --concurrency 1 -c /etc/envoy.yaml
```

In a separate terminal, curl at `localhost:8000` to interact with the running proxy. For example, if you type the following command, you will receive a response with HTTP code 200 Okay, indicating that the request has passed SQL injection detection.
```
curl -d "hello world" -v localhost:8000
```
If you instead put something suspicious in the body, for example, enter the following command:
```
curl -d "val=-1%27+and+1%3D1%0D%0A" -v localhost:8000
```
You will receive a response with HTTP code 403 Forbidden. The body of the http request above has the parameter `val` with the value `-1' and 1=1` in URL
encoding.

## Unit Tests

Unit tests for individual utility functions in the WAF WASM extension are
available in `test` directory. To run them, execute from the root of the
repository:
```
source ./build_test.sh
```


## Configuration
The rules for SQL injection detection can be configured from YAML files. An example of configuration can be found in `envoy-config.yaml`. Configuration are passsed through the field `config.config.configuration.value` in the yaml file in JSON syntax as below:

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

There are three parts that can be configured for now: query parameters(`query_param`), cookies(`cookie`, not shown above), and headers(`header`). Configuration for all three parts are optional. If nothing is passed in a field, a default configuration based on ModSecurity rule 942100 will apply. ModSecurity rule 942101 requires SQL injection detection on path of request. Configuration for path will be updated later.

### Query Parameters
The "Content-Type" field is required in query parameters configuration, Currently, the WASM module only supports SQL injection detection for the content type "application/x-www-form-urlencoded" (it has the syntax `param=value&param2=value2`). If the incoming http request has a different content type, detection on its body will be skipped.

In default setting, all query parameter namesand values will be checked for SQL injection. To change this setting, you can either add an `include` or an `exclude` field. Both take a list of parameter names. If `include` is present, only the parameters in the list will be checked. If `exclude` is present, all but the parameters in the list will be checked. `include` and `exclude` are not expected to be present at the same time.

### Headers
In default setting, the `Referrer` and `User-Agent` headers will be checked for SQL injection. The `include` and `exclude` fields work similarly as above, except that `Referrer` and `User-Agent` will always be checked unless explicitly enlisted in `exlude`.

### Cookies
In default setting, all cookie names will be checked. `include` and `exclude` work exactly the same as for query parameters.

