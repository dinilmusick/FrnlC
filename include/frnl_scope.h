#ifndef FRNL_SCOPE_H
#define FRNL_SCOPE_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { const char *id; const char *parent_id; } frnl_scope_entry;
typedef enum { FRNL_SCOPE_OK=0, FRNL_SCOPE_INVALID, FRNL_SCOPE_UNKNOWN, FRNL_SCOPE_CYCLE, FRNL_SCOPE_MISSING_PARENT, FRNL_SCOPE_CAPACITY, FRNL_SCOPE_MISMATCH, FRNL_SCOPE_DUPLICATE } frnl_scope_status;
/* Resolves root-to-leaf scope; output points into entries and remains borrowed. */
frnl_scope_status frnl_scope_resolve(const frnl_scope_entry *entries, size_t count, const char *id, const char **out, size_t capacity, size_t *length);
frnl_scope_status frnl_scope_validate(const frnl_scope_entry *entries, size_t count, const char *const *path, size_t length);
#ifdef __cplusplus
}
#endif
#endif
