#pragma once




#ifdef _WIN32
typedef char32_t platform_wchar_t;
#define PLATFORM_TEXT(str) U##str
#else
typedef wchar_t platform_wchar_t;
#define PLATFORM_TEXT(str) L##str
#endif






static const unsigned int XUIS_MAGIC = 0x53495558;

struct XUIS_Entry {
	unsigned short charCount; // Total count of characters.
	platform_wchar_t* string;
};

struct XUIS_Header {
public:
	unsigned int magic = 0;
	unsigned short unk1 = 0;
	int fileSize = 0;
	unsigned short entryCount = 0;
};

struct XUIS_File {
public:
	XUIS_Header header;
	XUIS_Entry* entries;
};
