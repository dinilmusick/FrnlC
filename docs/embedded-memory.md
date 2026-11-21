# FrnlC embedded storage and ownership

The semantic engine is **single-threaded C11**, contains no calls to malloc/free, and has no system clock, file, network, OS, GUI or threading dependency. It performs no hidden allocation. A client allocates an `frnl_runtime` and the individual arrays named in `frnl_storage`. Any array may be omitted if its capacity is zero. `max_depth` must be at least one.

## Capacity formula

Let the upper bounds be C components, S states, E state transitions, T themes, V visual entries, D data entries, M event mappings, F function bindings, L subscriptions, Q flow steps and K nested dispatch/flow levels. The caller supplies:

```
sizeof(frnl_runtime)
+ C*sizeof(frnl_component)
+ S*sizeof(frnl_state_spec)
+ E*sizeof(frnl_state_edge)
+ T*sizeof(frnl_theme)
+ V*sizeof(frnl_visual)
+ D*sizeof(frnl_data)
+ M*sizeof(frnl_event_def)
+ F*sizeof(frnl_function)
+ L*sizeof(frnl_subscription)
+ Q*sizeof(frnl_flow_step)
+ (L*K)*sizeof(frnl_subscription)   // event listener snapshots
+ (Q*K)*sizeof(frnl_flow_step)      // flow snapshots
```

This is the runtime's owned storage, excluding strings and payload buffers supplied by the host. Supply `subscription_scratch` with at least `L*K` entries and `flow_scratch` with at least `Q*K` entries; **do not alias** the live arrays or one another. `frnl_init` rejects invalid pointers and multiplicative overflow; it cannot measure the actual size of caller-provided buffers. Stack allocation is appropriate for tiny capacities; use static, arena, or host-owned heap arrays for larger values.

Every identifier, string, payload and callback context is **borrowed**, never copied or freed; keep it alive as long as referenced by registered components, definitions, themes, stores or callbacks. Values stored in `frnl_value` retain their byte length but the bytes are caller-owned. `frnl_cleanup` resets counters and callbacks, not the supplied buffers and not external objects. Reinitialization is allowed after cleanup. Do not call `frnl_cleanup` or `frnl_init` from inside a callback.

When capacity is exhausted, operations return `FRNL_CAPACITY`; no allocation is attempted. Nested event dispatch and flows have a hard limit `max_depth`. After callback errors, dispatch/flow depth is unwound. The `last` diagnostic provides a status plus a borrowed component identifier and static operation string. An absent semantic input resolver returns `FRNL_UNSUPPORTED`, not synthetic intelligence or a fabricated result.

## Semantics

Dispatch resolves **the current state**, copies the matching callback before execution, takes a depth-local snapshot of listeners **before** invoking the handler, calls the handler synchronously, and only notifies listeners on success. Subscription removals and new registrations do not change the current snapshot; later dispatches see them. The default callback error policy stops listener notification at the first failure and propagates a structured code.

Theme visual precedence is `state defaults < selected theme state < component overrides`. State transitions clear component visual overrides, and optional attack/release phases advance explicitly. Render callbacks receive renderer-neutral lifecycle/state/phase/visual notifications and are never assumed to be browser/React functions. A failing renderer reports `FRNL_RENDER_ERROR`; state may already have committed (adapter must reconcile its own frame).

The original `frnl_event_dispatch` and `frnl_scope` libraries remain separately installable for narrow integrations. The new semantic engine is the recommended integrated C runtime.

## GutC dependency boundary

Pinned foundation: `dinilmusick/GutC@0e26ac721c7a020078cfaf52e6fce259fe03301e`. Its `gutc_resolve_semantic_input` uses a JSON DOM plus optional host ranking/completion callbacks. FrnlC deliberately does not copy or fabricate its model output: an integrator may wrap the verified GutC resolver in `frnl_input_resolver`, retaining ownership of the returned DOM until the consumer has used it. For embedded configurations without GutC/model resources, `frnl_resolve_input` returns `FRNL_UNSUPPORTED` until a real resolver is bound. GutC itself is unchanged. On larger hosts, `FRNLC_WITH_GUTC=ON` builds `frnl_gutc_adapter`, which links the installed GutC and calls `gutc_resolve_deterministic_semantic` with JSON Schema contracts, returning a serialized JSON resolution in the caller-provided buffer. This adapter may allocate internally through GutC's JSON DOM; it is **not** part of the heap-free core. Unresolvable text returns `FRNL_NOT_FOUND`, unsupported RegExp/model paths return `FRNL_UNSUPPORTED`, and insufficient output buffer capacity returns `FRNL_CAPACITY`.
