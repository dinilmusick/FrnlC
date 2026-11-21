#include "frnl_gutc_adapter.h"
#include <gutc/semantic_deterministic.h>
#include <gutc/json_dom.h>
#include <string.h>
frnl_status frnl_gutc_resolve(void *context, const char *id,
                              const char *raw, frnl_value *out) {
  frnl_gutc_adapter *a=(frnl_gutc_adapter*)context;
  gutc_json_dom *schema=NULL,*resolution=NULL;
  const char *schema_json=NULL;
  gutc_status rc;
  frnl_status status;
  size_t i,error_at=0,needed=0;
  int resolved=0;
  if(out) { out->data=NULL; out->size=0; }
  if(!a||!out||!id||!*id||!raw||!a->output_json||!a->output_capacity ||
     (!a->contracts && a->contract_count)) return FRNL_INVALID;
  for(i=0;i<a->contract_count;i++) {
    if(a->contracts[i].id && strcmp(id,a->contracts[i].id)==0) {
      schema_json=a->contracts[i].schema_json; break;
    }
  }
  if(!schema_json) return FRNL_NOT_FOUND;
  rc=gutc_json_dom_parse((const unsigned char*)schema_json,strlen(schema_json),
                         32,1000,&schema,&error_at);
  if(rc!=GUTC_OK) return rc==GUTC_NO_MEMORY?FRNL_CAPACITY:FRNL_INVALID;
  rc=gutc_resolve_deterministic_semantic(raw,strlen(raw),schema,&resolution,&resolved);
  gutc_json_dom_free(schema);
  if(rc!=GUTC_OK) {
    gutc_json_dom_free(resolution);
    if(rc==GUTC_UNSUPPORTED) return FRNL_UNSUPPORTED;
    return rc==GUTC_NO_MEMORY?FRNL_CAPACITY:FRNL_INVALID;
  }
  if(!resolved || !resolution) {
    gutc_json_dom_free(resolution);
    return FRNL_NOT_FOUND;
  }
  rc=gutc_json_dom_stringify(resolution,a->output_json,a->output_capacity,&needed);
  gutc_json_dom_free(resolution);
  status=rc==GUTC_OK?FRNL_OK:
    (rc==GUTC_NO_SPACE || rc==GUTC_NO_MEMORY ? FRNL_CAPACITY :
     rc==GUTC_UNSUPPORTED ? FRNL_UNSUPPORTED : FRNL_INVALID);
  if(status==FRNL_OK) { out->data=a->output_json; out->size=needed; }
  return status;
}
