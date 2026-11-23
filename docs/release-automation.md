# Native GitHub source publishing

The current-main `Native C11` workflow must pass before Release Please
prepares semver release PRs or tags an immutable GitHub source archive.
`version.txt` is the canonical version for the CMake project.
The regular CI owns multi-platform dependency builds and consumers.
This release automation never publishes untested main commits.

No local machine token is needed. To get safe fully automatic release
PR merges with checks, configure GitHub App authentication plus required
status checks. The default Actions token does not trigger CI for PRs
it creates. Publishing to vcpkg and Conan still requires verified
registry recipes; GitHub tags alone do not provide registry indexing.
