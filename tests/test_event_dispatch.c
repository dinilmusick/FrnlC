#include "frnl_event_dispatch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"CHECK failed: %s at %s:%d\n",#x,__FILE__,__LINE__); exit(1); } } while(0)
static int calls, notifications, fail, replacement_calls, late_notifications;
static frnl_event_dispatcher *active;
static int handler(const void *p,void *ctx) { (void)ctx; calls++; return fail || !p; }
static void listener(const char *c,const char *e,const void *p,void *ctx) { (void)c;(void)e;(void)p;(void)ctx;notifications++; }
static void late_listener(const char *c,const char *e,const void *p,void *ctx) { (void)c;(void)e;(void)p;(void)ctx;late_notifications++; }
static int replacement(const void *p,void *ctx) { (void)p;(void)ctx;replacement_calls++;return 0; }
static int mutating_handler(const void *p,void *ctx) {
  (void)p;(void)ctx;
  active->handlers[0].callback=replacement;
  active->listener=late_listener;
  return 0;
}

/* Nested dispatch must not replace the outer dispatch listener snapshot. */
static int nested_calls, outer_notifications, nested_notifications;
static int nested_handler(const void *p, void *ctx) {
  frnl_event_dispatcher *d=(frnl_event_dispatcher *)ctx;
  nested_calls++;
  return frnl_event_invoke(d,"inner","ready","fire",p)!=FRNL_EVENT_OK;
}
static int inner_handler(const void *p, void *ctx) { (void)p;(void)ctx;return 0; }
static void nested_listener(const char *c,const char *e,const void *p,void *ctx) {
  (void)e;(void)p;(void)ctx;
  if (strcmp(c,"inner")==0) nested_notifications++;
  else if (strcmp(c,"outer")==0) outer_notifications++;
}
static void test_nested_dispatch(void) {
  frnl_event_binding events[2]={{"outer","ready","fire","run"},{"inner","ready","fire","run"}};
  frnl_handler_binding handlers[2];
  frnl_event_dispatcher d={events,2,2,handlers,2,2,nested_listener,0};
  int payload=1;
  handlers[0]=(frnl_handler_binding){"outer","run",nested_handler,&d};
  handlers[1]=(frnl_handler_binding){"inner","run",inner_handler,0};
  CHECK(frnl_event_invoke(&d,"outer","ready","fire",&payload)==FRNL_EVENT_OK);
  CHECK(nested_calls==1 && outer_notifications==1 && nested_notifications==1);
}

int main(void) {
  test_nested_dispatch();
  frnl_event_binding events[2];
  frnl_handler_binding handlers[2];
  frnl_event_dispatcher d={events,0,2,handlers,0,2,listener,0};
  int payload=1;
  frnl_event_binding b={"button","idle","click","onClick"};
  frnl_handler_binding h={"button","onClick",handler,0};
  CHECK(frnl_event_register(&d,b)==FRNL_EVENT_OK);
  CHECK(frnl_event_register(&d,b)==FRNL_EVENT_DUPLICATE);
  CHECK(frnl_event_invoke(&d,"missing","idle","click",&payload)==FRNL_EVENT_UNKNOWN_COMPONENT);
  CHECK(frnl_event_invoke(&d,"button","active","click",&payload)==FRNL_EVENT_UNKNOWN_STATE);
  CHECK(frnl_event_invoke(&d,"button","idle","hover",&payload)==FRNL_EVENT_UNKNOWN_EVENT);
  CHECK(frnl_event_invoke(&d,"button","idle","click",&payload)==FRNL_EVENT_MISSING_HANDLER);
  CHECK(frnl_handler_register(&d,h)==FRNL_EVENT_OK);
  CHECK(frnl_handler_register(&d,h)==FRNL_EVENT_DUPLICATE);
  CHECK(frnl_event_invoke(&d,"button","idle","click",&payload)==FRNL_EVENT_OK);
  CHECK(calls==1 && notifications==1);
  fail=1;
  CHECK(frnl_event_invoke(&d,"button","idle","click",&payload)==FRNL_EVENT_HANDLER_FAILED);
  CHECK(calls==2 && notifications==1);
  CHECK(frnl_event_invoke(&d,0,"idle","click",&payload)==FRNL_EVENT_INVALID_ARGUMENT);
  fail=0;
  active=&d;
  d.handlers[0].callback=mutating_handler;
  CHECK(frnl_event_invoke(&d,"button","idle","click",&payload)==FRNL_EVENT_OK);
  CHECK(notifications==2 && late_notifications==0);
  CHECK(frnl_event_invoke(&d,"button","idle","click",&payload)==FRNL_EVENT_OK);
  CHECK(replacement_calls==1 && late_notifications==1);
  d.event_count=3;
  CHECK(frnl_event_invoke(&d,"button","idle","click",&payload)==FRNL_EVENT_INVALID_ARGUMENT);
  return 0;
}
