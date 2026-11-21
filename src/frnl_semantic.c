#include "frnl_semantic.h"
#include <string.h>
#include <stdint.h>

static int valid(const char *s) { return s != NULL && s[0] != '\0'; }
static int same(const char *a, const char *b) { return a && b && strcmp(a,b)==0; }
static frnl_status report(frnl_runtime *rt, frnl_status status, const char *id, const char *op) {
  if (rt) { rt->last.status=status; rt->last.component_id=id; rt->last.operation=op; }
  return status;
}
static frnl_component *component(frnl_runtime *rt, const char *id) {
  size_t i;
  if (!rt || !valid(id)) return NULL;
  for (i=0;i<rt->components;i++) if (same(rt->storage.components[i].id,id)) return &rt->storage.components[i];
  return NULL;
}
static const frnl_state_spec *state_spec(frnl_runtime *rt, const char *type, const char *name) {
  size_t i;
  for (i=0;i<rt->states;i++) {
    const frnl_state_spec *s=&rt->storage.states[i];
    if (same(type,s->type) && same(name,s->name)) return s;
  }
  return NULL;
}
static frnl_status render(frnl_runtime *rt, frnl_render_kind kind, const frnl_component *c) {
  if (rt->renderer && rt->renderer(rt->renderer_context,kind,c)!=FRNL_OK)
    return report(rt,FRNL_RENDER_ERROR,c->id,"renderer");
  return FRNL_OK;
}
frnl_status frnl_init(frnl_runtime *rt, const frnl_storage *s) {
  if (!rt || !s || !s->max_depth ||
      (s->component_capacity && !s->components) || (s->state_capacity && !s->states) ||
      (s->edge_capacity && !s->edges) || (s->theme_capacity && !s->themes) || (s->visual_capacity && !s->visuals) ||
      (s->data_capacity && !s->data) || (s->event_capacity && !s->events) ||
      (s->function_capacity && !s->functions) ||
      (s->subscription_capacity && (!s->subscriptions || !s->subscription_scratch)) ||
      (s->flow_capacity && (!s->flows || !s->flow_scratch)) ||
      (s->subscription_capacity && s->max_depth > SIZE_MAX/s->subscription_capacity) ||
      (s->flow_capacity && s->max_depth > SIZE_MAX/s->flow_capacity)) return FRNL_INVALID;
  memset(rt,0,sizeof(*rt)); rt->storage=*s; rt->next_token=1; return FRNL_OK;
}
void frnl_cleanup(frnl_runtime *rt) { if (rt) memset(rt,0,sizeof(*rt)); }
frnl_status frnl_define_state(frnl_runtime *rt, frnl_state_spec state) {
  if (!rt || !valid(state.type) || !valid(state.name) || state.has_attack>1 || state.has_release>1)
    return report(rt,FRNL_INVALID,NULL,"define_state");
  if (state_spec(rt,state.type,state.name)) return report(rt,FRNL_DUPLICATE,NULL,"define_state");
  if (rt->states>=rt->storage.state_capacity) return report(rt,FRNL_CAPACITY,NULL,"define_state");
  rt->storage.states[rt->states++]=state; return FRNL_OK;
}
frnl_status frnl_allow_state(frnl_runtime *rt, frnl_state_edge edge) {
  size_t i;
  if (!rt || !valid(edge.type) || !valid(edge.from) || !valid(edge.to) ||
      !state_spec(rt,edge.type,edge.from) || !state_spec(rt,edge.type,edge.to))
    return report(rt,FRNL_INVALID,NULL,"allow_state");
  for (i=0;i<rt->edges;i++) {
    const frnl_state_edge *e=&rt->storage.edges[i];
    if (same(e->type,edge.type) && same(e->from,edge.from) && same(e->to,edge.to))
      return report(rt,FRNL_DUPLICATE,NULL,"allow_state");
  }
  if (rt->edges>=rt->storage.edge_capacity) return report(rt,FRNL_CAPACITY,NULL,"allow_state");
  rt->storage.edges[rt->edges++]=edge; return FRNL_OK;
}
frnl_status frnl_add_component(frnl_runtime *rt, const char *id, const char *parent, const char *type, const char *initial) {
  frnl_component c;
  if (!rt || !valid(id) || !valid(type) || !valid(initial) || (parent && !valid(parent)) || same(id,parent) ||
      !state_spec(rt,type,initial) || (parent && !component(rt,parent)))
    return report(rt,FRNL_INVALID,id,"add_component");
  if (component(rt,id)) return report(rt,FRNL_DUPLICATE,id,"add_component");
  if (rt->components>=rt->storage.component_capacity) return report(rt,FRNL_CAPACITY,id,"add_component");
  memset(&c,0,sizeof(c)); c.id=id; c.parent_id=parent; c.type=type; c.state=initial;
  c.lifecycle=FRNL_CREATED; c.phase=state_spec(rt,type,initial)->has_attack ? FRNL_ATTACK : FRNL_SUSTAIN;
  rt->storage.components[rt->components++]=c; return FRNL_OK;
}
frnl_status frnl_component_path(frnl_runtime *rt, const char *id, const char **path, size_t capacity, size_t *length) {
  frnl_component *c; size_t n=0,i;
  if (length) *length=0;
  if (!rt || !path || !length || !valid(id)) return report(rt,FRNL_INVALID,id,"component_path");
  c=component(rt,id);
  if (!c) return report(rt,FRNL_NOT_FOUND,id,"component_path");
  while (c) {
    for (i=0;i<n;i++) if (same(path[i],c->id)) return report(rt,FRNL_INVALID,id,"component_cycle");
    if (n>=capacity) return report(rt,FRNL_CAPACITY,id,"component_path");
    path[n++]=c->id;
    if (!c->parent_id) break;
    c=component(rt,c->parent_id);
    if (!c) return report(rt,FRNL_NOT_FOUND,id,"missing_parent");
  }
  for (i=0;i<n/2;i++) { const char *tmp=path[i]; path[i]=path[n-i-1]; path[n-i-1]=tmp; }
  *length=n; return FRNL_OK;
}
frnl_status frnl_check_path(frnl_runtime *rt, const char *const *path, size_t count) {
  frnl_component *c; size_t i;
  if (!rt || !path || !count) return report(rt,FRNL_INVALID,NULL,"check_path");
  for (i=0;i<count;i++) if (!valid(path[i])) return report(rt,FRNL_INVALID,NULL,"check_path");
  c=component(rt,path[count-1]);
  if (!c) return report(rt,FRNL_NOT_FOUND,path[count-1],"check_path");
  for (i=count;i>0;i--) {
    if (!c || !same(c->id,path[i-1])) return report(rt,FRNL_INVALID,path[count-1],"check_path");
    if (i==1) return c->parent_id ? report(rt,FRNL_INVALID,path[count-1],"check_path") : FRNL_OK;
    c=component(rt,c->parent_id);
  }
  return report(rt,FRNL_INVALID,NULL,"check_path");
}
static int may_transition(frnl_lifecycle from, frnl_lifecycle to) {
  static const unsigned short transitions[9]={
    (1u<<FRNL_INITIALIZING)|(1u<<FRNL_DISPOSING),
    (1u<<FRNL_MOUNTED)|(1u<<FRNL_FAILED)|(1u<<FRNL_DISPOSING),
    (1u<<FRNL_ACTIVE)|(1u<<FRNL_SUSPENDED)|(1u<<FRNL_DISPOSING)|(1u<<FRNL_FAILED),
    (1u<<FRNL_SUSPENDED)|(1u<<FRNL_DISPOSING)|(1u<<FRNL_FAILED),
    (1u<<FRNL_RESUMING)|(1u<<FRNL_DISPOSING)|(1u<<FRNL_FAILED),
    (1u<<FRNL_ACTIVE)|(1u<<FRNL_FAILED)|(1u<<FRNL_DISPOSING),
    (1u<<FRNL_DISPOSED),0,
    (1u<<FRNL_INITIALIZING)|(1u<<FRNL_DISPOSING)|(1u<<FRNL_DISPOSED)
  };
  return from<=FRNL_FAILED && to<=FRNL_FAILED && (transitions[from] & (1u<<to))!=0;
}
frnl_status frnl_lifecycle_transition(frnl_runtime *rt, const char *id, frnl_lifecycle to) {
  frnl_component *c=component(rt,id);
  if (!c) return report(rt,FRNL_NOT_FOUND,id,"lifecycle");
  if (c->lifecycle==to) return FRNL_OK;
  if (!may_transition(c->lifecycle,to)) return report(rt,FRNL_BAD_TRANSITION,id,"lifecycle");
  c->lifecycle=to; return render(rt,FRNL_RENDER_LIFECYCLE,c);
}
static void clear_override(frnl_runtime *rt, const char *id) {
  size_t i=0;
  while (i<rt->visuals) {
    if (rt->storage.visuals[i].layer==FRNL_VISUAL_OVERRIDE && same(rt->storage.visuals[i].component_id,id)) {
      size_t j; for (j=i+1;j<rt->visuals;j++) rt->storage.visuals[j-1]=rt->storage.visuals[j];
      rt->visuals--;
    } else i++;
  }
}
static frnl_status commit_state(frnl_runtime *rt, frnl_component *c, const char *state) {
  const frnl_state_spec *spec=state_spec(rt,c->type,state);
  c->state=state; c->pending_state=NULL;
  clear_override(rt,c->id);
  c->phase=spec->has_attack ? FRNL_ATTACK : FRNL_SUSTAIN;
  { frnl_status s=render(rt,FRNL_RENDER_STATE,c); if (s!=FRNL_OK) return s; }
  return render(rt,FRNL_RENDER_PHASE,c);
}
frnl_status frnl_set_state(frnl_runtime *rt, const char *id, const char *state) {
  frnl_component *c=component(rt,id); size_t i;
  if (!c) return report(rt,FRNL_NOT_FOUND,id,"set_state");
  if (!valid(state) || !state_spec(rt,c->type,state)) return report(rt,FRNL_INVALID,id,"set_state");
  if (c->pending_state) return report(rt,FRNL_BUSY,id,"set_state");
  if (same(c->state,state)) return FRNL_OK;
  for (i=0;i<rt->edges;i++) {
    const frnl_state_edge *e=&rt->storage.edges[i];
    if (same(c->type,e->type) && same(c->state,e->from) && same(state,e->to)) break;
  }
  if (i==rt->edges) return report(rt,FRNL_BAD_TRANSITION,id,"set_state");
  if (state_spec(rt,c->type,c->state)->has_release) {
    c->pending_state=state; c->phase=FRNL_RELEASE;
    return render(rt,FRNL_RENDER_PHASE,c);
  }
  return commit_state(rt,c,state);
}
frnl_status frnl_advance_phase(frnl_runtime *rt, const char *id) {
  frnl_component *c=component(rt,id);
  if (!c) return report(rt,FRNL_NOT_FOUND,id,"advance_phase");
  if (c->phase==FRNL_ATTACK) { c->phase=FRNL_SUSTAIN; return render(rt,FRNL_RENDER_PHASE,c); }
  if (c->phase==FRNL_RELEASE && c->pending_state) return commit_state(rt,c,c->pending_state);
  return report(rt,FRNL_BAD_TRANSITION,id,"advance_phase");
}
static int theme_exists(frnl_runtime *rt, const char *name) {
  size_t i;
  for (i=0;i<rt->themes;i++) if (same(rt->storage.themes[i].name,name)) return 1;
  return 0;
}
frnl_status frnl_theme_define(frnl_runtime *rt, const char *name) {
  if (!rt || !valid(name)) return report(rt,FRNL_INVALID,NULL,"theme_define");
  if (theme_exists(rt,name)) return report(rt,FRNL_DUPLICATE,NULL,"theme_define");
  if (rt->themes>=rt->storage.theme_capacity) return report(rt,FRNL_CAPACITY,NULL,"theme_define");
  rt->storage.themes[rt->themes++]=(frnl_theme){name};
  return FRNL_OK;
}
frnl_status frnl_set_theme(frnl_runtime *rt, const char *id, const char *theme) {
  frnl_component *c=component(rt,id);
  if (!c) return report(rt,FRNL_NOT_FOUND,id,"set_theme");
  if (!valid(theme)) return report(rt,FRNL_INVALID,id,"set_theme");
  if (!theme_exists(rt,theme)) return report(rt,FRNL_NOT_FOUND,id,"set_theme");
  c->theme=theme; return render(rt,FRNL_RENDER_VISUAL,c);
}
frnl_status frnl_visual_put(frnl_runtime *rt, frnl_visual v) {
  size_t i;
  if (!rt || v.layer>FRNL_VISUAL_OVERRIDE || !valid(v.key) || (!v.value.data && v.value.size) ||
      (v.layer==FRNL_VISUAL_OVERRIDE ? !component(rt,v.component_id) : !valid(v.type) || !valid(v.state) || !state_spec(rt,v.type,v.state)) ||
      (v.layer==FRNL_VISUAL_THEME && (!valid(v.theme) || !theme_exists(rt,v.theme)))) return report(rt,FRNL_INVALID,v.component_id,"visual_put");
  for (i=0;i<rt->visuals;i++) {
    frnl_visual *e=&rt->storage.visuals[i];
    if (e->layer==v.layer && same(e->key,v.key) &&
        (v.layer==FRNL_VISUAL_OVERRIDE ? same(e->component_id,v.component_id) :
         same(e->type,v.type) && same(e->state,v.state) && (v.layer==FRNL_VISUAL_BASE || same(e->theme,v.theme)))) {
      *e=v; return FRNL_OK;
    }
  }
  if (rt->visuals>=rt->storage.visual_capacity) return report(rt,FRNL_CAPACITY,v.component_id,"visual_put");
  rt->storage.visuals[rt->visuals++]=v; return FRNL_OK;
}
frnl_status frnl_visual_resolve(frnl_runtime *rt, const char *id, const char *key, frnl_value *out) {
  frnl_component *c=component(rt,id); size_t i; int found=0;
  if (!c || !valid(key) || !out) return report(rt,FRNL_INVALID,id,"visual_resolve");
  for (i=0;i<rt->visuals;i++) {
    const frnl_visual *v=&rt->storage.visuals[i];
    if (!same(v->key,key)) continue;
    if (v->layer==FRNL_VISUAL_BASE && same(v->type,c->type) && same(v->state,c->state)) { *out=v->value; found=1; }
  }
  for (i=0;i<rt->visuals;i++) {
    const frnl_visual *v=&rt->storage.visuals[i];
    if (v->layer==FRNL_VISUAL_THEME && same(v->theme,c->theme) && same(v->type,c->type) && same(v->state,c->state) && same(v->key,key)) { *out=v->value; found=1; }
  }
  for (i=0;i<rt->visuals;i++) {
    const frnl_visual *v=&rt->storage.visuals[i];
    if (v->layer==FRNL_VISUAL_OVERRIDE && same(v->component_id,id) && same(v->key,key)) { *out=v->value; found=1; }
  }
  return found ? FRNL_OK : report(rt,FRNL_NOT_FOUND,id,"visual_resolve");
}
frnl_status frnl_data_put(frnl_runtime *rt, frnl_data v) {
  size_t i;
  if (!rt || v.scope>FRNL_DATA_PAGE || !valid(v.owner) || !valid(v.key) || (!v.value.data && v.value.size) ||
      (v.scope==FRNL_DATA_COMPONENT && !component(rt,v.owner))) return report(rt,FRNL_INVALID,v.owner,"data_put");
  for (i=0;i<rt->data_count;i++) {
    frnl_data *e=&rt->storage.data[i];
    if (e->scope==v.scope && same(e->owner,v.owner) && same(e->key,v.key)) { *e=v; return FRNL_OK; }
  }
  if (rt->data_count>=rt->storage.data_capacity) return report(rt,FRNL_CAPACITY,v.owner,"data_put");
  rt->storage.data[rt->data_count++]=v; return FRNL_OK;
}
frnl_status frnl_data_get(frnl_runtime *rt, frnl_data_scope scope, const char *owner, const char *key, frnl_value *out) {
  size_t i;
  if (!rt || !out || !valid(owner) || !valid(key) || scope>FRNL_DATA_PAGE)
    return report(rt,FRNL_INVALID,owner,"data_get");
  for (i=0;i<rt->data_count;i++) {
    const frnl_data *e=&rt->storage.data[i];
    if (e->scope==scope && same(e->owner,owner) && same(e->key,key)) { *out=e->value; return FRNL_OK; }
  }
  return report(rt,FRNL_NOT_FOUND,owner,"data_get");
}
void frnl_bind_renderer(frnl_runtime *rt, frnl_renderer callback, void *context) {
  if (rt) { rt->renderer=callback; rt->renderer_context=context; }
}
frnl_status frnl_event_define(frnl_runtime *rt, frnl_event_def v) {
  size_t i;
  frnl_component *c=component(rt,v.component_id);
  if (!c || !valid(v.state) || !valid(v.event) || !valid(v.function_id) || !state_spec(rt,c->type,v.state))
    return report(rt,FRNL_INVALID,v.component_id,"event_define");
  for (i=0;i<rt->events;i++) {
    const frnl_event_def *e=&rt->storage.events[i];
    if (same(e->component_id,v.component_id) && same(e->state,v.state) && same(e->event,v.event))
      return report(rt,FRNL_DUPLICATE,v.component_id,"event_define");
  }
  if (rt->events>=rt->storage.event_capacity) return report(rt,FRNL_CAPACITY,v.component_id,"event_define");
  rt->storage.events[rt->events++]=v; return FRNL_OK;
}
frnl_status frnl_function_add(frnl_runtime *rt, frnl_function v) {
  size_t i;
  if (!rt || !component(rt,v.component_id) || !valid(v.function_id) || !v.action)
    return report(rt,FRNL_INVALID,v.component_id,"function_add");
  for (i=0;i<rt->functions;i++) {
    const frnl_function *f=&rt->storage.functions[i];
    if (same(f->component_id,v.component_id) && same(f->function_id,v.function_id))
      return report(rt,FRNL_DUPLICATE,v.component_id,"function_add");
  }
  if (rt->functions>=rt->storage.function_capacity) return report(rt,FRNL_CAPACITY,v.component_id,"function_add");
  rt->storage.functions[rt->functions++]=v; return FRNL_OK;
}
frnl_status frnl_subscribe(frnl_runtime *rt, const char *id, const char *event, frnl_event_listener cb, void *context, uint64_t *token) {
  frnl_subscription s;
  if (!rt || !component(rt,id) || !valid(event) || !cb || !token || rt->next_token==UINT64_MAX)
    return report(rt,FRNL_INVALID,id,"subscribe");
  if (rt->subscriptions>=rt->storage.subscription_capacity) return report(rt,FRNL_CAPACITY,id,"subscribe");
  s.token=rt->next_token++; s.component_id=id; s.event=event; s.listener=cb; s.context=context;
  rt->storage.subscriptions[rt->subscriptions++]=s; *token=s.token; return FRNL_OK;
}
frnl_status frnl_unsubscribe(frnl_runtime *rt, uint64_t token) {
  size_t i,j;
  if (!rt || !token) return report(rt,FRNL_INVALID,NULL,"unsubscribe");
  for (i=0;i<rt->subscriptions;i++) {
    if (rt->storage.subscriptions[i].token==token) {
      for (j=i+1;j<rt->subscriptions;j++) rt->storage.subscriptions[j-1]=rt->storage.subscriptions[j];
      rt->subscriptions--; return FRNL_OK;
    }
  }
  return report(rt,FRNL_NOT_FOUND,NULL,"unsubscribe");
}
frnl_status frnl_emit(frnl_runtime *rt, const char *id, const char *event, const void *payload) {
  frnl_component *c=component(rt,id); frnl_function handler; const frnl_event_def *mapping=NULL;
  frnl_subscription *snapshot; size_t i,n=0; frnl_status result=FRNL_OK;
  if (!c || !valid(event)) return report(rt,FRNL_INVALID,id,"emit");
  if (rt->dispatch_depth>=rt->storage.max_depth) return report(rt,FRNL_CAPACITY,id,"dispatch_depth");
  for (i=0;i<rt->events;i++) {
    const frnl_event_def *e=&rt->storage.events[i];
    if (same(e->component_id,id) && same(e->state,c->state) && same(e->event,event)) { mapping=e; break; }
  }
  if (!mapping) return report(rt,FRNL_NOT_FOUND,id,"event_mapping");
  memset(&handler,0,sizeof(handler));
  for (i=0;i<rt->functions;i++) {
    const frnl_function *f=&rt->storage.functions[i];
    if (same(f->component_id,id) && same(f->function_id,mapping->function_id)) { handler=*f; break; }
  }
  if (!handler.action) return report(rt,FRNL_NOT_FOUND,id,"event_handler");
  snapshot=rt->storage.subscription_capacity ?
    rt->storage.subscription_scratch+rt->dispatch_depth*rt->storage.subscription_capacity : NULL;
  for (i=0;i<rt->subscriptions;i++) {
    frnl_subscription s=rt->storage.subscriptions[i];
    if (same(s.component_id,id) && same(s.event,event)) snapshot[n++]=s;
  }
  rt->dispatch_depth++;
  if (handler.action(handler.context,payload)!=FRNL_OK) result=report(rt,FRNL_HANDLER_ERROR,id,"event_handler");
  if (result==FRNL_OK) {
    for (i=0;i<n;i++) if (snapshot[i].listener(snapshot[i].context,id,event,payload)!=FRNL_OK) {
      result=report(rt,FRNL_LISTENER_ERROR,id,"event_listener"); break;
    }
  }
  rt->dispatch_depth--;
  return result;
}
frnl_status frnl_flow_add(frnl_runtime *rt, frnl_flow_step step) {
  if (!rt || !valid(step.flow_id) || !step.action) return report(rt,FRNL_INVALID,NULL,"flow_add");
  if (rt->flows>=rt->storage.flow_capacity) return report(rt,FRNL_CAPACITY,NULL,"flow_add");
  rt->storage.flows[rt->flows++]=step; return FRNL_OK;
}
frnl_status frnl_flow_run(frnl_runtime *rt, const char *flow_id, const void *input) {
  frnl_flow_step *steps; size_t i,j,n=0;
  if (!rt || !valid(flow_id)) return report(rt,FRNL_INVALID,NULL,"flow_run");
  if (rt->flow_depth>=rt->storage.max_depth) return report(rt,FRNL_CAPACITY,NULL,"flow_depth");
  steps=rt->storage.flow_capacity ?
    rt->storage.flow_scratch+rt->flow_depth*rt->storage.flow_capacity : NULL;
  for (i=0;i<rt->flows;i++) if (same(rt->storage.flows[i].flow_id,flow_id)) steps[n++]=rt->storage.flows[i];
  if (!n) return report(rt,FRNL_NOT_FOUND,NULL,"flow_run");
  for (i=1;i<n;i++) {
    frnl_flow_step step=steps[i]; j=i;
    while (j>0 && steps[j-1].order>step.order) { steps[j]=steps[j-1]; j--; }
    steps[j]=step;
  }
  rt->flow_depth++;
  for (i=0;i<n;i++) if (steps[i].action(steps[i].context,input)!=FRNL_OK) {
    rt->flow_depth--; return report(rt,FRNL_HANDLER_ERROR,NULL,"flow_step");
  }
  rt->flow_depth--; return FRNL_OK;
}
void frnl_bind_input(frnl_runtime *rt, frnl_input_resolver callback, void *context) {
  if (rt) { rt->resolver=callback; rt->resolver_context=context; }
}
frnl_status frnl_resolve_input(frnl_runtime *rt, const char *contract, const char *raw, frnl_value *out) {
  if (!rt || !valid(contract) || !raw || !out) return report(rt,FRNL_INVALID,NULL,"resolve_input");
  if (!rt->resolver) return report(rt,FRNL_UNSUPPORTED,NULL,"resolve_input");
  { frnl_status s=rt->resolver(rt->resolver_context,contract,raw,out);
    return s==FRNL_OK ? s : report(rt,s,NULL,"resolve_input"); }
}
