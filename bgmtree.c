#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libdis/libdis.h"
#include "bgmtree.h"

void printTree(char *disasmBuf, int size);

void bogomuppetTree()
{
	char firstByte = 0;
	char secondByte = 0;
	char thirdByte = 0;
	char disasmBuf[15];


	x86_insn_t insn;

	memset(disasmBuf,0,15);

	for(firstByte = 0; firstByte < 255; firstByte++)
	{
		if(firstByte == '\x66' || firstByte == '\x67')
		{
			// skip prefixes
			continue;
		}
		for(secondByte = 0;secondByte < 255; secondByte++)
		{
			for(thirdByte = 0; thirdByte < 255; thirdByte++)
			{
				disasmBuf[0] = firstByte;
				disasmBuf[1] = secondByte;
				disasmBuf[2] = thirdByte;
		
				int newSize = x86_disasm (disasmBuf, 15, 0, 0, &insn);

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
		printf("%02x:1\n", disasmBuf[0]);
	}
	else if(size == 2)
	{
		printf("%02x%02x%02x:2\n", disasmBuf[0], disasmBuf[1], disasmBuf[2]);
	}
	else
	{
		printf("%02x%02x%02x:%d\n", disasmBuf[0], disasmBuf[1], disasmBuf[2], size);
	}
}
