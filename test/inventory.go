package test

import (
	env "istio.io/proxy/test/envoye2e/env"
)

var ExtensionE2ETests *env.TestInventory

func init() {
	ExtensionE2ETests = &env.TestInventory{
		Tests: []string{
			"TestBasicAuth/CorrectCredentials",
			"TestBasicAuth/IncorrectCredentials",
			"TestBasicAuth/MissingCredentials",
			"TestBasicAuth/NoPathMatch",
			"TestBasicAuth/NoMethodMatch",
			"TestBasicAuth/NoConfigurationCredentialsProvided",
			"TestBasicAuth/Realm",
			"TestBasicAuth/HostMismatch",
			"TestBasicAuth/HostExactMatch",
			"TestBasicAuth/HostPrefixMatch",
			"TestBasicAuth/HostSuffixMatch",
			"TestLocalRateLimit",
			"TestOPA/allow",
			"TestOPA/deny",
			"TestOPA/cache_expire",
		},
	}
}
