#ifndef FRNL_UI_ACTION_EXECUTOR_H
#define FRNL_UI_ACTION_EXECUTOR_H
#include "frnl_ui_action_gate.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
  frnl_ui_realization realization;
  int session_valid;
  int user_consented;
  int target_window_matches;
} frnl_ui_action_snapshot;
typedef int (*frnl_ui_action_probe)(void *context, frnl_ui_action_snapshot *snapshot);
typedef int (*frnl_ui_action_callback)(void *context);
typedef enum {
  FRNL_UI_EXEC_DENIED = 0,
  FRNL_UI_EXEC_OK = 1,
  FRNL_UI_EXEC_FAILED = -1
} frnl_ui_action_result;
/* Probe current platform/session/consent/window state immediately before
   invoking a native action. Deny missing probes and failed checks. */
static inline frnl_ui_action_result frnl_ui_action_execute(
    frnl_ui_action_probe probe, void *probe_context,
    frnl_ui_action_callback action, void *action_context) {
  frnl_ui_action_snapshot snapshot = {FRNL_UI_UNKNOWN, 0, 0, 0};
  if (!probe || !action || probe(probe_context, &snapshot) != 0 ||
      frnl_ui_action_authorize(snapshot.realization, snapshot.session_valid,
                               snapshot.user_consented,
                               snapshot.target_window_matches) != FRNL_UI_ALLOWED)
    return FRNL_UI_EXEC_DENIED;
  return action(action_context) == 0 ? FRNL_UI_EXEC_OK : FRNL_UI_EXEC_FAILED;
}
#ifdef __cplusplus
}
#endif
#endif
