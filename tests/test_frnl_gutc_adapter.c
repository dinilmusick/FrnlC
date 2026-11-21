#include "frnl_gutc_adapter.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
int main(void) {
  frnl_runtime rt; frnl_storage storage={0}; frnl_value result={0};
  char buffer[512],small[3];
  const frnl_gutc_contract contracts[]={
    {"number","{\"type\":\"number\"}"},
    {"regex","{\"type\":\"string\",\"pattern\":\"^a+$\"}"}
  };
  frnl_gutc_adapter adapter={contracts,2,buffer,sizeof buffer};
  storage.max_depth=1;
  assert(frnl_init(&rt,&storage)==FRNL_OK);
  assert(frnl_resolve_input(&rt,"number","42",&result)==FRNL_UNSUPPORTED);
  frnl_bind_input(&rt,frnl_gutc_resolve,&adapter);
  assert(frnl_resolve_input(&rt,"number","42",&result)==FRNL_OK);
  assert(result.data==buffer && result.size>20);
  assert(strstr((const char*)result.data,"\"status\":\"resolved\"")!=NULL);
  assert(strstr((const char*)result.data,"\"value\":42")!=NULL);
  assert(frnl_resolve_input(&rt,"number","not-a-number",&result)==FRNL_NOT_FOUND);
  assert(result.data==NULL && result.size==0);
  assert(frnl_resolve_input(&rt,"regex","aaa",&result)==FRNL_UNSUPPORTED);
  assert(frnl_resolve_input(&rt,"unknown","42",&result)==FRNL_NOT_FOUND);
  adapter.output_json=small; adapter.output_capacity=sizeof small;
  assert(frnl_resolve_input(&rt,"number","42",&result)==FRNL_CAPACITY);
  assert(result.data==NULL);
  frnl_cleanup(&rt);
  puts("FrnlC pinned GutC adapter PASS");
  return 0;
}
