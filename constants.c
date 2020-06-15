#include "argus.h"

const unsigned int MaxLineSize = 1024;
const unsigned int ReadBufferSize = 4096;
const char * InputFIFOName = "ArgusInput";
const char * OutputFIFOName = "ArgusOutput";
const char * argusTag = "argus$ ";
const char * idxname = "logs.idx";
const char * logfilename = "logs";