// Copyright 2015 Aditya Mandaleeka

#include "allocator.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

#include <climits>
#include <cstring>

namespace Alloc
{
    void* AllocationManager::AllocMem(uint32_t numBytes)
    {
        if (numBytes == 0)
        {
            // Should we allocate anything in this case?
            Logger::Log(Logger::LOG_LEVEL_ERROR, "AllocMem called with 0 for numBytes.");
            return nullptr;
        }

        int numChunks = GetNumberOfChunks(numBytes);
        if (numChunks > chunksPerPage)
        {
            Logger::Log(Logger::LOG_LEVEL_ERROR, "Cannot allocate more than %d chunks!", chunksPerPage);
            return nullptr;
        }

        int firstChunk = -1;
        AllocatorPageInfo *pageToAllocateOn = nullptr;

        for (AllocatorPageInfo *currPageInfo = pageInfos; currPageInfo != nullptr; currPageInfo = currPageInfo->nextPageInfo)
        {
            if (CheckPageForAvailability(currPageInfo, numChunks, &firstChunk))
            {
                pageToAllocateOn = currPageInfo;
                break;
            }
        }

        if (pageToAllocateOn == nullptr)
        {
            // No page had availability, so create a new page and use that.
            pageToAllocateOn = AddNewPage();
            if (pageToAllocateOn == nullptr)
            {
                Logger::Log(Logger::LOG_LEVEL_ERROR, "Call to AddNewPage failed!\n");
                return nullptr;
            }

            if (!CheckPageForAvailability(pageToAllocateOn, numChunks, &firstChunk))
            {
                Logger::Log(Logger::LOG_LEVEL_ERROR, "No availability on new page!");
            }
        }

        return ChangePageUsage(pageToAllocateOn, firstChunk, numChunks, true /* allocate */);
    }

    int AllocationManager::FreeMem(void* addr, int numBytes)
    {
        // This assumes page is aligned to 4096-byte boundary
        void* startOfPage = (void*) ((uint64_t)addr & ~0xFFF);

        AllocatorPageInfo *pageToFreeIn = nullptr;
        for (AllocatorPageInfo *currPageInfo = pageInfos; currPageInfo != nullptr; currPageInfo = currPageInfo->nextPageInfo)
        {
            if (currPageInfo->address == startOfPage)
            {
                pageToFreeIn = currPageInfo;
            }
        }

        if (pageToFreeIn == nullptr)
        {
            Logger::Log(Logger::LOG_LEVEL_ERROR, "Couldn't locate page in which to free!");
            return -1;
        }

        int numChunks = GetNumberOfChunks(numBytes);

        int positionOfFirstChunk = ((char*)addr - (char*)startOfPage) / chunkSize;

        ChangePageUsage(pageToFreeIn, positionOfFirstChunk, numChunks, false /* indicates free */);

        // Want to write a pattern over this memory if in debug?

        return 0;
    }

    int AllocationManager::GetNumberOfChunks(uint32_t numBytes)
    {
        int numChunks = (numBytes % chunkSize) == 0 ? numBytes / chunkSize : numBytes / chunkSize + 1;
        Logger::Log(Logger::LOG_LEVEL_INFO, "%d chunks for %lu bytes", numChunks, numBytes);

        return numChunks;
    }

    AllocatorPageInfo* AllocationManager::AddNewPage()
    {
        // TODO: add c-sec or something if needed

        Logger::Log(Logger::LOG_LEVEL_INFO, "Allocating new page.");

        void* newPage = mmap(
            nullptr,
            pageSize,
            PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_PRIVATE | MAP_ANONYMOUS,
            0,
            0);

        if ((uint64_t)newPage & 0xFFF)
        {
            Logger::Log(Logger::LOG_LEVEL_ERROR, "mmap allocated page at %p! errno is %x.", newPage, errno);
        }

        AllocatorPageInfo *newPageInfo = new AllocatorPageInfo();
        newPageInfo->address = newPage;
        newPageInfo->nextPageInfo = pageInfos;

        pageInfos = newPageInfo;

        return newPageInfo;
    }

    bool AllocationManager::CheckPageForAvailability(AllocatorPageInfo* pageInfo, int numChunks, /* _Out_ */ int* firstChunk)
    {
        uint64_t occupancy = pageInfo->occupancy;

        // Check for numChunks consecutive 0's in occupancy.
        int firstChunkToUse = -1;
        for (int i = 0; i <= (chunksPerPage - numChunks); i++)
        {
            uint64_t mask = ULLONG_MAX >> (64 - numChunks);
            if (((mask << i) & occupancy) == 0)
            {
                firstChunkToUse = chunksPerPage - numChunks - i;
                Logger::Log(Logger::LOG_LEVEL_INFO, "First chunk to use is %d", firstChunkToUse);
                break;
            }
        }

        if (firstChunk != nullptr)
        {
            *firstChunk = firstChunkToUse;

            if (firstChunkToUse != -1)
            {
                return true;
            }
        }

        return false;
    }

    void* AllocationManager::ChangePageUsage(AllocatorPageInfo* pageInfo, int firstChunk, uint32_t numChunks, bool allocateOrFree)
    {
        Logger::Log(
            Logger::LOG_LEVEL_INFO,
            "ChangePageUsage called. Page is %p, firstChunk is %d, numChunks is %d. %s",
            pageInfo, firstChunk, numChunks, allocateOrFree ? "allocating." : "freeing.");

        uint64_t mask = ULLONG_MAX >> (64 - numChunks);
        mask = mask << (chunksPerPage - firstChunk - numChunks);

        Logger::LogAsBitString(Logger::LOG_LEVEL_INFO, "Occupancy before operation is: ", pageInfo->occupancy);


        uint64_t prevOccupancy = pageInfo->occupancy;
        pageInfo->occupancy = allocateOrFree ? (prevOccupancy | mask) : (prevOccupancy ^ mask);

        Logger::LogAsBitString(Logger::LOG_LEVEL_INFO, "Occupancy after operation is:  ", pageInfo->occupancy);

        void* addrOfAlloc = (char*)pageInfo->address + firstChunk * chunkSize;
        return addrOfAlloc;
    }
}  // namespace Alloc
