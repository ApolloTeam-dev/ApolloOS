#include <stdlib.h>
#include <stdio.h>
#include <err.h>

//Prints only up to the first 5 characters of a string
void printf_substring()
{
	const char *data = "hello-world";
	printf("First 5 characters = '%.5s'\n", data);


	//If the string is shorter than 5 characters,
	//it must be NUL terminated.
	const char *data2= "foo";
	printf("First 5 characters = '%.5s'\n", data2);
}

void printf_varlen_substr()
{
	const char *data = "hello-world";

	//variable length
	int len=6;
	printf("First %d characters = '%.*s'\n", len, len, data);
}

int main()
{
	printf_substring();
	printf_varlen_substr();
	return 0;
}
