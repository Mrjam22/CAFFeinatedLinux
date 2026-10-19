#pragma once
#ifndef _GHOULBUNDLE
#define _GHOULBUNDLE
#include <vector>

static const size_t FILEENTRY_NAMESIZE = 0x80;
static const size_t FILEENTRY_SIZE = 160;

struct OffsetSet {
	int32_t offset;
	int32_t size;
};

struct GpuList {
	int32_t tableSize = 0;
	int32_t entryCount = 0;
	OffsetSet* offsets;
};

struct FileEntry {
	char fileName[0x80];

	int32_t type = 0;
	uint32_t timestamp = 0;
	float fUnk1 = 0;
	int32_t iUnk1 = 0;

	// Actual Section Info
	int32_t dataSectOffset = 0;
	int32_t dataSectSize = 0;
	int32_t gpuListOffset = 0;
	int32_t gpuSectAndListSize = 0;

	~FileEntry();
};

struct DirtyGpuSect {
	char* newData = nullptr;
	int32_t gpuSize = 0;
};

struct DirtyFile {
	char* newDataSect = nullptr;
	int32_t dataSize = 0;

	DirtyGpuSect* gpuSects;
};

struct GhoulBundle {
	char* dataPtr;

	bool isReady;
	bool hasErrored = false;

	int16_t entryCount;
	int16_t isCompressed;
	int32_t iUnk1;

	// Sections
	int32_t fileListOffset;
	int32_t fileListDataSize;
	int32_t gpuListOffset;
	int32_t gpuListSize;
	int32_t gpuSectOffset;
	int32_t gpuSectSize;
	int32_t dataSectOffset;
	int32_t dataSectSize;

	FileEntry* fileEntries;
	std::vector <GpuList> gpuList;

	DirtyFile* dirtyFiles;

	bool ReadBundleFile(char* data, size_t dataSize);
	bool readStandaloneBundleFile(char* fileName);
	void writeToFile(const char* fileName);

	void ClearBundleFileData();

	char* GetDataSection() {
		if (dataPtr == nullptr) return nullptr;

		char* data = (char*)malloc(dataSectSize);
		memcpy(data, dataPtr + dataSectOffset, dataSectSize);

		return data;
	}

	char* GetGpuSection() {
		if (dataPtr == nullptr) return nullptr;
		if (gpuSectSize == 0) return nullptr;

		char* data = (char*)malloc(gpuSectSize);
		memcpy(data, dataPtr + gpuSectOffset, gpuSectSize);

		return data;
	}

	char* GetGPUListSection() {
		if (dataPtr == nullptr) return nullptr;
		if (gpuListSize == 0) return nullptr;

		char* data = (char*)malloc(gpuListSize);
		memcpy(data, dataPtr + gpuListOffset, gpuListSize);

		return data;
	}

	char* GetFileData(int32_t idx) {
		if (idx < 0 || idx >= entryCount) return nullptr;

		char* data = GetDataSection();
		if (data == nullptr) return nullptr;

		char* fileData = (char*)malloc(fileEntries[idx].dataSectSize);
		memcpy(fileData, data + fileEntries[idx].dataSectOffset, fileEntries[idx].dataSectSize);

		free(data);
		return fileData;
	}

	char* GetFileGPUList(int32_t idx) {
		if (idx < 0 || idx >= entryCount) return nullptr;

		char* listSect = GetGPUListSection();
		if (listSect == nullptr) return nullptr;

		int32_t size = 0;

		memcpy(&size, listSect + fileEntries[idx].gpuListOffset, 4);

		char* fileData = (char*)malloc(size);
		memcpy(fileData, listSect + fileEntries[idx].gpuListOffset, size);

		free(listSect);
		return fileData;
	}

	char* GetFileGPU(int32_t idx, int32_t slot) {
		if (idx < 0 || idx >= entryCount) return nullptr;
		if (slot < 0 || slot >= gpuList[idx].entryCount) return nullptr;

		char* data = GetGpuSection();
		if (data == nullptr) return nullptr;

		char* fileData = (char*)malloc(gpuList[idx].offsets[slot].size);
		memcpy(fileData, data + gpuList[idx].offsets[slot].offset, gpuList[idx].offsets[slot].size);

		printf("%d %d %d\n", idx, gpuList[idx].offsets[slot].offset, gpuList[idx].offsets[slot].size);

		free(data);
		return fileData;
	}

	char* GetFileGPUForTexture(int32_t idx) {
		if (idx < 0 || idx >= entryCount) return nullptr;

		char* listSect = GetGPUListSection();
		if (listSect == nullptr) return nullptr;

		int32_t offset = 0;
		int32_t chunkSize = 0;
		memcpy(&offset, listSect + fileEntries[idx].gpuListOffset + 8, 4);
		memcpy(&chunkSize, listSect + fileEntries[idx].gpuListOffset + 0xC, 4);

		char* data = GetGpuSection();
		if (data == nullptr) return nullptr;

		char* fileData = (char*)malloc(chunkSize);
		memcpy(fileData, data + offset, chunkSize);

		printf("%d %d %d\n", idx, offset, chunkSize);

		free(listSect);
		free(data);
		return fileData;
	}
};

#endif