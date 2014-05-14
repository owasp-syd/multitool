/*
	multitool v0.1
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include "windefs.h"
#include "multitool.h"
#include "libdis/libdis.h"

char *inFile = NULL;
int opMode = OPMODE_NONE;
char *replaceText = NULL;
char *replaceWith = NULL;
char *findText = NULL;
char *writeText = NULL;
char *atAddress = NULL;
unsigned long _atAddr = 0;
char *paintTimes = NULL;
char *convertAddr = NULL;

int caOpmode = VIRTUAL_TO_PHYSICAL;

int main(int argc, char **argv)
{
	printf("[V] multitool v0.1 (%s)\n", __TIMESTAMP__);
	int argCount = getopt(argc, argv);

	if(argCount == 0)
	{
		return 0;
	}

	if(opMode == OPMODE_REPLACE && inFile != NULL && replaceText != NULL && replaceWith != NULL)
	{
		// printf("[I] entering replace mode\n");
		replaceString(inFile, replaceText, replaceWith);
	}
	else if(opMode == OPMODE_FIND && inFile != NULL && findText != NULL)
	{
		findString(inFile, findText);
	}
	else if(opMode == OPMODE_WRITE && inFile != NULL && writeText != NULL && atAddress != NULL)
	{
		writeString(inFile, writeText, atAddress);
	}
	else if(opMode == OPMODE_PAINT && inFile != NULL && writeText != NULL && atAddress != NULL && paintTimes != NULL)
	{
		paintString(inFile, writeText, atAddress, paintTimes);
	}
	else if(opMode == OPMODE_CONVERT && inFile != NULL && convertAddr != NULL)
	{
		unsigned long u_convertAddr = wrapperConvertAddress(inFile, convertAddr);
		printf("[I] %s is %08x\n", convertAddr, u_convertAddr);
	}
	else if(opMode == OPMODE_DISASSEMBLE && inFile != NULL && atAddress != NULL)
	{
		int disassembleCount = 5;
		if(paintTimes != NULL)
		{
			disassembleCount = atoi(paintTimes);
		}
		x86_init (opt_none, NULL, NULL);
		disassembleAt(inFile, atAddress, disassembleCount);
	}
	else if(opMode == OPMODE_NOP && inFile != NULL && atAddress != NULL)
	{
		int disassembleCount = 1;
		if(paintTimes != NULL)
		{
			disassembleCount = atoi(paintTimes);
		}
		x86_init (opt_none, NULL, NULL);
		nopAt(inFile, atAddress, disassembleCount);
	}
	else
	{
		usage();
	}
	return 0;
}

int getopt(int argc, char **argv)
{
	if(argc < 2 || argv == NULL)
	{
		printf("[E] bad argument string supplied to getopt\n");
		return 0;
	}

	int argHead = 1;
	while(argHead < argc)
	{
		if(strcmp(argv[argHead],"-in") == 0 && argHead < argc - 1)
		{
			inFile = argv[argHead + 1];
			argHead++;
		}
		else if(strcmp(argv[argHead],"-replace") == 0 && argHead < argc - 1 && opMode == OPMODE_NONE)
		{
			opMode = OPMODE_REPLACE;
			replaceText = argv[argHead + 1];
			argHead++;
		}
		else if(strcmp(argv[argHead],"-with") == 0 && argHead < argc - 1 && opMode == OPMODE_REPLACE && replaceWith == NULL)
		{
			replaceWith = argv[argHead + 1];
			argHead++;
		}
		else if(strcmp(argv[argHead],"-find") == 0 && argHead < argc - 1 && opMode == OPMODE_NONE)
		{
			opMode = OPMODE_FIND;
			findText = argv[argHead + 1];
			argHead++;
		}
		else if(strcmp(argv[argHead],"-write") == 0 && argHead < argc - 1 && opMode == OPMODE_NONE)
		{
			opMode = OPMODE_WRITE;
			writeText = argv[argHead + 1];
			argHead++;
		}
		else if(strcmp(argv[argHead],"-paint") == 0 && argHead < argc - 1 && opMode == OPMODE_NONE)
		{
			opMode = OPMODE_PAINT;
			writeText = argv[argHead + 1];
			argHead++;
		}
		else if(strcmp(argv[argHead],"-times") == 0 && argHead < argc - 1)
		{
			paintTimes = argv[argHead + 1];
			argHead++;
		}
		else if(strcmp(argv[argHead],"-convert") == 0 && argHead < argc - 1 && opMode == OPMODE_NONE)
		{
			opMode = OPMODE_CONVERT;
			convertAddr = argv[argHead + 1];
			argHead++;
		}
		else if(strcmp(argv[argHead],"-at") == 0 && argHead < argc - 1)
		{
			atAddress = argv[argHead + 1];
			argHead++;
		}
		else if(strcmp(argv[argHead],"-disassemble") == 0 && opMode == OPMODE_NONE)
		{
			opMode = OPMODE_DISASSEMBLE;
		}
		else if(strcmp(argv[argHead],"-nop") == 0 && opMode == OPMODE_NONE)
		{
			opMode = OPMODE_NOP;
		}
		else
		{
			printf("[E] unparseable argument %s in position %d\n", argv[argHead], argHead);
			return 0;
		}
		argHead++;
	}

	return argHead - 1;
}

void findString(char *inFile, char *findText)
{
	char *findTextBin = parseHexString(findText);
	int findTextLen = strlen(findText) / 2;

	if(findTextBin == NULL)
	{
		return;
	}

	FILE *f = fopen(inFile,"rb+");
	fseek(f,0,SEEK_END);
	unsigned long fSize = ftell(f);
	fseek(f,0,SEEK_SET);

	char *fileBuf = (char *)malloc(fSize);
	memset(fileBuf,0,fSize);

	unsigned long fread_result = 0;
	if( (fread_result = fread(fileBuf, 1, fSize, f)) != fSize)
	{
		printf("[E] could not read from file %s (fread returned %d, expected %d)\n", inFile, fread_result, fSize);
		fclose(f);
		free(fileBuf);
		return;
	}

	unsigned long lastSpotted = 0;
	unsigned long searchResult = naiveSearch(fileBuf, fSize, findTextBin, findTextLen, 0);
	
	while(searchResult != fSize)
	{
		lastSpotted = searchResult;
		displayHexString(fileBuf, fSize, lastSpotted, findTextLen);
		// printf("[I] needle found in haystack at position %x\n", lastSpotted);
		searchResult = naiveSearch(fileBuf, fSize, findTextBin, findTextLen, lastSpotted + 1);
	}

	fclose(f);
	free(fileBuf);
	return;
}

void displayHexString(char *haystack, unsigned long haystackLen, unsigned long needlePos, int needleLen)
{
	int i = 0;
	char minidump[17];
	minidump[16] = '\0';
	printf("[%08x] :", needlePos);
	if(needlePos + 16 < haystackLen) // needle + 16 < haystackLen
	{
		for( i = 0;i < 16; i++)
		{
			printf(" %02x", (unsigned char )haystack[needlePos + i]);
			minidump[i] = isprint(haystack[needlePos + i]) ? haystack[needlePos + i] : '.';
		}
	}
	else // needle + 16 too big for haystackLen
	{
		for( i = 0;i < 16; i++)
		{
			if(needlePos + i < haystackLen) { printf(" %02x", (unsigned char )haystack[needlePos + i]); minidump[i] = isprint(haystack[needlePos + i]) ? haystack[needlePos + i] : '.'; }
			else { printf(" .."); minidump[i] = '.'; }
		}
	}
	printf(" : %s\n", minidump);
	return;
}

void replaceString(char *inFile, char *replaceText, char *replaceWith)
{
	char *replaceTextBin = parseHexString(replaceText);
	char *replaceWithBin = parseHexString(replaceWith);

	if(replaceTextBin == NULL || replaceWithBin == NULL)
	{

		printf("[E] replacestring needs a replace key, and something to replace it with\n");
		return;
	}
	
	FILE *f = fopen(inFile,"rb+");
	if(f == NULL)
	{
		printf("[E] could not open %s for r+ mode\n", inFile);
		return;
	}

	fseek(f,0,SEEK_END);
	unsigned long fSize = ftell(f);
	// printf("[I] fsize is %08x\n", fSize );
	fseek(f,0,SEEK_SET);
	// return;

	unsigned long replaceTextLen = strlen(replaceText) / 2;
	unsigned long replaceWithLen = strlen(replaceWith) / 2;


	char *fileBuf = (char *)malloc(fSize + replaceWithLen);
	memset(fileBuf,0,fSize);

	unsigned long fread_result = 0;
	if( (fread_result = fread(fileBuf, 1, fSize, f)) != fSize)
	{
		printf("[E] could not read from file %s (fread returned %d, expected %d)\n", inFile, fread_result, fSize);
		fclose(f);
		free(fileBuf);
		return;
	}

	unsigned long instanceCount = 0;
	unsigned long lastSpotted = 0xFFFFFFFF;
	unsigned long searchResult = 0;

	searchResult = naiveSearch(fileBuf, fSize, replaceTextBin, replaceTextLen, 0);

	while(searchResult != fSize)
	{
		lastSpotted = searchResult;
		printf("[I] needle found in haystack at position %x\n", lastSpotted);
		instanceCount++;
		searchResult = naiveSearch(fileBuf, fSize, replaceTextBin, replaceTextLen, lastSpotted + 1);
	}

	if(instanceCount == 1)
	{
		// start writing data...
		printf("[I] replacing instance of %s at %08x\n", replaceText, lastSpotted);
		char *replaceBuffer = (char *)malloc(fSize - lastSpotted - replaceTextLen);
		printf("[I] copying %d bytes to a temp buffer...\n", fSize - lastSpotted - replaceTextLen);
		memcpy(replaceBuffer,fileBuf + lastSpotted + replaceTextLen, fSize - lastSpotted - replaceTextLen);
		printf("[I] replacing with size %d needle...\n", replaceWithLen);
		memcpy(fileBuf + lastSpotted, replaceWithBin, replaceWithLen);
		printf("[I] repairing buffer...\n");
		memcpy(fileBuf + lastSpotted + replaceWithLen, replaceBuffer, fSize - lastSpotted - replaceTextLen);
		// all data written
		printf("[I] closing existing file handle\n");
		fclose(f);
		printf("[I] opening file again in w+ mode, writing %d bytes (replaceTextLen = %d, replaceWithLen = %d)\n", fSize - replaceTextLen + replaceWithLen, replaceTextLen, replaceWithLen);
		f = fopen(inFile,"wb+");
		fwrite(fileBuf,1,(unsigned long )(fSize - replaceTextLen + replaceWithLen),f);
		printf("[I] file written...\n");
	}
	else if(instanceCount == 0)
	{
		printf("[I] needle %s not found in haystack\n", replaceText);
	}
	else
	{
		printf("[I] %d instances found, use -replaceall to replace all instances\n", instanceCount);
	}

	// this is broken? why?
	free(fileBuf);
	fclose(f);
	
	return;
}

char *parseHexString(char *hexString)
{
	int hexLen = strlen(hexString) / 2;
	if(strlen(hexString) % 2 != 0)
	{
		printf("[E] hex string has an odd length\n");
		return NULL;
	}
	char *out = (char *)malloc(hexLen + 1);
	if(out == NULL)
	{
		printf("[E] malloc failed at %s:%d\n", __FILE__, __LINE__);
		return NULL;
	}
	memset(out,0,hexLen + 1);

	// temp buffer to use sscanf to read hex bytes.
	char hexByte[3];
	hexByte[0] = '\0';
	hexByte[1] = '\0';
	hexByte[2] = '\0';

	int i = 0;
	for(; i < hexLen; i++)
	{
		hexByte[0] = hexString[i * 2];
		hexByte[1] = hexString[(i * 2) + 1];
		char hexChar;
		int result = sscanf(hexByte,"%x", &hexChar);
		if(result == EOF || result != 1)
		{
			printf("[E] failed to parse something in %s\n", hexString);
			return NULL;
		}
		out[i] = hexChar;
	}
	
	return out;
}

// this isn't finding all the instances of single-letter keys?
unsigned long  naiveSearch(char *haystack, unsigned long haystackLen, char *needle, int needleLen, int startLen)
{
	// printf("[I] starting naive search for size %d needle in size %d haystack, starting at %d\n", needleLen, haystackLen, startLen);
	unsigned long  i = startLen;
	int needlePtr = 0;

	for( ; i + needleLen - 1 < haystackLen;i++)
	{
		/*
			if(i == 0x16F79C)
			{
				printf("%02x == %02x (%d)\n", haystack[i], needle[0], needleLen);
			}
		*/
		if(haystack[i] == needle[0] && needleLen != 1)
		{
			for( needlePtr = 1; needlePtr < needleLen; needlePtr++)
			{
				if(needle[needlePtr] != haystack[i + needlePtr])
				{
					goto notFound;
				}
			}
			return i;

			notFound:;
		}
		else if(haystack[i] == needle[0])
		{
			return i;
		}
	}

	return haystackLen;
}

