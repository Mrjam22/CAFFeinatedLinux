#include "CommonReader.h"
#include "GhoulBundle.h"
#include "LoadingProcess.h"
#include "MessageHandle.h"

FileEntry::~FileEntry() {
	memset(fileName, 0, FILEENTRY_NAMESIZE);
}

bool GhoulBundle::ReadBundleFile(char* data, size_t dataSize) {
	isReady = false;

	if (data == nullptr) return false;
	dataPtr = data;

	try {
		memcpy_s(&entryCount, sizeof(int16_t), data, sizeof(int16_t));
		memcpy_s(&isCompressed, sizeof(int16_t), data + 2, sizeof(int16_t));
		memcpy_s(&iUnk1, sizeof(int32_t), data + 4, sizeof(int32_t));

		memcpy_s(&fileListOffset, sizeof(int32_t), data + 0x8, sizeof(int32_t));
		memcpy_s(&fileListDataSize, sizeof(int32_t), data + 0xC, sizeof(int32_t));
		memcpy_s(&gpuListOffset, sizeof(int32_t), data + 0x10, sizeof(int32_t));
		memcpy_s(&gpuListSize, sizeof(int32_t), data + 0x14, sizeof(int32_t));

		memcpy_s(&gpuSectOffset, sizeof(int32_t), data + 0x18, sizeof(int32_t));
		memcpy_s(&gpuSectSize, sizeof(int32_t), data + 0x1C, sizeof(int32_t));
		memcpy_s(&dataSectOffset, sizeof(int32_t), data + 0x20, sizeof(int32_t));
		memcpy_s(&dataSectSize, sizeof(int32_t), data + 0x24, sizeof(int32_t));

		int32_t totalSize = 0x28 + fileListDataSize + gpuListSize + gpuSectSize + dataSectSize;

		if (entryCount == 0 || iUnk1 != 0) return false;

		char* uncompedBaseData = (char*)malloc(totalSize);

		int32_t size = dataSize - 0x28;

		if (isCompressed == 1) {
			memcpy_s(uncompedBaseData, 0x28, data, 0x28);

			// Do the section table first
			char* in = (char*)malloc(size);

			memcpy_s(in, size, data + fileListOffset, size);

			char* out = InflateData(in, 9, size, fileListDataSize + gpuListSize + gpuSectSize + dataSectSize);

			memcpy_s(uncompedBaseData + 0x28, fileListDataSize + gpuListSize + gpuSectSize + dataSectSize, out, fileListDataSize + gpuListSize + gpuSectSize + dataSectSize);

			free(out);
		}
		else {
			memcpy_s(uncompedBaseData, totalSize, data, totalSize);
		}

		dataPtr = uncompedBaseData;


		for (int32_t i = 0; i < entryCount; i++) {
			fileEntries.emplace_back();
			dirtyFiles.emplace_back();
			strncpy(fileEntries[i].fileName, dataPtr + fileListOffset + (sizeof(FileEntry) * i), FILEENTRY_NAMESIZE);

			memcpy(&fileEntries[i].type, dataPtr + fileListOffset + (sizeof(FileEntry) * i) + FILEENTRY_NAMESIZE, sizeof(int32_t));
			memcpy(&fileEntries[i].timestamp, dataPtr + fileListOffset + (sizeof(FileEntry) * i) + FILEENTRY_NAMESIZE + 4, sizeof(int32_t));
			memcpy(&fileEntries[i].fUnk1, dataPtr + fileListOffset + (sizeof(FileEntry) * i) + FILEENTRY_NAMESIZE + 8, sizeof(int32_t));
			memcpy(&fileEntries[i].iUnk1, dataPtr + fileListOffset + (sizeof(FileEntry) * i) + FILEENTRY_NAMESIZE + 0xC, sizeof(int32_t));

			memcpy(&fileEntries[i].dataSectOffset, dataPtr + fileListOffset + (sizeof(FileEntry) * i) + FILEENTRY_NAMESIZE + 0x10, sizeof(int32_t));
			memcpy(&fileEntries[i].dataSectSize, dataPtr + fileListOffset + (sizeof(FileEntry) * i) + FILEENTRY_NAMESIZE + 0x14, sizeof(int32_t));
			memcpy(&fileEntries[i].gpuListOffset, dataPtr + fileListOffset + (sizeof(FileEntry) * i) + FILEENTRY_NAMESIZE + 0x18, sizeof(int32_t));
			memcpy(&fileEntries[i].gpuSectAndListSize, dataPtr + fileListOffset + (sizeof(FileEntry) * i) + FILEENTRY_NAMESIZE + 0x1C, sizeof(int32_t));

			gpuList.emplace_back();

			// These have GPU sections.
			if (fileEntries[i].type == 0x4 || fileEntries[i].type == 0x1) {
				memcpy(&gpuList[i].tableSize, dataPtr + gpuListOffset + fileEntries[i].gpuListOffset, sizeof(int32_t));
				memcpy(&gpuList[i].entryCount, dataPtr + gpuListOffset + fileEntries[i].gpuListOffset + 4, sizeof(int32_t));
				gpuList[i].offsets = new OffsetSet[gpuList[i].entryCount];
				dirtyFiles[i].gpuSects = new DirtyGpuSect[gpuList[i].entryCount];
				for (int32_t k = 0; k < gpuList[i].entryCount; k++) {
					memcpy(&gpuList[i].offsets[k].offset, dataPtr + gpuListOffset + fileEntries[i].gpuListOffset + 8 + (8 * k), sizeof(int32_t));
					memcpy(&gpuList[i].offsets[k].size, dataPtr + gpuListOffset + fileEntries[i].gpuListOffset + 12 + (8 * k), sizeof(int32_t));
				}
			}
		}

		isReady = true;
	}
	catch (...) {
		printf("An error occured while reading a Ghoulies bundle file.\n");
		return false;
	}

	return true;
}

