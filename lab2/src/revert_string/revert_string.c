#include "revert_string.h"

void RevertString(char* str)
{
	int len = strlen(str);
    char *begin, *end;
    char temp;
    begin  = str;
    end    = str;
    for (int i = 0; i < len - 1; i++)
        end++;
 
    for (int i = 0; i < len/2; i++) {        
        temp   = *end;
        *end   = *begin;
        *begin = temp;
        begin++;
        end--;
    }
}
