# FrnlC — portable C11 semantic frontend runtime

FrnlC implements the renderer-neutral, synchronous, deterministic portion of [FrnlTS](https://github.com/dinilmusick/FrnlTS), with caller-owned fixed-capacity buffers and no operating-system dependencies. It is not React, Floks or a display driver.

## Libraries

* `FrnlC::frnl_semantic` (`frnl_semantic.h`): component/state definitions, nested hierarchical scopes, validated state/lifecycle transitions, explicit Attack/Sustain/Release, component/page data, visual themes, synchronous event handler/function registration, listener snapshots, ordered flows, renderer and semantic-input callbacks.
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
