#ifndef FRNL_EVENT_DISPATCH_H
#define FRNL_EVENT_DISPATCH_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
  FRNL_EVENT_OK = 0, FRNL_EVENT_INVALID_ARGUMENT,
  FRNL_EVENT_DUPLICATE, FRNL_EVENT_UNKNOWN_COMPONENT,
  FRNL_EVENT_UNKNOWN_STATE, FRNL_EVENT_UNKNOWN_EVENT,
  FRNL_EVENT_MISSING_HANDLER, FRNL_EVENT_HANDLER_FAILED,
  FRNL_EVENT_CAPACITY
} frnl_event_status;
typedef int (*frnl_event_handler)(const void *payload, void *context);
typedef void (*frnl_event_listener)(const char *component, const char *event, const void *payload, void *context);
typedef struct {
  const char *component;
  const char *state;
  const char *event;
  const char *function_id;
} frnl_event_binding;
typedef struct {
  const char *component;
  const char *function_id;
  frnl_event_handler callback;
  void *context;
} frnl_handler_binding;
typedef struct {
  frnl_event_binding *events;
  size_t event_count, event_capacity;
  frnl_handler_binding *handlers;
  size_t handler_count, handler_capacity;
  frnl_event_listener listener;
  void *listener_context;
} frnl_event_dispatcher;
frnl_event_status frnl_event_register(frnl_event_dispatcher *dispatcher, frnl_event_binding binding);
frnl_event_status frnl_handler_register(frnl_event_dispatcher *dispatcher, frnl_handler_binding binding);
frnl_event_status frnl_event_invoke(frnl_event_dispatcher *dispatcher, const char *component, const char *state, const char *event, const void *payload);
/* Registry lifecycle operations: remove exact bindings without callbacks.
   All remaining entries are validated before mutation. */
frnl_event_status frnl_event_unregister(frnl_event_dispatcher *dispatcher, const char *component, const char *state, const char *event);
frnl_event_status frnl_handler_unregister(frnl_event_dispatcher *dispatcher, const char *component, const char *function_id);
#ifdef __cplusplus
}
#endif
#endif
