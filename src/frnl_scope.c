#include "frnl_scope.h"
#include <string.h>
static int nonempty(const char *s) { return s && *s; }
static size_t find(const frnl_scope_entry *entries, size_t count, const char *id) {
    size_t i;
    for (i=0; i<count; ++i) if (nonempty(entries[i].id) && strcmp(entries[i].id,id)==0) return i;
    return count;
}
frnl_scope_status frnl_scope_resolve(const frnl_scope_entry *entries, size_t count, const char *id, const char **out, size_t capacity, size_t *length) {
    size_t depth=0, i, j;
    const char *cursor=id;
    if (length) *length=0;
    if (!entries || !out || !length || !nonempty(id)) return FRNL_SCOPE_INVALID;
    while (nonempty(cursor)) {
        i=find(entries,count,cursor);
        if (i==count) return depth ? FRNL_SCOPE_MISSING_PARENT : FRNL_SCOPE_UNKNOWN;
        for (j=i+1;j<count;++j) if (nonempty(entries[j].id) && strcmp(entries[j].id,cursor)==0) return FRNL_SCOPE_DUPLICATE;
        for (j=0;j<depth;++j) if (strcmp(out[j],cursor)==0) return FRNL_SCOPE_CYCLE;
        if (depth>=capacity) return FRNL_SCOPE_CAPACITY;
        out[depth++]=entries[i].id;
        cursor=entries[i].parent_id;
    }
    for (i=0;i<depth/2;++i) { const char *tmp=out[i];out[i]=out[depth-i-1];out[depth-i-1]=tmp; }
    *length=depth;
    return FRNL_SCOPE_OK;
}
frnl_scope_status frnl_scope_validate(const frnl_scope_entry *entries, size_t count, const char *const *path, size_t length) {
    size_t i, index;
    if (!entries || !path || !length) return FRNL_SCOPE_INVALID;
    for (i=0;i<length;++i) if (!nonempty(path[i])) return FRNL_SCOPE_INVALID;
    index=find(entries,count,path[length-1]);
    if (index==count) return FRNL_SCOPE_UNKNOWN;
    for (i=length;i>0;--i) {
        const char *expected=entries[index].id;
        const char *parent=entries[index].parent_id;
        size_t j;
        if (strcmp(path[i-1],expected)!=0) return FRNL_SCOPE_MISMATCH;
        for (j=index+1;j<count;++j) if (nonempty(entries[j].id) && strcmp(entries[j].id,expected)==0) return FRNL_SCOPE_DUPLICATE;
        if (i==1) return nonempty(parent) ? FRNL_SCOPE_MISMATCH : FRNL_SCOPE_OK;
        if (!nonempty(parent)) return FRNL_SCOPE_MISMATCH;
        index=find(entries,count,parent);
        if (index==count) return FRNL_SCOPE_MISSING_PARENT;
    }
    return FRNL_SCOPE_INVALID;
}
