# Wasm Extension Integration Test Dev Guide
---

Before deploying your Wasm extensions in production, it is **highly** recommended to write integration tests which load and run the extension with Envoy binary. To make writing such integration test easier, the integration test framework from istio/proxy repo will be used, which is the same framework used to test Istio telemetry extensions.

## Overview
---

The reference document about the test framework could be found [here](https://godoc.org/github.com/istio/proxy/test/envoye2e/driver). At the high level, the test framework:
* Downloads Envoy binary, which is built by Istio proxy postsubmit and used in istio proxy docker container.
* Spawns up an xDS server locally, which serves customizable xDS resources. In your test, the XDS resource will point to your own Wasm extension.
* Spawns up Envoy processes locally with customizable bootstrap templates.
* Executes test logic which sends requests through Envoy, then examines extension logic accordingly.

To take a more detailed look, the test framework models every operation in the test as a [`Step`](https://godoc.org/github.com/istio/proxy/test/envoye2e/driver#Step). For example, one of the most important steps is [`Envoy`](https://godoc.org/github.com/istio/proxy/test/envoye2e/driver#Envoy), which downloads and spawns an Envoy process in `Run` method, and stops the process in `Cleanup` method.

Every `Run` method implementation takes a [`Params`](https://godoc.org/github.com/istio/proxy/test/envoye2e/driver#Params) struct as input, which should be created at the beginning of the test and carries test context that is shared by all `Steps`. Information provided by `Params` includes:
* Port assignment, such as port numbers assigned to the client Envoy, the server Envoy, and the xDS server.
* Variable map, which is used to fill in templates for various Envoy configuration.
* Server state of XDS server.

In the following guide, we will walk through the [integration test for the example extension](../example/test/example_test.go). which verifies that the example extension could inject header to the response header. Several snippets are provided at the end for more complicated usage.

## Example Extension Integration Test Walkthrough
---

As the first step of the test, a `Params` struct is created. Besides the Golang testing object, the other two variables are passed in:
* a string map, which will be copied to `Vars` string map in `Params`. `Vars` are used to fill in all kinds of template that are used in the test, such as Envoy bootstrap configuration, listener configuration, filter configuration, etc. We will look at how `Vars` are used later.
* a test inventory, which lists all tests that could be ran by go test. Inside the function, each test is going to have a unique set of ports assigned, which will be used later when spawn up Envoy and XDS server. Test inventory is to prevent port collision. For example, [here](https://github.com/istio/proxy/blob/32a5195862266bc49faa94bfb88d1719420abb3b/test/envoye2e/inventory.go#L21) is the test inventory of `istio/proxy`.
```go
params := driver.NewTestParams(t, map[string]string{
		"ExampleWasmFile": filepath.Join(env.GetBazelBinOrDie(), "example.wasm"),
	}, test.ExtensionE2ETests)
```

Then a HTTP filter configuration is rendered from [a Golang template file](../example/test/testdata/server_filter.yaml.tmpl). Inside the template, a WebAssembly filter is configured, which points to a local file with `{{ .Vars.ExampleWasmFile }}`. As you see, the `ExampleWamFile` variable is specified when `params` is created. The rendered Wasm HTTP filter is also stored into variables map in `params`, which will be used later when constructing Envoy listener.

```go
params.Vars["ServerHTTPFilters"] = params.LoadTestData("test/testdata/server_filter.yaml.tmpl")
```

Now all needed configurations are rendered, it is time to get to the core of the test. All test setup and logic are wrapped by a [`Scenario`](https://godoc.org/github.com/istio/proxy/test/envoye2e/driver#Scenario) object, which is also a `Step` and acts as a collection of `Step`. In its `Run` method, all steps will be ran sequentially and the same `params` struct will be passed in to all steps.

```go
if err := (&driver.Scenario{
    []driver.Step{
        &driver.XDS{},
        ...
    },
}).Run(params); err != nil {
    t.Fatal(err)
}
```

Now let's take a detailed look at the steps listed in `Scenario`. First two steps start up a XDS server, which listens on `params.Ports.XDSPort`, and prepare the first listener update (LDS). Several fields are specified in the listener update step, which include the targeting Envoy proxy node id `server`, version of this update `0`, and template for the listener update `testdata.MustAsset("listener/server.yaml.tmpl")`. [`testdata`](https://github.com/istio/proxy/blob/32a5195862266bc49faa94bfb88d1719420abb3b/testdata/testdata.gen.go#L3) is a vfs package generated with the same resource template files used by `istio/proxy` integration tests. Here in this step, the [server listener](https://github.com/istio/proxy/blob/32a5195862266bc49faa94bfb88d1719420abb3b/testdata/testdata.gen.go#L394-L425) template is used and we fill in `.Vars.ServerHTTPFilters` with the Example Wasm filter configuration rendered in the former step.

```go
&driver.Scenario{
    &driver.XDS{},
    &driver.Update{
        Node: "server", Version: "0", Listeners: []string{string(testdata.MustAsset("listener/server.yaml.tmpl"))},
    },
    ...
}
```

In the following two steps download Envoy with a given version, take a bootstrap template shipped with `testdata` package and generate bootstrap configuration from it, then start up the Envoy process with it. By default the server config embeds a static reply server, which could be disabled or replaced by your own server if desired via variables in `params`. The variables could be found in the [server sidecar bootstrap](https://github.com/istio/proxy/blob/32a5195862266bc49faa94bfb88d1719420abb3b/testdata/testdata.gen.go#L145-L238).

```go
&driver.Scenario{
    ...
    &driver.Envoy{
        Bootstrap:       params.FillTestData(string(testdata.MustAsset("bootstrap/server.yaml.tmpl"))),
        DownloadVersion: "1.11",
    },
    &driver.Sleep{Duration: 1 * time.Second},
    ...
}
```

At the final step, a HTTP call is made to the server Envoy, with wanted response header and response code. The `HTTPCall` step will verify the response indeed has the given response header `x-wasm-custom`.

```go
&driver.Scenario{
    ...
    &driver.HTTPCall{
        Port:            params.Ports.ServerPort,
        Method:          "GET",
        ResponseHeaders: map[string]string{"x-wasm-custom": "foo"},
        ResponseCode:    200,
    },
    ...
}
```

With the test passing, we will be confident that the extension will work within an Istio setup.

## Example snippets
---

There are many example integration tests that could be found [under `istio/proxy` repo](https://github.com/istio/proxy/tree/master/test/envoye2e). Here are some common step sequences that you might find useful.

**Start up client and server sidecars, and send request through both of them**:
```go
Steps: []driver.Step{
    &driver.XDS{},
    &driver.Update{Node: "client", Version: "0", Listeners: []string{params.FillTestData(string(testdata.MustAsset("listener/client.yaml.tmpl")))}},
    &driver.Update{Node: "server", Version: "0", Listeners: []string{params.FillTestData(string(testdata.MustAsset("listener/server.yaml.tmpl"))}},
    &driver.Envoy{Bootstrap: params.FillTestData(string(testdata.MustAsset("bootstrap/client.yaml.tmpl")))},
    &driver.Envoy{Bootstrap: params.FillTestData(string(testdata.MustAsset("bootstrap/server.yaml.tmpl")))},
    &driver.Sleep{Duration: 1 * time.Second},
    &driver.Repeat{
        N: 10,
        Step: &driver.HTTPCall{
            Port:         params.Ports.ClientPort,
            ResponseCode: 200,
        },
    },
}
```

**Check stats generated by Envoy**
```go
Steps: []driver.Step{
    &driver.Stats{params.Ports.ClientAdmin, map[string]driver.StatMatcher{
        "istio_requests_total": &driver.ExactStat{"testdata/metric/client_request_total.yaml.tmpl"},
    }},
    &driver.Stats{params.Ports.ServerAdmin, map[string]driver.StatMatcher{
        "istio_requests_total": &driver.ExactStat{"testdata/metric/server_request_total.yaml.tmpl"},
    }},
}
```
