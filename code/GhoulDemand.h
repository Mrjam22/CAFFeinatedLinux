#pragma once
#ifndef _GHOULDEMAND
#define _GHOULDEMAND

struct DemandGpuListEntry {
	int32_t offset;
	int32_t size;
};

struct DemandGpuListTable {
	int32_t sizeOfTable;
	int32_t entryCount;

	DemandGpuListEntry* entries;
};

struct GhoulDemand {
	char* dataPtr;

	bool isReady;
	bool hasErrored;

	int32_t type;
	uint32_t timestamp;
	float fUnk1;
	int32_t iUnk1;

	// Actual Section Info
	int32_t dataSectOffset;
	int32_t dataSectSize;
	int32_t gpuSectOffset;
	int32_t gpuSectSize;

	DemandGpuListTable gpuList;

	void ReadDemandFile(char* data);
	bool readStandaloneDemandFile(char* fileName);

	void ClearActiveData();

	char* GetDataSection() {
		if (dataPtr == nullptr) return nullptr;

		char* data = (char*)malloc(dataSectSize);
		memcpy(data, dataPtr + dataSectOffset, dataSectSize);

		return data;
	}

	char* GetWholeGpuSection() {
		if (dataPtr == nullptr) return nullptr;
		if (gpuSectSize == 0) return nullptr;

		char* data = (char*)malloc(gpuSectSize);
		memcpy(data, dataPtr + gpuSectOffset, gpuSectSize);

		return data;
	}

	char* GetGpuSection(int32_t slot) {
		if (dataPtr == nullptr) return nullptr;
		if (gpuSectSize == 0) return nullptr;

		char* data = (char*)malloc(gpuList.entries[slot].size);
		memcpy(data, dataPtr + gpuSectOffset + gpuList.entries[slot].offset, gpuList.entries[slot].size);

		return data;
	}
};

#endif