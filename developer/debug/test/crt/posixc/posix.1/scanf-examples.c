#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// scanf with skipped parsed items
// ('assignment suppressing')
void skip_vars()
{
  int num=0;

  // Skip 1 string and 1 float, and parse an int
  int i = sscanf("abcdefg 1.4 42",
                 "%*s %*f %d",
                 &num);
  assert(i == 1);
  assert(num == 42);
}

// scanf with maximum (fixed) length strings
void fixed_len()
{
  char data[11];
  char rest[1024];

  memset (data, 1, sizeof(data));
  memset (rest, 1, sizeof(rest));

  /* length (%10s) does NOT include the NULL - the buffer size is 11 bytes.
     If length is same as buffer size - buffer-overflow might happen. */
  int i = sscanf ("abcdefghijklmnopqrstuvwxyz 45",
                  "%10s %s",
                  (char*)&data, (char*)&rest);
  assert(i==2);
  assert(data[0] == 'a');
  assert(data[9] == 'j'); // 10 characters were read
  assert(data[10]== 0  ); //sscanf should add a NUL
  assert(rest[0] == 'k'); //scanning continues at the stopped position

  /* If the buffer is too small, the pasrsing of the next param (%d)
     will continue from the stopped position (character 'k') and will fail.
     In the example below, only one parameter will be parsed successfully (i==1). */
  int num=0;
  i = sscanf("abcdefghijklmnopqrstuvwxyz 45",
             "%10s %d",
             (char*)&data, &num);
  assert(i==1); //not 2 - the second parameter failed to parse
  assert(num==0); // it wasn't parsed, and the value wasn't changed.
}

//scanf will allocate the string (%ms)
void alloc_string()
{
  char *data=NULL;
  int num=0;

  int i = sscanf("hello-world-long-string 42",
                 "%ms %d", &data, &num);
  assert(i == 2);
  assert(num == 42);
  assert(data != NULL);
  assert(data[0] == 'h');
  assert(strlen(data) == 23);
  free(data);
}

//scanf with positional variables:
//  %3$9s  => third scanf parameter is the pointer a string.
//            store upto 9 characters in a 'char*'
//  %1$d   => first scanf parameter is the pointer to a int.
//  %2$c   => second scanf parameter is the pointer to a char.
void pos_var()
{
  int num;
  char c;
  char data[10];
  memset(data,1,sizeof(data));

  int i = sscanf("hello 42 f",
                 "%3$9s %1$d %2$c",
                 &num, &c, (char*)&data);

  assert(i == 3);
  assert(num == 42);
  assert(c == 'f');
  assert(data[0] == 'h');
  assert(data[4] == 'o');
  assert(data[5] == 0);
}

void match_chatset()
{
  int num=0;
  char data[10];
  memset(data,1,sizeof(data));

  //accept only lower-case letters
  int i = sscanf("helloWorld",
                 "%9[a-z]",
                 (char*)&data);

  assert(i == 1);
  assert(data[0] == 'h');
  assert(data[4] == 'o');
  assert(data[5] == 0);

  //Unlike other formats (e.g. %s), the match syntax
  //can be used to match whitespace
  i = sscanf("foo bar 43",
             "%9[a-z ] %d",
             (char*)&data, &num);

  assert(i == 2);
  assert(num == 43);
  assert(strcmp(data,"foo bar ")==0);
}

void scan_char()
{
  unsigned char c = 0 ;

  int i = sscanf("42", "%hhd", &c);
  assert(i==1);
  assert(c==42);

  //Internally scanf uses native int size (able to store '1000'),
  //when converting to char, value will reduced to 8-bits
  c = 0 ;
  i = sscanf("1000", "%hhd", &c);
  assert(i == 1);
  assert(c == (1000%256) );
}


int main()
{
  skip_vars();
  fixed_len();
  alloc_string();
  pos_var();
  match_chatset();
  scan_char();
  return 0;
}
