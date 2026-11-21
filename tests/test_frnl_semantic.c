#include "frnl_semantic.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

typedef struct {
 frnl_component components[3]; frnl_state_spec states[3]; frnl_state_edge edges[2];
 frnl_visual visuals[4]; frnl_data data[3]; frnl_event_def events[3];
 frnl_function funcs[3]; frnl_subscription subs[3],snap[6];
 frnl_flow_step flows[3],flow_snap[6];
 frnl_runtime rt;
} fixture;
static frnl_status renderer(void *ctx, frnl_render_kind kind, const frnl_component *c) {
  int *count=(int*)ctx;
  assert(c && kind<=FRNL_RENDER_VISUAL); (*count)++; return FRNL_OK;
}
static int notifications;
static frnl_status listener(void *ctx,const char *id,const char *event,const void *payload) {
 (void)ctx; (void)payload; assert(strcmp(id,"child")==0 && strcmp(event,"click")==0); notifications++; return FRNL_OK;
}
typedef struct { frnl_runtime *rt; uint64_t token; int nested; int calls; } invoke_context;
static frnl_status click(void *ctx,const void *payload) {
 invoke_context *c=ctx; (void)payload;
 c->calls++;
 if (c->token) assert(frnl_unsubscribe(c->rt,c->token)==FRNL_OK);
 if (c->nested) return frnl_emit(c->rt,"child","nested",NULL);
 return FRNL_OK;
}
static frnl_status nested_fail(void *ctx, const void *input) { (void)ctx;(void)input; return FRNL_INVALID; }
static int order[3],index_order;
static frnl_status step(void *ctx, const void *input) { (void)input; order[index_order++]=*(int*)ctx; return FRNL_OK; }
static frnl_status resolve(void *ctx,const char *contract,const char *raw,frnl_value *out) {
 (void)ctx; assert(strcmp(contract,"field")==0); out->data=raw;out->size=strlen(raw);return FRNL_OK;
}
static void setup(fixture *f) {
 frnl_storage s={0}; memset(f,0,sizeof(*f));
 s.components=f->components;s.component_capacity=3;s.states=f->states;s.state_capacity=3;
 s.edges=f->edges;s.edge_capacity=2;s.visuals=f->visuals;s.visual_capacity=4;
 s.data=f->data;s.data_capacity=3;s.events=f->events;s.event_capacity=3;
 s.functions=f->funcs;s.function_capacity=3;s.subscriptions=f->subs;s.subscription_capacity=3;
 s.subscription_scratch=f->snap;s.flows=f->flows;s.flow_capacity=3;s.flow_scratch=f->flow_snap;s.max_depth=2;
 assert(frnl_init(&f->rt,&s)==FRNL_OK);
 assert(frnl_define_state(&f->rt,(frnl_state_spec){"tile","idle",1,1})==FRNL_OK);
 assert(frnl_define_state(&f->rt,(frnl_state_spec){"tile","pressed",0,0})==FRNL_OK);
 assert(frnl_allow_state(&f->rt,(frnl_state_edge){"tile","idle","pressed"})==FRNL_OK);
 assert(frnl_add_component(&f->rt,"root",NULL,"tile","idle")==FRNL_OK);
 assert(frnl_add_component(&f->rt,"child","root","tile","idle")==FRNL_OK);
}
int main(void) {
 fixture f; const char *path[3];size_t n=0; int count=0;
 frnl_value v; invoke_context ctx;uint64_t token=0;int a=1,b=2,c=3;
 setup(&f);
 assert(frnl_add_component(&f.rt,"bad","missing","tile","idle")==FRNL_INVALID);
 assert(frnl_add_component(&f.rt,"root",NULL,"tile","idle")==FRNL_DUPLICATE);
 assert(frnl_component_path(&f.rt,"child",path,3,&n)==FRNL_OK && n==2);
 assert(strcmp(path[0],"root")==0 && strcmp(path[1],"child")==0);
 assert(frnl_check_path(&f.rt,path,n)==FRNL_OK);
 {const char *bad[]={"child","root"}; assert(frnl_check_path(&f.rt,bad,2)==FRNL_INVALID);}
 assert(frnl_component_path(&f.rt,"child",path,1,&n)==FRNL_CAPACITY);
 frnl_bind_renderer(&f.rt,renderer,&count);
 assert(frnl_lifecycle_transition(&f.rt,"child",FRNL_ACTIVE)==FRNL_BAD_TRANSITION);
 assert(frnl_lifecycle_transition(&f.rt,"child",FRNL_INITIALIZING)==FRNL_OK);
 assert(frnl_lifecycle_transition(&f.rt,"child",FRNL_MOUNTED)==FRNL_OK);
 assert(frnl_lifecycle_transition(&f.rt,"child",FRNL_ACTIVE)==FRNL_OK);
 assert(frnl_lifecycle_transition(&f.rt,"child",FRNL_ACTIVE)==FRNL_OK);
 assert(frnl_set_state(&f.rt,"child","pressed")==FRNL_OK);
 assert(f.components[1].phase==FRNL_RELEASE && strcmp(f.components[1].state,"idle")==0);
 assert(frnl_set_state(&f.rt,"child","pressed")==FRNL_BUSY);
 assert(frnl_advance_phase(&f.rt,"child")==FRNL_OK && f.components[1].phase==FRNL_SUSTAIN);
 assert(strcmp(f.components[1].state,"pressed")==0);
 assert(frnl_set_state(&f.rt,"child","idle")==FRNL_BAD_TRANSITION);
 assert(frnl_advance_phase(&f.rt,"child")==FRNL_BAD_TRANSITION);
 assert(frnl_visual_put(&f.rt,(frnl_visual){FRNL_VISUAL_BASE,"tile","pressed",NULL,NULL,"color",{"black",6}})==FRNL_OK);
 assert(frnl_visual_put(&f.rt,(frnl_visual){FRNL_VISUAL_THEME,"tile","pressed","dark",NULL,"color",{"gray",5}})==FRNL_OK);
 assert(frnl_set_theme(&f.rt,"child","dark")==FRNL_OK);
 assert(frnl_visual_resolve(&f.rt,"child","color",&v)==FRNL_OK && strcmp(v.data,"gray")==0);
 assert(frnl_visual_put(&f.rt,(frnl_visual){FRNL_VISUAL_OVERRIDE,NULL,NULL,NULL,"child","color",{"red",4}})==FRNL_OK);
 assert(frnl_visual_resolve(&f.rt,"child","color",&v)==FRNL_OK && strcmp(v.data,"red")==0);
 assert(frnl_data_put(&f.rt,(frnl_data){FRNL_DATA_PAGE,"page","value",{"text",5}})==FRNL_OK);
 assert(frnl_data_get(&f.rt,FRNL_DATA_PAGE,"page","value",&v)==FRNL_OK && strcmp(v.data,"text")==0);
 assert(frnl_data_put(&f.rt,(frnl_data){FRNL_DATA_COMPONENT,"child","count",{&a,sizeof(a)}})==FRNL_OK);
 assert(frnl_data_get(&f.rt,FRNL_DATA_COMPONENT,"child","count",&v)==FRNL_OK && *(const int*)v.data==1);
 assert(frnl_event_define(&f.rt,(frnl_event_def){"child","pressed","click","clickFn"})==FRNL_OK);
 assert(frnl_event_define(&f.rt,(frnl_event_def){"child","pressed","nested","failFn"})==FRNL_OK);
 ctx=(invoke_context){&f.rt,0,0,0};
 assert(frnl_function_add(&f.rt,(frnl_function){"child","clickFn",click,&ctx})==FRNL_OK);
 assert(frnl_function_add(&f.rt,(frnl_function){"child","failFn",nested_fail,NULL})==FRNL_OK);
 assert(frnl_subscribe(&f.rt,"child","click",listener,NULL,&token)==FRNL_OK);
 ctx.token=token;assert(frnl_emit(&f.rt,"child","click",NULL)==FRNL_OK);
 assert(notifications==1 && f.rt.subscriptions==0 && ctx.calls==1);
 ctx.token=0; assert(frnl_emit(&f.rt,"child","click",NULL)==FRNL_OK); /* removal visible on next invocation */
 ctx.token=0;ctx.nested=1;
 assert(frnl_emit(&f.rt,"child","click",NULL)==FRNL_HANDLER_ERROR);
 assert(f.rt.dispatch_depth==0);
 ctx.nested=0;
 assert(frnl_emit(&f.rt,"child","click",NULL)==FRNL_OK);
 assert(frnl_unsubscribe(&f.rt,token)==FRNL_NOT_FOUND);
 assert(frnl_flow_add(&f.rt,(frnl_flow_step){"flow",2,step,&b})==FRNL_OK);
 assert(frnl_flow_add(&f.rt,(frnl_flow_step){"flow",1,step,&a})==FRNL_OK);
 assert(frnl_flow_add(&f.rt,(frnl_flow_step){"flow",3,step,&c})==FRNL_OK);
 assert(frnl_flow_run(&f.rt,"flow",NULL)==FRNL_OK && index_order==3);
 assert(order[0]==1 && order[1]==2 && order[2]==3);
 assert(frnl_flow_add(&f.rt,(frnl_flow_step){"flow",4,step,&c})==FRNL_CAPACITY);
 assert(frnl_resolve_input(&f.rt,"field","hello",&v)==FRNL_UNSUPPORTED);
 frnl_bind_input(&f.rt,resolve,NULL);
 assert(frnl_resolve_input(&f.rt,"field","hello",&v)==FRNL_OK && v.size==5);
 assert(frnl_lifecycle_transition(&f.rt,"child",FRNL_DISPOSING)==FRNL_OK);
 assert(frnl_lifecycle_transition(&f.rt,"child",FRNL_DISPOSED)==FRNL_OK);
 assert(frnl_lifecycle_transition(&f.rt,"child",FRNL_ACTIVE)==FRNL_BAD_TRANSITION);
 assert(count>4);
 frnl_cleanup(&f.rt); setup(&f); frnl_cleanup(&f.rt);
 puts("FrnlC semantic runtime PASS"); return 0;
}