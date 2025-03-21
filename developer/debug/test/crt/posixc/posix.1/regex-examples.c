#include <regex.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

void regerror_test1()
{
  char errbuf[512];
  regex_t re;

  // This regex is invalid, regcomp should fail.
  int i = regcomp(&re, "hello([0-9*world", REG_EXTENDED);
  assert (i != 0);
  // Get the failure error message
  regerror(i,&re,errbuf,sizeof(errbuf));
  printf("(expected) regcomp error string: %s\n", errbuf);
}

//Simple match/no-match usage
void regex_match_test1()
{
  regex_t re;

  int i = regcomp(&re, "hello([0-9]*)world", REG_EXTENDED|REG_NOSUB);
  assert(i==0);

  i = regexec(&re, "hello42world",0,NULL,0);
  assert(i == 0);

  i = regexec(&re, "helloworld",0,NULL,0);
  assert(i == 0);

  i = regexec(&re, "hello4-2world",0,NULL,0);
  assert(i == REG_NOMATCH);

  regfree(&re);
}

void regex_match_test2()
{
  regex_t re;
  regmatch_t matches[10];

  int i = regcomp(&re, "hello([0-9]*)world", REG_EXTENDED);
  assert(i==0);

  const char *data = "hello42world";
  i = regexec(&re, data,
              sizeof(matches)/sizeof(matches[0]),
              (regmatch_t*)&matches,0);
  assert(i == 0);

  // First element [0] is always the entire match
  assert(matches[0].rm_so==0);
  assert(matches[0].rm_eo==12);

  // Second element [1] is the first matched group (should match the '42')
  assert(matches[1].rm_so==5);
  assert(matches[1].rm_eo==7);

  // Extract it
  char *val = strndup(data+matches[1].rm_so,
                      matches[1].rm_eo - matches[1].rm_so);
  assert ( strcmp(val, "42")==0);
  free(val);

  regfree(&re);
}

int main()
{
  regerror_test1();
  regex_match_test1();
  regex_match_test2();
  return 0;
}
