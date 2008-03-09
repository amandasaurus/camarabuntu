#include <stdio.h>
int main()
{
		    unsigned char x;  /* Must be unsigned for extended ASCII */
			   /* Print extended ASCII characters 180 through 203 */
			   for (x = 128; x < 255; x++)
						    {
										    printf("\nASCII code %d is character %c", x, x);
											   }
				  return 0;
}