// completely self-contained. DO NOT FOPEN BEFORE THIS.
void writeString(char *_inFile, char *_writeText, char *_atAddress)
{
	unsigned long writeLocation = 0;
	if(_atAddress[0] == 'v')
	{
		writeLocation = wrapperConvertAddress(_inFile, _atAddress);
	}
	else
	{
		sscanf(_atAddress, "%x", &writeLocation);
	}

	if(writeLocation == 0 && !(atAddress[0] == '0' && atAddress[1] == '\0') )
	{
		printf("[E] write location %s cannot be parsed correctly\n", _atAddress);
		return;
	}

	printf("[I] attempting write to %x\n", writeLocation);

	char *writeBuf = parseHexString(_writeText);
	int writeLen = strlen(_writeText) / 2;

	if(writeBuf == NULL)
	{
		return;
	}

	// printf("");

	FILE *f = fopen(inFile,"rb+");
	if(f == NULL)
	{
		printf("[E] fopen file %s for write failed\n", _inFile);
		return;
	}

	if(fseek(f,writeLocation,SEEK_SET) != 0)
	{
		printf("[E] fseek operation to %s in file %s failed\n", _atAddress, _inFile);
		fclose(f);
		return;
	}

	int fwriteResult = fwrite(writeBuf,1,writeLen,f);

	if(fwriteResult != 0)
	{
		printf("[I] fwrite ok, returned %d\n", fwriteResult);
	}
	else
	{
		printf("[E] fwrite failed, returned 0\n");
		fclose(f);
		return;
	}

	fclose(f);

	return;
}

