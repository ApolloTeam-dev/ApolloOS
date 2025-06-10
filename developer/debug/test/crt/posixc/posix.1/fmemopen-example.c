#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <err.h>

const char* data="\
[instaweb]\n\
local = true\n\
httpd = lighttpd\n\
port = 5151\n\
modulepath = /usr/lib/apache2/modules";

void fmemopen_test()
{
  size_t count=0;
  ssize_t len;
  char *line=NULL;
  size_t linecap=0;

  FILE *fp = fmemopen((void*)data,strlen(data),"r");
  if (fp==NULL)
    err(1,"fmemopen failed");

  while ((len = getline(&line, &linecap, fp)) > 0) {
    if (line[len-1]=='\n') {
      line[len-1]=0;
      --len;
    }
    ++count;

    printf("line %zu: %s\n", count, line);
  }
  if (ferror(fp))
    err(1,"memfile read error");

  if (fclose(fp)!=0)
    err(1,"fclose");

  free(line);
}


int main()
{
  fmemopen_test();
  return 0;
}
