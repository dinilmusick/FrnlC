#ifndef FRNL_GUTC_ADAPTER_H
#define FRNL_GUTC_ADAPTER_H
#include "frnl_semantic.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Optional HOST adapter. GutC's deterministic resolver uses malloc internally.
 * The FrnlC semantic core remains heap-free and operates without GutC.
 * JSON contracts/ids are borrowed. output_json is caller-owned storage, valid
 * until the next resolution on this adapter. The returned frnl_value borrows it. */
typedef struct { const char *id; const char *schema_json; } frnl_gutc_contract;
typedef struct {
  const frnl_gutc_contract *contracts;
  size_t contract_count;
  char *output_json;
  size_t output_capacity;
} frnl_gutc_adapter;
frnl_status frnl_gutc_resolve(void *context, const char *contract_id,
                              const char *raw_input, frnl_value *out);
#ifdef __cplusplus
}
#endif
#endif