void paintString(char *_inFile, char *_writeText, char *_atAddress, char *_paintTimes)
{
	unsigned long writeLocation = 0;
	if(_atAddress[0] == 'v')
	{
		writeLocation = wrapperConvertAddress(_inFile, _atAddress);
	}
	else
	{
		sscanf(_atAddress, "%x", &writeLocation);
	}

	if(writeLocation == 0 && !(atAddress[0] == '0' && atAddress[1] == '\0') )
	{
		printf("[E] write location %s cannot be parsed correctly\n");
		return;
	}

	unsigned long paintTimes = 0;
	sscanf(_paintTimes, "%d", &paintTimes);

	printf("[I] attempting write to %x\n", writeLocation);

	char *writeBuf = parseHexString(_writeText);
	int writeLen = strlen(_writeText) / 2;

	if(writeBuf == NULL)
	{
		return;
	}

	// printf("");

	FILE *f = fopen(inFile,"rb+");
	if(f == NULL)
	{
		printf("[E] fopen file %s for write failed\n", _inFile);
		return;
	}

	if(fseek(f,writeLocation,SEEK_SET) != 0)
	{
		printf("[E] fseek operation to %s in file %s failed\n", _atAddress, _inFile);
		fclose(f);
		return;
	}

	int i = 0;
	int fwriteResult = 0;
	for(i = 0;i < paintTimes;i++)
	{
		fwriteResult = fwrite(writeBuf,1,writeLen,f);
		if(fwriteResult == 0)
		{
			printf("[E] fwrite failed, returned 0\n");
			fclose(f);
			return;
		}
	}

	fclose(f);

	return;
}

