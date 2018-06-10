#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void main()
{
   char str[]="?ano =2017 &mes =10";

   char var1[10],var2[10],val1[10],val2[10];

   sscanf( str, "?%s=%s&%s=%s", var1, val1, var2, val2);

   printf( "%s = %s\n%s = %s\n",var1,val1,var2,val2);

}
