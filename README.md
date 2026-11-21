# FrnlC — portable C11 semantic frontend runtime

FrnlC implements the renderer-neutral, synchronous, deterministic portion of [FrnlTS](https://github.com/dinilmusick/FrnlTS), with caller-owned fixed-capacity buffers and no operating-system dependencies. It is not React, Floks or a display driver.

## Libraries

* `FrnlC::frnl_semantic` (`frnl_semantic.h`): component/state definitions, nested hierarchical scopes, validated state/lifecycle transitions, explicit Attack/Sustain/Release, component/page data, visual themes, synchronous event handler/function registration, listener snapshots, ordered flows, renderer and semantic-input callbacks.
* `FrnlC::frnl_gutc_adapter` (`frnl_gutc_adapter.h`): optional real deterministic GutC integration, only with `-DFRNLC_WITH_GUTC=ON` and a compatible installed `GutC::gutc`. Returns `FRNL_NOT_FOUND` for unresolved input, `FRNL_UNSUPPORTED` for regex/model-only paths; output goes to a caller-owned JSON buffer. Does not change the heap-free core.
* `FrnlC::frnl_scope` (`frnl_scope.h`): standalone hierarchical scope resolver.
* `FrnlC::frnl_event_dispatch` (`frnl_event_dispatch.h`): standalone state-aware event registry/dispatch; still supports existing integration consumers.

## Build / verify / install

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS='-Wall -Wextra -Werror -pedantic'
cmake --build build
ctest --test-dir build -C Release --output-on-failure
cmake --install build --prefix "$PWD/stage"
cmake -S examples/consumer -B consumer-build -DCMAKE_PREFIX_PATH="$PWD/stage"
cmake --build consumer-build
ctest --test-dir consumer-build --output-on-failure
```

For Linux Clang set `CC=clang` before configure. GitHub Actions runs GCC/Clang, Linux/macOS/Windows (Visual Studio), Debug/Release, sanitizers and the installed external consumer.

**Memory, ownership and diagnostics:** [docs/embedded-memory.md](docs/embedded-memory.md). **Pinned contract-by-contract coverage:** [docs/frnlts-parity.md](docs/frnlts-parity.md). **External consumer:** [examples/consumer/main.c](examples/consumer/main.c).

The public API accepts borrowed opaque values. No background agents, timers, network or graphics libraries are included. A semantic resolver without a real bound adapter explicitly fails with `FRNL_UNSUPPORTED`. GutC is an **optional external integration** rather than a silently required (or faked) model dependency.

To enable the optional pinned-GutC adapter on a host: install `GutC@0e26ac721c7a020078cfaf52e6fce259fe03301e` first, then configure FrnlC with `-DFRNLC_WITH_GUTC=ON -DCMAKE_PREFIX_PATH=<GutC install prefix>`. The CI `gutc-pinned-adapter` job fetches and tests that exact revision. The default embedded build does not depend on GutC.
