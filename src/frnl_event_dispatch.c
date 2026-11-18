#include "frnl_event_dispatch.h"
#include <string.h>
static int valid(const char *s) { return s && *s; }
static int same(const char *a, const char *b) { return strcmp(a,b)==0; }
frnl_event_status frnl_event_register(frnl_event_dispatcher *d, frnl_event_binding b) {
  size_t i;
  if (!d || !d->events || !valid(b.component) || !valid(b.state) || !valid(b.event) || !valid(b.function_id)) return FRNL_EVENT_INVALID_ARGUMENT;
  for (i=0;i<d->event_count;i++)
    if (same(d->events[i].component,b.component) && same(d->events[i].state,b.state) && same(d->events[i].event,b.event)) return FRNL_EVENT_DUPLICATE;
  if (d->event_count>=d->event_capacity) return FRNL_EVENT_CAPACITY;
  d->events[d->event_count++]=b;
  return FRNL_EVENT_OK;
}
frnl_event_status frnl_handler_register(frnl_event_dispatcher *d, frnl_handler_binding b) {
  size_t i;
  if (!d || !d->handlers || !valid(b.component) || !valid(b.function_id) || !b.callback) return FRNL_EVENT_INVALID_ARGUMENT;
  for (i=0;i<d->handler_count;i++)
    if (same(d->handlers[i].component,b.component) && same(d->handlers[i].function_id,b.function_id)) return FRNL_EVENT_DUPLICATE;
  if (d->handler_count>=d->handler_capacity) return FRNL_EVENT_CAPACITY;
  d->handlers[d->handler_count++]=b;
  return FRNL_EVENT_OK;
}
frnl_event_status frnl_event_invoke(frnl_event_dispatcher *d, const char *component, const char *state, const char *event, const void *payload) {
  size_t i,j;
  int component_found=0, state_found=0;
  if (!d || !valid(component) || !valid(state) || !valid(event)) return FRNL_EVENT_INVALID_ARGUMENT;
  for (i=0;i<d->event_count;i++) {
    const frnl_event_binding *b=&d->events[i];
    if (!same(b->component,component)) continue;
    component_found=1;
    if (!same(b->state,state)) continue;
    state_found=1;
    if (!same(b->event,event)) continue;
    for (j=0;j<d->handler_count;j++) {
      const frnl_handler_binding *h=&d->handlers[j];
      if (same(h->component,component) && same(h->function_id,b->function_id)) {
        if (h->callback(payload,h->context)!=0) return FRNL_EVENT_HANDLER_FAILED;
        if (d->listener) d->listener(component,event,payload,d->listener_context);
        return FRNL_EVENT_OK;
      }
    }
    return FRNL_EVENT_MISSING_HANDLER;
  }
  if (!component_found) return FRNL_EVENT_UNKNOWN_COMPONENT;
  return state_found ? FRNL_EVENT_UNKNOWN_EVENT : FRNL_EVENT_UNKNOWN_STATE;
}
