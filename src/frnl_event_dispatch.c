#include "frnl_event_dispatch.h"
#include <string.h>
static int valid(const char *s) { return s && *s; }
static int same(const char *a, const char *b) { return strcmp(a,b)==0; }
frnl_event_status frnl_event_register(frnl_event_dispatcher *d, frnl_event_binding b) {
  size_t i;
  if (!d || !d->events || !valid(b.component) || !valid(b.state) || !valid(b.event) || !valid(b.function_id) || d->event_count>d->event_capacity) return FRNL_EVENT_INVALID_ARGUMENT;
  for (i=0;i<d->event_count;i++) {
    const frnl_event_binding existing=d->events[i];
    if (!valid(existing.component) || !valid(existing.state) || !valid(existing.event) || !valid(existing.function_id)) return FRNL_EVENT_INVALID_ARGUMENT;
    if (same(d->events[i].component,b.component) && same(d->events[i].state,b.state) && same(d->events[i].event,b.event)) return FRNL_EVENT_DUPLICATE;
  }
  if (d->event_count>=d->event_capacity) return FRNL_EVENT_CAPACITY;
  d->events[d->event_count++]=b;
  return FRNL_EVENT_OK;
}
frnl_event_status frnl_handler_register(frnl_event_dispatcher *d, frnl_handler_binding b) {
  size_t i;
  if (!d || !d->handlers || !valid(b.component) || !valid(b.function_id) || !b.callback || d->handler_count>d->handler_capacity) return FRNL_EVENT_INVALID_ARGUMENT;
  for (i=0;i<d->handler_count;i++) {
    const frnl_handler_binding existing=d->handlers[i];
    if (!valid(existing.component) || !valid(existing.function_id) || !existing.callback) return FRNL_EVENT_INVALID_ARGUMENT;
    if (same(d->handlers[i].component,b.component) && same(d->handlers[i].function_id,b.function_id)) return FRNL_EVENT_DUPLICATE;
  }
  if (d->handler_count>=d->handler_capacity) return FRNL_EVENT_CAPACITY;
  d->handlers[d->handler_count++]=b;
  return FRNL_EVENT_OK;
}
frnl_event_status frnl_event_invoke(frnl_event_dispatcher *d, const char *component, const char *state, const char *event, const void *payload) {
  size_t i,j;
  int component_found=0, state_found=0;
  if (!d || !valid(component) || !valid(state) || !valid(event) || (d->event_count && !d->events) || (d->handler_count && !d->handlers) || d->event_count>d->event_capacity || d->handler_count>d->handler_capacity) return FRNL_EVENT_INVALID_ARGUMENT;
  /* Fail closed before invoking user callbacks: a malformed entry anywhere
     in the registry must not be hidden behind an earlier matching entry. */
  for (i=0;i<d->event_count;i++) {
    const frnl_event_binding b=d->events[i];
    size_t k;
    if (!valid(b.component) || !valid(b.state) || !valid(b.event) || !valid(b.function_id)) return FRNL_EVENT_INVALID_ARGUMENT;
    /* Caller-owned arrays can be modified outside registration. Reject
       ambiguous event mappings before invoking any application callback. */
    for (k=0;k<i;k++) {
      const frnl_event_binding prior=d->events[k];
      if (same(prior.component,b.component) && same(prior.state,b.state) && same(prior.event,b.event)) return FRNL_EVENT_DUPLICATE;
    }
  }
  for (j=0;j<d->handler_count;j++) {
    const frnl_handler_binding h=d->handlers[j];
    size_t k;
    if (!valid(h.component) || !valid(h.function_id) || !h.callback) return FRNL_EVENT_INVALID_ARGUMENT;
    for (k=0;k<j;k++) {
      const frnl_handler_binding prior=d->handlers[k];
      if (same(prior.component,h.component) && same(prior.function_id,h.function_id)) return FRNL_EVENT_DUPLICATE;
    }
  }
  for (i=0;i<d->event_count;i++) {
    const frnl_event_binding b=d->events[i];
    if (!valid(b.component) || !valid(b.state) || !valid(b.event) || !valid(b.function_id)) return FRNL_EVENT_INVALID_ARGUMENT;
    if (!same(b.component,component)) continue;
    component_found=1;
    if (!same(b.state,state)) continue;
    state_found=1;
    if (!same(b.event,event)) continue;
    for (j=0;j<d->handler_count;j++) {
      const frnl_handler_binding h=d->handlers[j];
      if (!valid(h.component) || !valid(h.function_id) || !h.callback) return FRNL_EVENT_INVALID_ARGUMENT;
      if (same(h.component,component) && same(h.function_id,b.function_id)) {
        /* A handler may synchronously change the registry. Keep the selected
           callback and listener stable for this dispatch. */
        const frnl_event_listener notify=d->listener;
        void *const notify_context=d->listener_context;
        if (h.callback(payload,h.context)!=0) return FRNL_EVENT_HANDLER_FAILED;
        if (notify) notify(component,event,payload,notify_context);
        return FRNL_EVENT_OK;
      }
    }
    return FRNL_EVENT_MISSING_HANDLER;
  }
  if (!component_found) return FRNL_EVENT_UNKNOWN_COMPONENT;
  return state_found ? FRNL_EVENT_UNKNOWN_EVENT : FRNL_EVENT_UNKNOWN_STATE;
}