void usage()
{
	printf("[I] multitool -in [file] -replace [hexstring] -with [hexstring]\n");
	printf("[I] multitool -in [file] -find [hexstring]\n");
	printf("[I] multitool -in [file] -write [hexstring] -at v?[hexaddr]\n");
	printf("[I] multitool -in [file] -paint [hexstring] -at v?[hexaddr] -times [writecount]\n");
	printf("[I] multitool -in [file] -convert [p/v][hexaddr]\n");
	printf("[I] multitool -disassemble -in [file] -at [p/v][hexaddr] -times [instructoincount=5]\n");
	printf("[I] multitool -nop -in [file] -at [p/v][hexaddr] -times [instructoincount=5]\n");
	return;
}

unsigned long convertAddress(char *mBuf, unsigned long bufLen, char *addrOrig)
{
    unsigned long targetAddr;
    char *addr = addrOrig;

    if(addrOrig[0] == 'p') // address is physical
    {
        caOpmode = PHYSICAL_TO_VIRTUAL;
        addr = (char *)(addrOrig + 1);
    }
    else if(addrOrig[0] == 'v') // address is virtual
    {
        caOpmode = VIRTUAL_TO_PHYSICAL;
        addr = (char *)(addrOrig + 1);
    }
	else
	{
		printf("[E] first character of address must be 'p' if it is physical, or 'v' if it is virtual\n");
		return 0;
	}

    sscanf(addr, "%x", &targetAddr);
    IMAGE_DOS_HEADER *imgDosHdr = (IMAGE_DOS_HEADER *)mBuf;
	if((unsigned short )imgDosHdr->e_magic != 0x5a4d)
	{
		printf("[E] convertAddress(): file is not an executable\n");
		return 0;
	}

    // printf("* %04x\n", (unsigned short )imgDosHdr->e_magic);
    IMAGE_NT_HEADERS *imgNtHdrs = (IMAGE_NT_HEADERS *)(mBuf + imgDosHdr->e_lfanew);

    IMAGE_SECTION_HEADER *imgSectionHdr = (IMAGE_SECTION_HEADER *)((unsigned long )imgNtHdrs + (unsigned long )sizeof(IMAGE_NT_HEADERS));

    // printf("* imgSectionHdr = %08x\n", (unsigned long )imgSectionHdr - (unsigned long )mBuf);
    int i = 0;
	if(imgNtHdrs->FileHeader.NumberOfSections == 0)
	{
		printf("[E] convertAddress(): zero sections. maybe the section table is destroyed?\n");
		return 0;
	}
    // printf("* NumberOfSections = %d\n", imgNtHdrs->FileHeader.NumberOfSections);

    unsigned long imageBase = imgNtHdrs->OptionalHeader.ImageBase;
    // printf("* imagebase = %08x\n", imageBase);
    
    for(i = 0;i < imgNtHdrs->FileHeader.NumberOfSections;i++)
    {
        // printf("* [SECTION %02d:%08x] %s VirtualAddress:%08x, SizeOfRawData:%08x\n", i, (unsigned long )imgSectionHdr, (char *)imgSectionHdr->Name, (unsigned long )imgSectionHdr->VirtualAddress, (unsigned long )imgSectionHdr->SizeOfRawData);
        unsigned long sectionStart = imageBase + imgSectionHdr->VirtualAddress;
        unsigned long sectionEnd = imageBase + imgSectionHdr->VirtualAddress + imgSectionHdr->Misc.VirtualSize;
		
        if(caOpmode == VIRTUAL_TO_PHYSICAL)
        {
            if(targetAddr >= sectionStart && targetAddr <= sectionEnd)
            {
                // printf("[I] %08x\n", (unsigned long )(imgSectionHdr->PointerToRawData + targetAddr - sectionStart));
				unsigned long result = (unsigned long )(imgSectionHdr->PointerToRawData + targetAddr - sectionStart);
				//free(mBuf);
			    // fclose(f);
				return result;
            }
        }
        else
        {
            if(targetAddr > imgSectionHdr->PointerToRawData && (targetAddr - imgSectionHdr->PointerToRawData <= sectionEnd - sectionStart) )
            {
                // printf("[I] %08x\n", (unsigned long )targetAddr - imgSectionHdr->PointerToRawData + sectionStart);
				unsigned long result = (unsigned long )targetAddr - imgSectionHdr->PointerToRawData + sectionStart;
				// free(mBuf);
			    // fclose(f);
				return result;
            }
        }
        imgSectionHdr += 1;
        
    }

    return 0;
}

