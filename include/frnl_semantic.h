#ifndef FRNL_SEMANTIC_H
#define FRNL_SEMANTIC_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* C11, single-threaded, no heap or operating-system dependencies. All strings,
 * payloads and callback contexts are borrowed; caller owns their lifetimes. */
typedef enum {
  FRNL_OK=0, FRNL_INVALID, FRNL_CAPACITY, FRNL_DUPLICATE, FRNL_NOT_FOUND,
  FRNL_BAD_TRANSITION, FRNL_HANDLER_ERROR, FRNL_LISTENER_ERROR,
  FRNL_RENDER_ERROR, FRNL_UNSUPPORTED, FRNL_BUSY
} frnl_status;
typedef enum {
  FRNL_CREATED, FRNL_INITIALIZING, FRNL_MOUNTED, FRNL_ACTIVE,
  FRNL_SUSPENDED, FRNL_RESUMING, FRNL_DISPOSING, FRNL_DISPOSED, FRNL_FAILED
} frnl_lifecycle;
typedef enum { FRNL_ATTACK, FRNL_SUSTAIN, FRNL_RELEASE } frnl_phase;
typedef enum { FRNL_VISUAL_BASE, FRNL_VISUAL_THEME, FRNL_VISUAL_OVERRIDE } frnl_visual_layer;
typedef enum { FRNL_DATA_COMPONENT, FRNL_DATA_PAGE } frnl_data_scope;
typedef enum { FRNL_RENDER_LIFECYCLE, FRNL_RENDER_PHASE, FRNL_RENDER_STATE, FRNL_RENDER_VISUAL } frnl_render_kind;
typedef struct { const void *data; size_t size; } frnl_value;
typedef struct {
  const char *id, *parent_id, *type, *state, *pending_state, *theme;
  frnl_lifecycle lifecycle;
  frnl_phase phase;
} frnl_component;
typedef struct { const char *type, *name; unsigned has_attack, has_release; } frnl_state_spec;
/* Themes are explicit identities; a theme may have no visual overrides. */
typedef struct { const char *name; } frnl_theme;
typedef struct { const char *type, *from, *to; } frnl_state_edge;
typedef struct {
  frnl_visual_layer layer;
  const char *type, *state, *theme, *component_id, *key;
  frnl_value value;
} frnl_visual;
typedef struct { frnl_data_scope scope; const char *owner, *key; frnl_value value; } frnl_data;
typedef struct { const char *component_id, *state, *event, *function_id; } frnl_event_def;
typedef frnl_status (*frnl_action)(void *context, const void *input);
typedef struct { const char *component_id, *function_id; frnl_action action; void *context; } frnl_function;
typedef frnl_status (*frnl_semantic_listener)(void *context, const char *component_id, const char *event, const void *payload);
typedef struct { uint64_t token; const char *component_id, *event; frnl_semantic_listener listener; void *context; } frnl_subscription;
typedef struct { const char *flow_id; unsigned order; frnl_action action; void *context; } frnl_flow_step;
typedef struct { frnl_status status; const char *component_id, *operation; } frnl_diagnostic;
typedef frnl_status (*frnl_renderer)(void *context, frnl_render_kind kind, const frnl_component *component);
typedef frnl_status (*frnl_input_resolver)(void *context, const char *contract, const char *raw, frnl_value *output);
typedef struct {
  frnl_component *components; size_t component_capacity;
  frnl_state_spec *states; size_t state_capacity;
  frnl_state_edge *edges; size_t edge_capacity;
  frnl_theme *themes; size_t theme_capacity;
  frnl_visual *visuals; size_t visual_capacity;
  frnl_data *data; size_t data_capacity;
  frnl_event_def *events; size_t event_capacity;
  frnl_function *functions; size_t function_capacity;
  frnl_subscription *subscriptions; size_t subscription_capacity;
  frnl_flow_step *flows; size_t flow_capacity;
  /* Separate caller-owned snapshots: at least capacity * max_depth entries.
   These buffers must not alias registry storage. Do not call cleanup/init from a callback. */
  frnl_subscription *subscription_scratch;
  frnl_flow_step *flow_scratch;
  size_t max_depth;
} frnl_storage;
typedef struct {
  frnl_storage storage;
  size_t components, states, edges, themes, visuals, data_count, events, functions, subscriptions, flows;
  size_t dispatch_depth, flow_depth;
  uint64_t next_token;
  frnl_diagnostic last;
  frnl_renderer renderer; void *renderer_context;
  frnl_input_resolver resolver; void *resolver_context;
} frnl_runtime;
frnl_status frnl_init(frnl_runtime *rt, const frnl_storage *storage);
void frnl_cleanup(frnl_runtime *rt);
frnl_status frnl_define_state(frnl_runtime *rt, frnl_state_spec state);
frnl_status frnl_allow_state(frnl_runtime *rt, frnl_state_edge edge);
frnl_status frnl_add_component(frnl_runtime *rt, const char *id, const char *parent_id, const char *type, const char *initial_state);
frnl_status frnl_component_path(frnl_runtime *rt, const char *id, const char **path, size_t capacity, size_t *length);
frnl_status frnl_check_path(frnl_runtime *rt, const char *const *path, size_t length);
frnl_status frnl_lifecycle_transition(frnl_runtime *rt, const char *id, frnl_lifecycle to);
frnl_status frnl_set_state(frnl_runtime *rt, const char *id, const char *state);
frnl_status frnl_advance_phase(frnl_runtime *rt, const char *id);
frnl_status frnl_theme_define(frnl_runtime *rt, const char *name);
frnl_status frnl_set_theme(frnl_runtime *rt, const char *id, const char *theme);
frnl_status frnl_visual_put(frnl_runtime *rt, frnl_visual visual);
frnl_status frnl_visual_resolve(frnl_runtime *rt, const char *id, const char *key, frnl_value *out);
frnl_status frnl_data_put(frnl_runtime *rt, frnl_data entry);
frnl_status frnl_data_get(frnl_runtime *rt, frnl_data_scope scope, const char *owner, const char *key, frnl_value *out);
void frnl_bind_renderer(frnl_runtime *rt, frnl_renderer callback, void *context);
frnl_status frnl_event_define(frnl_runtime *rt, frnl_event_def event);
frnl_status frnl_function_add(frnl_runtime *rt, frnl_function function);
frnl_status frnl_subscribe(frnl_runtime *rt, const char *id, const char *event, frnl_semantic_listener callback, void *context, uint64_t *token);
frnl_status frnl_unsubscribe(frnl_runtime *rt, uint64_t token);
frnl_status frnl_emit(frnl_runtime *rt, const char *id, const char *event, const void *payload);
frnl_status frnl_flow_add(frnl_runtime *rt, frnl_flow_step step);
frnl_status frnl_flow_run(frnl_runtime *rt, const char *flow_id, const void *input);
void frnl_bind_input(frnl_runtime *rt, frnl_input_resolver callback, void *context);
frnl_status frnl_resolve_input(frnl_runtime *rt, const char *contract, const char *raw, frnl_value *out);
#ifdef __cplusplus
}
#endif
#endif