/* Remove one exact event binding, retaining the order of other entries.
   No callbacks are executed during removal. */
frnl_event_status frnl_event_unregister(frnl_event_dispatcher *d,
    const char *component, const char *state, const char *event) {
  size_t i, j;
  if (!d || !valid(component) || !valid(state) || !valid(event) ||
      (d->event_count && !d->events) || d->event_count > d->event_capacity)
    return FRNL_EVENT_INVALID_ARGUMENT;
  for (i = 0; i < d->event_count; ++i) {
    const frnl_event_binding b = d->events[i];
    if (!valid(b.component) || !valid(b.state) || !valid(b.event) ||
        !valid(b.function_id)) return FRNL_EVENT_INVALID_ARGUMENT;
  }
  for (i = 0; i < d->event_count; ++i) {
    const frnl_event_binding b = d->events[i];
    if (same(b.component, component) && same(b.state, state) &&
        same(b.event, event)) {
      for (j = i + 1; j < d->event_count; ++j) d->events[j - 1] = d->events[j];
      --d->event_count;
      memset(&d->events[d->event_count], 0, sizeof(d->events[0]));
      return FRNL_EVENT_OK;
    }
  }
  return FRNL_EVENT_UNKNOWN_EVENT;
}

/* Remove one exact handler binding. Existing event mappings intentionally
   remain and will report MISSING_HANDLER until another handler is registered. */
frnl_event_status frnl_handler_unregister(frnl_event_dispatcher *d,
    const char *component, const char *function_id) {
  size_t i, j;
  if (!d || !valid(component) || !valid(function_id) ||
      (d->handler_count && !d->handlers) ||
      d->handler_count > d->handler_capacity)
    return FRNL_EVENT_INVALID_ARGUMENT;
  for (i = 0; i < d->handler_count; ++i) {
    const frnl_handler_binding h = d->handlers[i];
    if (!valid(h.component) || !valid(h.function_id) || !h.callback)
      return FRNL_EVENT_INVALID_ARGUMENT;
  }
  for (i = 0; i < d->handler_count; ++i) {
    const frnl_handler_binding h = d->handlers[i];
    if (same(h.component, component) && same(h.function_id, function_id)) {
      for (j = i + 1; j < d->handler_count; ++j)
        d->handlers[j - 1] = d->handlers[j];
      --d->handler_count;
      memset(&d->handlers[d->handler_count], 0, sizeof(d->handlers[0]));
      return FRNL_EVENT_OK;
    }
  }
  return FRNL_EVENT_MISSING_HANDLER;
}
