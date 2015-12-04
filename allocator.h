// Copyright 2015 Aditya Mandaleeka

#include "logger.h"

#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_

namespace Alloc
{

struct AllocatorPageInfo
{
    // The address where the page begins
    void* address;

    // 64 bits representing the 64 chunks of the page
    uint64_t occupancy;

    AllocatorPageInfo *nextPageInfo;

    AllocatorPageInfo()
    {
        address = nullptr;
        occupancy = 0;
        nextPageInfo = nullptr;
    }
};

class AllocationManager
{
 public:
    AllocationManager(int pageSize, int chunkSize)
        : pageSize(pageSize)
        , chunkSize(chunkSize)
    {
        pageInfos = nullptr;

        chunksPerPage = pageSize / chunkSize;
        if ((pageSize % chunkSize) != 0)
        {
            Logger::Log(Logger::LOG_LEVEL_ERROR, "Page size must be a multiple of chunk size!");
        }
    }

    void* AllocMem(uint32_t numBytes);
    int FreeMem(void* addr, int numBytes);

 private:
    int GetNumberOfChunks(uint32_t numBytes);
    AllocatorPageInfo* AddNewPage();
    bool CheckPageForAvailability(AllocatorPageInfo* pageInfo, int numChunks, /* _Out_ */ int* firstChunk);
    void* ChangePageUsage(AllocatorPageInfo* pageInfo, int firstChunk, uint32_t numChunks, bool allocateOrFree);

 private:
    AllocatorPageInfo* pageInfos;
    int pageSize;
    int chunkSize;
    int chunksPerPage;
};

}  // namespace Alloc

#endif  // ALLOCATOR_H_
