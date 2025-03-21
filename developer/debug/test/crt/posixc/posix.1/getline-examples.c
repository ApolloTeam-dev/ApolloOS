#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <err.h>

void readfile()
{
  char *line = NULL;
  size_t linecap = 0;
  ssize_t len;

  int count=0;
  size_t maxlen=0;

  const char* filename="/etc/passwd";

  FILE *fp = fopen(filename,"r");
  if (fp==NULL)
    err(1,"fopen(%s) failed",filename);

  // 'len' is the number of characters in the returned line,
  // including delimiter (if present), but NOT including NULL.
  // The 'while' condition guarentees len>0, thus [len-1] is always valid.
  while ((len = getline(&line, &linecap, fp)) > 0) {
    if (line[len-1]=='\n') {
      line[len-1]=0;
      --len;
    }
    if ((size_t)len>maxlen)
      maxlen = len;

    ++count;
  }
  if (ferror(fp))
    err(1,"read error on '%s'", filename);

  if (fclose(fp)!=0)
    err(1,"fclose(%s) failed",filename);

  free(line);

  printf("%d lines in %s, longest line had %zu characters\n",
         count,filename,maxlen);
}


int main()
{
  readfile();
  return 0;
}
