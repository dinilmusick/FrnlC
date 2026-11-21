#include "frnl_scope.h"
#include <assert.h>
#include <string.h>
int main(void) {
 const frnl_scope_entry e[]={{"root",0},{"child","root"},{"leaf","child"}};
 const frnl_scope_entry cyclic[]={{"a","b"},{"b","a"}};
 const frnl_scope_entry missing[]={{"a","absent"}};
 const frnl_scope_entry duplicate[]={{"a",0},{"a",0}};
 const char *out[4];size_t n=0;
 const char *valid[]={"root","child","leaf"};
 const char *forged[]={"root","leaf"};
 assert(frnl_scope_resolve(e,3,"leaf",out,4,&n)==FRNL_SCOPE_OK && n==3);
 assert(strcmp(out[0],"root")==0 && strcmp(out[2],"leaf")==0);
 assert(frnl_scope_validate(e,3,valid,3)==FRNL_SCOPE_OK);
 assert(frnl_scope_validate(e,3,forged,2)==FRNL_SCOPE_MISMATCH);
 assert(frnl_scope_validate(e,3,valid,0)==FRNL_SCOPE_INVALID);
 assert(frnl_scope_resolve(e,3,"ghost",out,4,&n)==FRNL_SCOPE_UNKNOWN);
 assert(frnl_scope_resolve(e,3,"leaf",out,2,&n)==FRNL_SCOPE_CAPACITY);
 assert(frnl_scope_resolve(cyclic,2,"a",out,4,&n)==FRNL_SCOPE_CYCLE);
 assert(frnl_scope_resolve(missing,1,"a",out,4,&n)==FRNL_SCOPE_MISSING_PARENT);
 assert(frnl_scope_resolve(duplicate,2,"a",out,4,&n)==FRNL_SCOPE_DUPLICATE);
 return 0;
}