void nopAt(char *_inFile, char *_atAddress, int disassembleTimes)
{
	FILE *f = fopen(_inFile,"wb+");
	if(f == NULL)
	{
		printf("[E] could not open %s\n", _inFile);
		return;
	}

	fseek(f,0,SEEK_END);
	unsigned long fSize = ftell(f);
	fseek(f,0,SEEK_SET);
	char *fileBuf = (char *)malloc(fSize);
	if(fileBuf == NULL)
	{
		printf("[E] could not malloc %d bytes\n", fSize);
		fclose(f);
		return;
	}

	unsigned long bytesRead = fread(fileBuf,1,fSize,f);
	if(bytesRead != fSize)
	{
		printf("[E] could not read file: expected %d bytes, got %d bytes\n", fSize, bytesRead);
		fclose(f);
		free(fileBuf);
		return;
	}


	unsigned long readAddress = 0;

	if(_atAddress[0] == 'v')
	{
		readAddress = convertAddress(fileBuf, fSize, _atAddress);
	}
	else
	{
		sscanf(_atAddress,"%x",&readAddress);
	}

	if(readAddress == 0 && !(_atAddress[0] == '0' && _atAddress[1] == '\0'))
	{
		printf("[E] section table address conversion failed\n");
		return;
	}

	x86_insn_t insn;
	char line[256];	
	memset(line,0,256);
	char nopBuf = '\x90';

	unsigned long currentCount = 0;
	
	int i = 0;
	for( ; i < disassembleTimes; i++)
	{
		if( (unsigned long )(readAddress + currentCount + MAX_INSTR_SIZE) > fSize)
		{
			printf("[I] (readAddress=%08x + currentCount=%x + maxInstrSize=15 > fSize=%08x", readAddress, currentCount, fSize);
			break;
		}
		int newSize = x86_disasm ((unsigned char *)(fileBuf + readAddress + currentCount), MAX_INSTR_SIZE, 0, 0, &insn);
		x86_format_insn (&insn, line, 256, intel_syntax);
		printf("[+] %s\n", line);
		// writing nops
		fseek(f,(unsigned long )(readAddress + currentCount),SEEK_SET);
		int nopCount = 0;
		for( ; nopCount < newSize; nopCount++)
		{
			fwrite(&nopBuf,1,1,f);
		}
		currentCount += newSize;
	}

	fclose(f);
	return;
}

