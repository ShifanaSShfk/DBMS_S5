
//  Stage 3     : 
        //    ||  declaration happens after Disk(); ||
                //  StaticBuffer(),
                //  ~StaticBuffer(),
                //  getFreeBuffer(int blockNum),
                //  getBufferNum(int blockNum)


//  Stage 6     :   SB                              --  modified
                //  ~SB                             --  modified
                //  getFreeBuffer                   --  modified
                //  setDirtyBit(int blockNum)


//  Stage 7     :   SB                              --  modified
                //  ~SB                             --  modified
                

#include "StaticBuffer.h"

// #include <iostream>
#include <cstring>

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];
// declare the blockAllocMap array
unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];

StaticBuffer::StaticBuffer(){

    // copy blockAllocMap blocks from disk to buffer (using readblock() of disk)
    // blocks 0 to 3
    unsigned char buffPtr[BLOCK_SIZE];
    int blockAllocMapSlot = 0;
    for(int blockIdx=0;blockIdx<=3;blockIdx++){
        Disk::readBlock(buffPtr,blockIdx);
        for(int slot=0;slot<BLOCK_SIZE;slot++){

            blockAllocMap[blockAllocMapSlot++] = buffPtr[slot];
            
        }
    }

    //as this is the constructer we initialise all the buffer(cache) as unused
    for(int bufferBlockIdx = 0;bufferBlockIdx<BUFFER_CAPACITY;bufferBlockIdx++){
        metainfo[bufferBlockIdx].free = true;
        metainfo[bufferBlockIdx].dirty = false;
        metainfo[bufferBlockIdx].timeStamp = -1;
        metainfo[bufferBlockIdx].blockNum = -1;
    }
    
}
/*
    In stage 3 we wont be modifying the destructor so we just initialised the destructor
*/

// write back all modified blocks on system exit
StaticBuffer::~StaticBuffer(){

    // copy blockAllocMap blocks from buffer to disk(using writeblock() of disk)

    unsigned char buffPtr[BLOCK_SIZE];
    int blockAllocMapSlot = 0;
    for(int blockIdx=0;blockIdx<=3;blockIdx++){
        for(int slot=0;slot<BLOCK_SIZE;slot++){

            buffPtr[slot] = blockAllocMap[blockAllocMapSlot++];
            
        }
        Disk::writeBlock(buffPtr,blockIdx);
    }

    /*iterate through all the buffer blocks,
    write back blocks with metainfo as free=false,dirty=true
    using Disk::writeBlock()
    */
    for(int idx=0;idx<BUFFER_CAPACITY;idx++){
        if(!metainfo[idx].free && metainfo[idx].dirty){
            Disk::writeBlock(blocks[idx],metainfo[idx].blockNum);
        }
    }
}

int StaticBuffer::getFreeBuffer(int blockNum){

    // for checking if the blockNum overflows we return E_OUTOFBOUND
    if(blockNum < 0 || blockNum > DISK_BLOCKS){
        return E_OUTOFBOUND;
    }

    int bufferNum = -1;
    // increase the timeStamp in metaInfo of all occupied buffers.

    // let bufferNum be used to store the buffer number of the free/freed buffer.

    // here we check which bufferBlock is free by iterating through the metinfo of the buffer metinfo and gets the free block index
    for(int bufferBlockIdx = 0;bufferBlockIdx<BUFFER_CAPACITY;bufferBlockIdx++){
        
        if(metainfo[bufferBlockIdx].free && bufferNum == -1){
            bufferNum = bufferBlockIdx;
        }else if(!metainfo[bufferBlockIdx].free){
            metainfo[bufferBlockIdx].timeStamp++;
        }

    }

    if(bufferNum == -1){
        int maxTime = -1,idx;
        for(int bufferBlockIdx = 0;bufferBlockIdx<BUFFER_CAPACITY;bufferBlockIdx++){
            if(!metainfo[bufferNum].free && maxTime < metainfo[bufferBlockIdx].timeStamp){
                maxTime = metainfo[bufferBlockIdx].timeStamp;
                idx = bufferBlockIdx;
            }
        }
        bufferNum = idx;
        if(!metainfo[bufferNum].free && metainfo[bufferNum].dirty){
            Disk::writeBlock(blocks[bufferNum],metainfo[bufferNum].blockNum);
        }
    }

    // we make the block as taken on the metainfo 
    metainfo[bufferNum].free = false;
    metainfo[bufferNum].blockNum = blockNum;
    metainfo[bufferNum].dirty = false;
    metainfo[bufferNum].timeStamp = 0;

    return bufferNum;

}

int StaticBuffer::setDirtyBit(int blockNum){
    
    int bufferIdx = getBufferNum(blockNum);

    if(bufferIdx == E_BLOCKNOTINBUFFER){
        return E_BLOCKNOTINBUFFER;
    }
    
    if(bufferIdx == E_OUTOFBOUND){
        return E_OUTOFBOUND;
    }

    metainfo[bufferIdx].dirty = true;

    return SUCCESS;
}

int StaticBuffer::getBufferNum(int blockNum){

    if(blockNum < 0 || blockNum > DISK_BLOCKS){
        return E_OUTOFBOUND;
    }
    
    //this to find where we have stored the block in the cache
    for(int bufferBlockIdx = 0; bufferBlockIdx < BUFFER_CAPACITY; bufferBlockIdx++){
        if(!metainfo[bufferBlockIdx].free && metainfo[bufferBlockIdx].blockNum == blockNum){
            return bufferBlockIdx;
        }
    }

    return E_BLOCKNOTINBUFFER;
}