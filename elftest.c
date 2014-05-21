#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <elf.h>

unsigned long wrapperConvertAddress(char *inFile, char *convertAddr);
unsigned long convertAddress(char *buf, unsigned long bufSize, char *convertAddr);

int main(int argc, char **argv)
{
	if(argc == 3)
	{
		printf("[I] %08x\n", wrapperConvertAddress(argv[1], argv[2]));
	}
	return 0;
}

unsigned long wrapperConvertAddress(char *inFile, char *convertAddr)
{
  printf("[I] wrapperConvertAddress(%s,%s)\n", inFile, convertAddr);
  char *mBuf = NULL;
  unsigned long fSize = 0;

  FILE *f = fopen(inFile,"rb");
  fseek(f,0,SEEK_END);
  fSize = ftell(f);
  fseek(f,0,SEEK_SET);
  mBuf = (char *)malloc(fSize);
  memset(mBuf,0,fSize);
  int bytesRead = fread(mBuf,1,fSize,f);
  fclose(f);

  if(bytesRead != fSize || fSize == 0)
  {
   printf("[E] could not read file %s (fSize == %d, bytesRead == %d)\n", inFile,     fSize, bytesRead);
   return 0;
  }

  return convertAddress(mBuf, fSize, convertAddr);
}

unsigned long convertAddress(char *buf, unsigned long bufSize, char *convertAddr)
{
	Elf32_Ehdr *elf = (Elf32_Ehdr *)buf;
	if(elf->e_ident[0] == 0x7f && elf->e_ident[1] == 'E' && elf->e_ident[2] == 'L' && elf->e_ident[3] == 'F')
	{
		printf("[I] elf_magic ok\n");
	}
	else
	{
		return 0;
	}
	
	printf("[I] %x \n", elf->e_machine);


	return 0;
}
