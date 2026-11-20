#ifndef FRNL_UI_ACTION_GATE_H
#define FRNL_UI_ACTION_GATE_H
#ifdef __cplusplus
extern "C" {
#endif
/* Platform adapters must report the current realization of a native control.
   UNKNOWN is deliberately not equivalent to VISIBLE. */
typedef enum {
  FRNL_UI_UNKNOWN = 0,
  FRNL_UI_UNMOUNTED,
  FRNL_UI_MISSING,
  FRNL_UI_OFFSCREEN,
  FRNL_UI_VISIBLE
} frnl_ui_realization;
typedef enum {
  FRNL_UI_DENIED = 0,
  FRNL_UI_ALLOWED = 1
} frnl_ui_authority;
/* Call this immediately before every remote action, not just at discovery.
   A platform adapter owns the realization check; this module never assumes
   that a React DOM, browser, or particular native window system exists. */
static inline frnl_ui_authority frnl_ui_action_authorize(
    frnl_ui_realization realization, int session_valid,
    int user_consented, int target_window_matches) {
  return (realization == FRNL_UI_VISIBLE && session_valid == 1 &&
          user_consented == 1 && target_window_matches == 1)
             ? FRNL_UI_ALLOWED : FRNL_UI_DENIED;
}
#ifdef __cplusplus
}
#endif
#endif
