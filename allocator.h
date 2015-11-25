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

    AllocatorPageInfo()
    {
        address = nullptr;
        occupancy = 0;
    }
};

class AllocationManager
{
 public:
    AllocationManager(int pageSize, int chunkSize, int maxPages)
        : pageSize(pageSize)
        , chunkSize(chunkSize)
        , maxPages(maxPages)
    {
        pageInfos = new AllocatorPageInfo[maxPages];
        numPagesUsed = 0;

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
    int AddNewPage();
    bool CheckPageForAvailability(AllocatorPageInfo* pageInfo, int numChunks, /* _Out_ */ int* firstChunk);
    void* ChangePageUsage(int pageNum, int firstChunk, uint32_t numChunks, bool allocateOrFree);

 private:
    AllocatorPageInfo* pageInfos;
    int numPagesUsed;
    int maxPages;
    int pageSize;
    int chunkSize;
    int chunksPerPage;
};

}  // namespace Alloc

#endif  // ALLOCATOR_H_