void disassembleAt(char *_inFile, char *_atAddress, int disassembleTimes)
{
	FILE *f = fopen(_inFile,"rb");
	if(f == NULL)
	{
		printf("[E] could not open %s\n", _inFile);
		return;
	}

	fseek(f,0,SEEK_END);
	unsigned long fSize = ftell(f);
	fseek(f,0,SEEK_SET);
	char *fileBuf = (char *)malloc(fSize);
	if(fileBuf == NULL)
	{
		printf("[E] could not malloc %d bytes\n", fSize);
		fclose(f);
		return;
	}

	unsigned long bytesRead = fread(fileBuf,1,fSize,f);
	if(bytesRead != fSize)
	{
		printf("[E] could not read file: expected %d bytes, got %d bytes\n", fSize, bytesRead);
		fclose(f);
		free(fileBuf);
		return;
	}

	fclose(f);

	unsigned long readAddress = 0;

	if(_atAddress[0] == 'v')
	{
		readAddress = convertAddress(fileBuf, fSize, _atAddress);
	}
	else
	{
		sscanf(_atAddress,"%x",&readAddress);
	}

	if(readAddress == 0 && !(_atAddress[0] == '0' && _atAddress[1] == '\0'))
	{
		printf("[E] section table address conversion failed\n");
		return;
	}

	x86_insn_t insn;
	char line[256];	
	memset(line,0,256);

	unsigned long currentCount = 0;
	
	int i = 0;
	for( ; i < disassembleTimes; i++)
	{
		if( (unsigned long )(readAddress + currentCount + MAX_INSTR_SIZE) > fSize)
		{
			printf("[I] (readAddress=%08x + currentCount=%x + maxInstrSize=15 > fSize=%08x", readAddress, currentCount, fSize);
			break;
		}
		int newSize = x86_disasm ((unsigned char *)(fileBuf + readAddress + currentCount), MAX_INSTR_SIZE, 0, 0, &insn);
		currentCount += newSize;
		x86_format_insn (&insn, line, 256, intel_syntax);
		printf("[+] %s\n", line);
		// printf("%08x : %s\n", (unsigned long )startOffset, line);
	}

	return;
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
		printf("[E] could not read file %s (fSize == %d, bytesRead == %d)\n", inFile, fSize, bytesRead);
		return 0;
	}
	
	return convertAddress(mBuf, fSize, convertAddr);
}