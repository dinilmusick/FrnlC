# FrnlTS pinned → FrnlC C11 parity matrix

FrnlTS authority: `dinilmusick/FrnlTS@a9339b9e92f9ef375ac164e274842644dca60d8a`. This is **semantic** parity, not a JavaScript/React ABI or promise implementation. GutC reference: `dinilmusick/GutC@0e26ac721c7a020078cfaf52e6fce259fe03301e`.

| Reference contract | FrnlC implementation | Status / qualification |
| --- | --- | --- |
| `component-definition.ts` component identifiers, type and states | `frnl_define_state`, `frnl_add_component`, `frnl_state_spec` | Supported; bounded static registries |
| `events.ts` parent-child scopes | `frnl_component_path`, `frnl_check_path`, plus separate `frnl_scope` | Supported; rooted paths, missing parent and cycle guards |
| `core.ts` lifecycle transition table | `frnl_lifecycle_transition` | Supported; 9 phases and allowed transition table |
| `store.ts` entity state transitions | `frnl_allow_state`, `frnl_set_state` | Supported; explicit validated edges (intentional stricter native policy) |
| `component-definition.ts` Attack/Sustain/Release | `frnl_phase`, `frnl_advance_phase` | Supported; explicitly advanced; no animation engine |
| `store.ts` per-component properties | `frnl_data_put/get(FRNL_DATA_COMPONENT)` | Supported; borrowed opaque values |
| `store.ts` page-shared data | `frnl_data_put/get(FRNL_DATA_PAGE)` | Supported; page IDs owned by host |
| `events.ts` state-specific event map | `frnl_event_define`, `frnl_emit` | Supported; synchronous only |
| `events.ts` per-component function registry | `frnl_function_add` | Supported; C callback signatures |
| `orchestration.ts` subscriptions and unsubscription | `frnl_subscribe`, `frnl_unsubscribe` | Supported; tokenized, bounded |
| `orchestration.ts` listener snapshots | depth-local scratch in `frnl_emit` | Supported; removal does not affect current dispatch |
| `events.ts` nested event handler failures | `frnl_emit` error propagation and depth unwinding | Supported; structured status, no JS exceptions |
| `orchestration.ts` synchronous flow setup and sequence | `frnl_flow_add`, `frnl_flow_run` | Supported; ordered fixed-capacity steps, synchronous |
| `themes.ts` registered named themes | `frnl_theme_define`, `frnl_set_theme` | Supported; empty themes allowed |
| `themes.ts` visual parameter fallback | `frnl_visual_put`, `frnl_visual_resolve` | Supported; defaults → theme → override |
| `store.ts` reset overrides on state change | `clear_override` in state commit | Supported |
| `presentation.ts` host-facing presentation metadata | `frnl_renderer` callback | Partial: host owns actual renderer and accessibility realization |
| `validation.ts` general rich schemas | argument/status validation, bounded registry checks | Partial; no Zod/JSON Schema engine |
| `semantic-input.ts` semantic field resolution | `frnl_bind_input`, `frnl_resolve_input` | Host interface; optional real pinned GutC deterministic adapter; without bound resolver reports `FRNL_UNSUPPORTED` |
| `semantic-input.ts` asynchronous model ranking/completion | optional GutC-backed or host callback adapter | Not implemented natively; requires actual host integration |
| `semantic-input.ts` form JSON Schema compilation | host-provided contract string/validator | Not implemented; no automatic form JSON schema generator |
| `orchestration.ts` Promise event listeners | synchronous C listeners only | Not applicable to deterministic embedded profile |
| `libraries.ts` arbitrary JS libraries | host callback/opaque payload boundary | Not implemented as dynamic JS module registry |
| `routing.ts` client router | host responsibility | Not implemented; unrelated to renderer-neutral UI kernel |
| `react/*`, FrnlTS hooks | separate rendering adapters | Intentionally excluded: no React clone |
| `ui-session.ts` DOM/session authority | existing `frnl_ui_action_gate.h` & executor | Partial: host must supply real platform/session/visibility state |
| `resources.ts` asynchronous resource fetching | external host adapters | Not implemented: requires OS/network |
| `measurement.ts` time/DOM measurements | renderer/platform callback | Host responsibility; no clock/GUI dependencies |

**Acceptance boundary:** A bounded, reusable native semantic runtime; not complete feature/API parity for FrnlTS web tooling, asynchronous browser APIs or model-dependent semantic inference. Do not assert support for any row marked Partial, Not implemented or Host responsibility without a real host adapter and separate tests.
