#include "frnl_ui_action_executor.h"
#include <assert.h>
typedef struct {
  int probes, calls, probe_fails, action_fails;
  frnl_ui_action_snapshot state;
} fixture;
static int probe(void *p, frnl_ui_action_snapshot *out) {
  fixture *f = p;
  ++f->probes;
  if (f->probe_fails) return -1;
  *out = f->state;
  return 0;
}
static int action(void *p) {
  fixture *f = p;
  ++f->calls;
  return f->action_fails ? -1 : 0;
}
int main(void) {
  fixture f = {0, 0, 0, 0, {FRNL_UI_VISIBLE, 1, 1, 1}};
  assert(frnl_ui_action_execute(probe,&f,action,&f)==FRNL_UI_EXEC_OK);
  assert(f.probes==1 && f.calls==1);
  f.state.user_consented=0;
  assert(frnl_ui_action_execute(probe,&f,action,&f)==FRNL_UI_EXEC_DENIED);
  assert(f.probes==2 && f.calls==1);
  f.state.user_consented=1; f.state.target_window_matches=0;
  assert(frnl_ui_action_execute(probe,&f,action,&f)==FRNL_UI_EXEC_DENIED);
  assert(f.calls==1);
  f.state.target_window_matches=1; f.probe_fails=1;
  assert(frnl_ui_action_execute(probe,&f,action,&f)==FRNL_UI_EXEC_DENIED);
  assert(f.calls==1);
  f.probe_fails=0; f.action_fails=1;
  assert(frnl_ui_action_execute(probe,&f,action,&f)==FRNL_UI_EXEC_FAILED);
  assert(f.calls==2);
  assert(frnl_ui_action_execute(0,&f,action,&f)==FRNL_UI_EXEC_DENIED);
  assert(frnl_ui_action_execute(probe,&f,0,&f)==FRNL_UI_EXEC_DENIED);
  return 0;
}
