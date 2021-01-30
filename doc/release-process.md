# Wasm Extensions Release Process

The release of Wasm extensions shipped from this repo should follow Istio releases. When Istio makes a minor release, this repo should also cut a new release with the following steps. Assume we are release extensions for 1.x:

1. Create a new release branch (release-1.x): `git checkout -b release-1.x upstream/master && git push upstream release-1.x`.
2. At the new release branch, run `update-dep.sh` to update various dep's SHA to match Istio release. For example `./scripts/update-dep.sh -r 1.x` for `release-1.x` branch. Create a PR to commit the dep update.
3. After dep is updated, which also means the extension has passed integration test with the 1.x Istio proxy, create and push a release tag. For example, `git tag -a 1.x.0 -m "istio ecosystem wasm extensions release 1.x.0"` and `git push upstream 1.x.0`. After the tag is pushed, a [release workflow](https://github.com/istio-ecosystem/wasm-extensions/actions?query=workflow%3ARelease) should be triggered, which create a github release and upload Wasm binaries. If any more patch releases are needed, repeat step 2 and 3.
4. With new release published, various user guide also needs to be updated. Specifically, at `master` and `release-1.x` branch, run `update-guide.sh -r 1.x` to update guide and example extension to be based on 1.x. We always want guide and example to be based on the latest release.

TODO: Automate step 1, 2, and 4 (maybe 3 as well) with prow bot.