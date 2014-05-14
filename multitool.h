#define OPMODE_NONE 0
#define OPMODE_REPLACE 1
#define OPMODE_FIND 2
#define OPMODE_WRITE 3
#define OPMODE_PAINT 4
#define OPMODE_CONVERT 5
#define OPMODE_DISASSEMBLE 6
#define OPMODE_NOP 7
#define OPMODE_USAGE 8

#define VIRTUAL_TO_PHYSICAL 0
#define PHYSICAL_TO_VIRTUAL 1

#define MAX_INSTR_SIZE 15

int getopt(int argc, char **argv);
void replaceString(char *inFile, char* replaceText, char *replaceWith);
void findString(char *inFile, char* findText);
char *parseHexString(char *hexString);
unsigned long  naiveSearch(char *haystack, unsigned long haystackLen, char *needle, int needleLen, int startLen);
void findString(char *inFile, char *findText);
void displayHexString(char *haystack, unsigned long haystackLen, unsigned long needlePos, int needleLen);
void writeString(char *_inFile, char *_writeText, char *_atAddress);
void paintString(char *_inFile, char *_writeText, char *_atAddress, char *_paintTimes);
void usage();
unsigned long wrapperConvertAddress(char *inFile, char *addrOrig);
unsigned long convertAddress(char *fileBuffer, unsigned long bufLen, char *addrOrig);
void disassembleAt(char *_inFile, char *_atAddress, int disassembleTimes);
void nopAt(char *_inFile, char *_atAddress, int disassembleTimes);