bool GhoulBundle::readStandaloneBundleFile(char* fileName) {
	FILE* currentFile = fopen(fileName, "rb");

	if (ferror(currentFile) != 0) {
		printf("Error occured while trying to open the file.\n");
		return false;
	}

	fseek(currentFile, 0L, SEEK_END);
	int32_t length = ftell(currentFile);
	fseek(currentFile, 0L, SEEK_SET);

	char* data = (char*)malloc(length);

	fread(data, sizeof(char), length, currentFile);

	fclose(currentFile);

	return ReadBundleFile(data, length);
}

void GhoulBundle::ClearBundleFileData() {
	isReady = false;

	fileEntries.clear();
	gpuList.clear();
	dirtyFiles.clear();

	if (dataPtr != nullptr) {
		free(dataPtr);
		dataPtr = nullptr;
	}
}

void GhoulBundle::writeToFile(const char* fileName) {
	if (dataPtr == nullptr) {
		printf("No data pointer is present. How can I write anything with no data? (This usually implies the file hasn't been read first)");
		return;
	}

	char emptyBuf[128] = { 0 };
	memset(emptyBuf, 0, 128);

	FILE* writeStrm = fopen(fileName, "wb");

	try {
		size_t gpuSize = 0;
		size_t dataSize = 0;
		size_t fileListSize = 0xA0 * entryCount;
		size_t newGpuListSize = 0;
		for (int i = 0; i < entryCount; i++) {
			dataSize += fileEntries[i].dataSectSize;
			newGpuListSize += gpuList[i].tableSize;
			for (int k = 0; k < gpuList[i].entryCount; k++) {
				if (dirtyFiles[i].gpuSects[k].gpuSize != 0) {
					gpuSize += dirtyFiles[i].gpuSects[k].gpuSize;
				}
				else {
					gpuSize += gpuList[i].offsets[k].size;
				}
			}
		}

		int32_t fileListOffs = 0x28; // More or less a constant. File List will always be after the header.
		int32_t gpuListOffs = fileListOffs + fileListSize;
		int32_t gpuOffs = gpuListOffs + newGpuListSize;
		int32_t dataOffs = gpuOffs + gpuSize;

		size_t mainDataSize = fileListSize + newGpuListSize + gpuSize + dataSize;

		char* mainData = (char*)malloc(mainDataSize);

		fwrite(&entryCount, sizeof(int16_t), 1, writeStrm);
		fwrite(&isCompressed, sizeof(int16_t), 1, writeStrm);
		fwrite(&iUnk1, sizeof(int32_t), 1, writeStrm);

		fwrite(&fileListOffs, sizeof(int32_t), 1, writeStrm);
		fwrite(&fileListSize, sizeof(int32_t), 1, writeStrm);
		fwrite(&gpuListOffs, sizeof(int32_t), 1, writeStrm);
		fwrite(&newGpuListSize, sizeof(int32_t), 1, writeStrm);

		fwrite(&gpuOffs, sizeof(int32_t), 1, writeStrm);
		fwrite(&gpuSize, sizeof(int32_t), 1, writeStrm);
		fwrite(&dataOffs, sizeof(int32_t), 1, writeStrm);
		fwrite(&dataSize, sizeof(int32_t), 1, writeStrm);

		int32_t fileGPUOffs = 0;
		int32_t fileDataOffs = 0;
		for (int i = 0; i < entryCount; i++) {
			memcpy(mainData + (sizeof(FileEntry) * i), fileEntries[i].fileName, 0x80);
			memcpy(mainData + (sizeof(FileEntry) * i) + 0x80, &fileEntries[i].type, sizeof(int32_t));
			memcpy(mainData + (sizeof(FileEntry) * i) + 0x84, &fileEntries[i].timestamp, sizeof(int32_t));
			memcpy(mainData + (sizeof(FileEntry) * i) + 0x88, &fileEntries[i].fUnk1, sizeof(int32_t));
			memcpy(mainData + (sizeof(FileEntry) * i) + 0x8C, &fileEntries[i].iUnk1, sizeof(int32_t));

			memcpy(mainData + (sizeof(FileEntry) * i) + 0x90, &fileDataOffs, sizeof(int32_t));

			if (dirtyFiles[i].dataSize != 0) {
				memcpy(mainData + (sizeof(FileEntry) * i) + 0x94, &dirtyFiles[i].dataSize, sizeof(int32_t));
				fileDataOffs += dirtyFiles[i].dataSize;
			}
			else {
				memcpy(mainData + (sizeof(FileEntry) * i) + 0x94, &fileEntries[i].dataSectSize, sizeof(int32_t));
				fileDataOffs += fileEntries[i].dataSectSize;
			}

			if (gpuList[i].entryCount == 0) {
				int32_t blank = 0;
				memcpy(mainData + (sizeof(FileEntry) * i) + 0x98, &blank, sizeof(int32_t));
			}
			else {
				memcpy(mainData + (sizeof(FileEntry) * i) + 0x98, &fileGPUOffs, sizeof(int32_t));
			}

			int32_t totalSize = gpuList[i].tableSize;
			for (int k = 0; k < gpuList[i].entryCount; k++) {
				if (dirtyFiles[i].gpuSects[k].gpuSize != 0) {
					totalSize += dirtyFiles[i].gpuSects[k].gpuSize;
				}
				else {
					totalSize += gpuList[i].offsets[k].size;
				}
			}
			memcpy(mainData + (sizeof(FileEntry) * i) + 0x9C, &totalSize, sizeof(int32_t));
			fileGPUOffs += gpuList[i].tableSize;
		}

		int32_t currentGpuOffs = 0;
		int32_t currentGpuListOffs = gpuListOffs - 0x28;
		for (int i = 0; i < entryCount; i++) {
			if (gpuList[i].entryCount == 0) continue;
			memcpy(mainData + currentGpuListOffs, &gpuList[i].tableSize, sizeof(int32_t));
			memcpy(mainData + currentGpuListOffs + 4, &gpuList[i].entryCount, sizeof(int32_t));

			for (int k = 0; k < gpuList[i].entryCount; k++) {
				memcpy(mainData + currentGpuListOffs + 8 + (8 * k), &currentGpuOffs, sizeof(int32_t));
				if (dirtyFiles[i].gpuSects[k].gpuSize != 0) {
					memcpy(mainData + currentGpuListOffs + 12 + (8 * k), &dirtyFiles[i].gpuSects[k].gpuSize, sizeof(int32_t));
					currentGpuOffs += dirtyFiles[i].gpuSects[k].gpuSize;
				}
				else {
					memcpy(mainData + currentGpuListOffs + 12 + (8 * k), &gpuList[i].offsets[k].size, sizeof(int32_t));
					currentGpuOffs += gpuList[i].offsets[k].size;
				}
			}

			currentGpuListOffs += gpuList[i].tableSize;
		}

		gpuOffs -= 0x28;
		dataOffs -= 0x28;

		currentGpuOffs = gpuOffs;
		int32_t currentDataOffs = dataOffs;

		for (int i = 0; i < entryCount; i++) {
			// Data
			if (dirtyFiles[i].dataSize != 0) {
				memcpy(mainData + currentDataOffs, dirtyFiles[i].newDataSect, dirtyFiles[i].dataSize);
				currentDataOffs += dirtyFiles[i].dataSize;
			}
			else {
				char* dataBuf = GetFileData(i);
				memcpy(mainData + currentDataOffs, dataBuf, fileEntries[i].dataSectSize);
				currentDataOffs += fileEntries[i].dataSectSize;
				free(dataBuf);
			}

			// GPU
			for (int k = 0; k < gpuList[i].entryCount; k++) {
				if (dirtyFiles[i].gpuSects[k].gpuSize != 0) {
					memcpy(mainData + currentGpuOffs, dirtyFiles[i].gpuSects[k].newData, dirtyFiles[i].gpuSects[k].gpuSize);
					currentGpuOffs += dirtyFiles[i].gpuSects[k].gpuSize;
				}
				else {
					char* dataBuf = GetFileGPU(i, k);
					memcpy(mainData + currentGpuOffs, dataBuf, gpuList[i].offsets[k].size);
					currentGpuOffs += gpuList[i].offsets[k].size;
					free(dataBuf);
				}
			}

			IncreaseCurrentSavedOnLoadingWidget();
		}

		if (isCompressed) {
			uint32_t compedSize = compressBound(mainDataSize);
			unsigned long dstSize = 0;

			unsigned char* compedBuffer = (unsigned char*)malloc(compedSize);
			int ret = compress(compedBuffer, &dstSize, (unsigned char*)mainData, mainDataSize);

			printf("%d\n", ret);
			if (ret == Z_OK) {
				fwrite(compedBuffer, dstSize, 1, writeStrm);
			}

			free(compedBuffer);
		}
		else {
			fwrite(mainData, mainDataSize, 1, writeStrm);
		}

		free(mainData);
	}
	catch (std::exception e) {
		char msgBug[1024] = { 0 };
		sprintf(msgBug, "An error occured while trying to write the file. Error Message %s\n", e.what());
		FireMessage(msgBug, ErrorType_Error);
	}

	fflush(writeStrm);
	fclose(writeStrm);
}