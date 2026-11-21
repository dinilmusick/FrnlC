#include <frnl_semantic.h>
#include <frnl_scope.h>
#include <frnl_event_dispatch.h>
#include <frnl_ui_action_gate.h>
#include <stdio.h>
#include <string.h>
#define REQUIRE(x) do { if (!(x)) { fprintf(stderr,"failure %s:%d: %s\n", __FILE__,__LINE__,#x); return 1; } } while (0)
static frnl_status on_action(void *context,const void *value) {
  (void)value; (*(int*)context)++; return FRNL_OK;
}
static int dispatch_action(const void *payload,void *context) {
  (void)payload; (*(int*)context)++; return 0;
}
int main(void) {
  frnl_runtime runtime;
  frnl_component components[2]; frnl_state_spec states[1];
  frnl_event_def mappings[1]; frnl_function functions[1];
  frnl_storage s={0}; int calls=0;
  const frnl_scope_entry scopes[]={{"page",NULL},{"panel","page"}};
  const char *resolved[2]; size_t length=0;
  frnl_event_dispatcher legacy={0};
  frnl_event_binding legacy_events[1]; frnl_handler_binding legacy_handlers[1];
  s.components=components;s.component_capacity=2;s.states=states;s.state_capacity=1;
  s.events=mappings;s.event_capacity=1;s.functions=functions;s.function_capacity=1;s.max_depth=1;
  REQUIRE(frnl_init(&runtime,&s)==FRNL_OK);
  REQUIRE(frnl_define_state(&runtime,(frnl_state_spec){"button","idle",0,0})==FRNL_OK);
  REQUIRE(frnl_add_component(&runtime,"page",NULL,"button","idle")==FRNL_OK);
  REQUIRE(frnl_add_component(&runtime,"panel","page","button","idle")==FRNL_OK);
  REQUIRE(frnl_event_define(&runtime,(frnl_event_def){"panel","idle","press","run"})==FRNL_OK);
  REQUIRE(frnl_function_add(&runtime,(frnl_function){"panel","run",on_action,&calls})==FRNL_OK);
  REQUIRE(frnl_emit(&runtime,"panel","press",NULL)==FRNL_OK && calls==1);
  REQUIRE(frnl_scope_resolve(scopes,2,"panel",resolved,2,&length)==FRNL_SCOPE_OK && length==2);
  REQUIRE(strcmp(resolved[0],"page")==0 && strcmp(resolved[1],"panel")==0);
  legacy.events=legacy_events; legacy.event_capacity=1;
  legacy.handlers=legacy_handlers; legacy.handler_capacity=1;
  REQUIRE(frnl_event_register(&legacy,(frnl_event_binding){"panel","idle","press","run"})==FRNL_EVENT_OK);
  REQUIRE(frnl_handler_register(&legacy,(frnl_handler_binding){"panel","run",dispatch_action,&calls})==FRNL_EVENT_OK);
  REQUIRE(frnl_event_invoke(&legacy,"panel","idle","press",NULL)==FRNL_EVENT_OK && calls==2);
  REQUIRE(frnl_ui_action_authorize(FRNL_UI_MISSING,1,1,1)==FRNL_UI_DENIED);
  frnl_cleanup(&runtime);
  puts("FrnlC installed external consumer PASS"); return 0;
}
