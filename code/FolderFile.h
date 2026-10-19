#pragma once
#ifndef _FOLDERFILEH
#define _FOLDERFILEH
#include <vector>
#include "Bundle.h"

struct FolderFile
{
    int fileCount = 0;
    char* currentDirectory; // Used for restoring the currentFilePath when you exit a file.
    CaffType currentType;

    std::vector<char*> fileNames;
    std::vector<CaffType> fileTypes;

    bool showInvalidFiles = false;
    
    void ClearActiveData()
    {
        fileCount = 0;
        fileNames.clear();
        fileTypes.clear();

        showInvalidFiles = false;
    };
};

#endif