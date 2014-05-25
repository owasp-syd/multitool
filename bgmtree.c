#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libdis/libdis.h"
#include "bgmtree.h"

void printTree(char *disasmBuf, int size);

void bogomuppetTree()
{
	int firstByte = 0;
	int secondByte = 0;
	int thirdByte = 0;
	char disasmBuf[15];

	int newSize = 0;

	x86_insn_t insn;

	memset(disasmBuf,0,15);

	for(firstByte = 0; firstByte < 255; firstByte++)
	{
		disasmBuf[0] = firstByte;
		printf("a");
		if(firstByte == '\x66' || firstByte == '\x67')
		{
			continue;
		}
		for(secondByte = 0;secondByte < 255; secondByte++)
		{
			printf("b");
			disasmBuf[1] = secondByte;
			for(thirdByte = 0; thirdByte < 255; thirdByte++)
			{
				disasmBuf[2] = thirdByte;
		
				newSize = x86_disasm (disasmBuf, 15, 0, 0, &insn);

				printTree(disasmBuf,newSize);

				if(newSize == 1)
				{
					thirdByte = 255;
					secondByte = 255;
				}
				else if(newSize == 2)
				{
					thirdByte = 255;
				}
			}
		}
	}

	return;
}

void printTree(char *disasmBuf, int size)
{
	if(size == 1)
	{
		printf("%02x:+1\n", (unsigned char )disasmBuf[0]);
	}
	else if(size == 2)
	{
		printf("%02x%02x%02x:+2\n", (unsigned char )disasmBuf[0], (unsigned char )disasmBuf[1], (unsigned char )disasmBuf[2]);
	}
	else
	{
		printf("%02x%02x%02x:%d\n", (unsigned char )disasmBuf[0], (unsigned char)disasmBuf[1], (unsigned char )disasmBuf[2], size);
	}
}
