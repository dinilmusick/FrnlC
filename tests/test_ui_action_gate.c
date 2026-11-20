#include "frnl_ui_action_gate.h"
#include <stdio.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while (0)
int main(void) {
  int r, session, consent, window;
  for (r = FRNL_UI_UNKNOWN; r <= FRNL_UI_VISIBLE; ++r)
    for (session = 0; session <= 1; ++session)
      for (consent = 0; consent <= 1; ++consent)
        for (window = 0; window <= 1; ++window) {
          const int expected = r == FRNL_UI_VISIBLE && session && consent && window;
          CHECK(frnl_ui_action_authorize((frnl_ui_realization)r,session,consent,window) == (expected ? FRNL_UI_ALLOWED : FRNL_UI_DENIED));
        }
  CHECK(frnl_ui_action_authorize((frnl_ui_realization)99,1,1,1) == FRNL_UI_DENIED);
  CHECK(frnl_ui_action_authorize(FRNL_UI_VISIBLE,2,1,1) == FRNL_UI_DENIED);
  return 0;
}
