#include "revert_string.h"
#include "../swap/swap.h"

void RevertString(char *str)
{
	int i = 0;
	while(str[i]) i++;
	for(int j = 0; j < i / 2; j++)
	{
		Swap(&str[j], &str[i - j - 1]);
	}
}

