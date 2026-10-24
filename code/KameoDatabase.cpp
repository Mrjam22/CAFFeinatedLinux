
#include "CommonReader.h"
#include "KameoDatabase.h"

void KameoDBFile::ReadDatabaseFile(char* data, char byteswap) {
	if (data == nullptr) {
		return;
	}

	dbData = data;

	header.ReadDatabaseHeader(data, byteswap);

	fileTable.ReadDatabaseFileTable(data, header.fileTableOffset, byteswap);
}

void KameoDBHeader::ReadDatabaseHeader(char* data, char byteswap) {
	memcpy(headerString, data, 0x20);

	int fileTableOffsetVar = 0;
	int unk1Var = 0;

	if (byteswap == 1) {
		memcpy(&fileTableOffsetVar, data + 0x20, 4);
		memcpy(&unk1Var, data + 0x24, 4);

		fileTableOffset = flipEndian(fileTableOffsetVar);
		unk1 = flipEndian(unk1Var);
	}

	if (byteswap == 0) {
		memcpy(&fileTableOffset, data + 0x20, 4);
		memcpy(&unk1, data + 0x24, 4);
	}
}

void KameoDBFileTable::ReadDatabaseFileTable(char* data, int offset, char byteswap) {

	int entryCountVar = 0;
	int offs1Var = 0;
	int offs2Var = 0;
	int offs3Var = 0;

	if (byteswap == 1) {
		memcpy(&entryCountVar, data + offset, 4);
		memcpy(&offs1Var, data + offset + 0x4, 4);
		memcpy(&offs2Var, data + offset + 0x8, 4);
		memcpy(&offs3Var, data + offset + 0xC, 4);

		header.entryCount = flipEndian(entryCountVar);
		header.nameTableOffs = flipEndian(offs1Var);
		header.indexTableOffs = flipEndian(offs2Var);
		header.hashTableOffs = flipEndian(offs3Var);
	}

	if (byteswap == 0) {
		memcpy(&header.entryCount, data, 4);
		memcpy(&header.nameTableOffs, data + 0x4, 4);
		memcpy(&header.indexTableOffs, data + 0x8, 4);
		memcpy(&header.hashTableOffs, data + 0xC, 4);
	}

	nameEntries = new KameoDBFileTableNameEntry[header.entryCount]();
	indexEntries = new KameoDBFileTableIndexEntry[header.entryCount]();
	hashEntries = new KameoDBFileTableHashEntry[header.entryCount]();

	int tmpVal1 = 0;
	int tmpVal2 = 0;
	int tmpVal3 = 0;

	// Name Entries
	int position = header.nameTableOffs;
	for (int i = 0; i < header.entryCount; i++) {
		if (byteswap == 1) {
			memcpy(&tmpVal1, data + position, 4);
			memcpy(&tmpVal2, data + position + 0x4, 4);
			memcpy(&tmpVal3, data + position + 0x8, 4);

			nameEntries[i].nameOffset = flipEndian(tmpVal1);
			nameEntries[i].unk1 = flipEndian(tmpVal2);
			nameEntries[i].unk2 = flipEndian(tmpVal3);
		}

		if (byteswap == 0) {
			memcpy(&nameEntries[i].nameOffset, data + position, 4);
			memcpy(&nameEntries[i].unk1, data + position + 0x4, 4);
			memcpy(&nameEntries[i].unk2, data + position + 0x8, 4);
		}

		printf("Entry %d - [%d %d %d]\n", i, nameEntries[i].nameOffset, nameEntries[i].unk1, nameEntries[i].unk2);
		position += 0xC;
	}

	// Index Table
	position = header.indexTableOffs;
	for (int i = 0; i < header.entryCount; i++) {
		if (byteswap == 1) {
			memcpy(&tmpVal1, data + position, 4);
			memcpy(&tmpVal2, data + position + 0x4, 4);
			memcpy(&tmpVal3, data + position + 0x8, 4);

			indexEntries[i].unk1 = flipEndian(tmpVal1);
			indexEntries[i].index = flipEndian(tmpVal2);
			indexEntries[i].nameTableOffset = flipEndian(tmpVal3);
		}

		if (byteswap == 0) {
			memcpy(&indexEntries[i].unk1, data + position, 4);
			memcpy(&indexEntries[i].index, data + position + 0x4, 4);
			memcpy(&indexEntries[i].nameTableOffset, data + position + 0x8, 4);
		}

		printf("Entry %d - [%d %d %d]\n", i, indexEntries[i].unk1, indexEntries[i].index, indexEntries[i].nameTableOffset);
		position += 0xC;
	}

	// Hash Entries
	position = header.hashTableOffs;
	for (int i = 0; i < header.entryCount; i++) {
		if (byteswap == 1) {
			memcpy(&tmpVal1, data + position, 4);
			memcpy(&tmpVal2, data + position + 0x4, 4);
			memcpy(&tmpVal3, data + position + 0x8, 4);

			hashEntries[i].hash = flipEndian(tmpVal1);
			hashEntries[i].unk1 = flipEndian(tmpVal2);
			hashEntries[i].nameTableOffset = flipEndian(tmpVal3);
		}

		if (byteswap == 0) {
			memcpy(&hashEntries[i].hash, data + position, 4);
			memcpy(&hashEntries[i].unk1, data + position + 0x4, 4);
			memcpy(&hashEntries[i].nameTableOffset, data + position + 0x8, 4);
		}

		printf("Entry %d - [%08X %d %d]\n", i, hashEntries[i].hash, hashEntries[i].unk1, hashEntries[i].nameTableOffset);
		position += 0xC;
	}

	fileNames = new char* [header.entryCount]();

	for (int i = 0; i < header.entryCount; i++) {
		fileNames[i] = new char[128];
		strcpy(fileNames[i], data + nameEntries[i].nameOffset);

		printf("Entry %d - [%s]\n", i, fileNames[i]);
	}
}