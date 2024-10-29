#include "StaticBuffer.h"

#include <cstring>

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
                


// the declarations for this class can be found at "StaticBuffer.h"
unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];
// declare the blockAllocMap array
unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];

/*
TODO:: write block allocation map from block 0-3 to block allocation map*/
StaticBuffer::StaticBuffer() {
  // initialise all blocks as free
  for (int i = 0, blockMapslot = 0; i < 4; i++) {
    unsigned char buffer[BLOCK_SIZE];
    Disk::readBlock(buffer, i);
    for (int slot = 0; slot < BLOCK_SIZE; slot++, blockMapslot++) {
      StaticBuffer::blockAllocMap[blockMapslot] = buffer[slot];
    }
  }

  for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
    metainfo[bufferIndex].free = true;
    metainfo[bufferIndex].dirty = false;
    metainfo[bufferIndex].blockNum = -1;
    metainfo[bufferIndex].timeStamp = -1;
  }
}

/*
At this stage, we are not writing back from the buffer to the disk since we are
not modifying the buffer. So, we will define an empty destructor for now. In
subsequent stages, we will implement the write-back functionality here.
*/

/* 
TODO::writing back to block allocation map*/
StaticBuffer::~StaticBuffer() {
  for (int i = 0, blockMapslot = 0; i < 4; i++) {
    unsigned char buffer[BLOCK_SIZE];
    for (int slot = 0; slot < BLOCK_SIZE; slot++, blockMapslot++) {
      buffer[slot] = blockAllocMap[blockMapslot];
    }
    Disk::writeBlock(buffer, i);
  }

  for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
    if (metainfo[bufferIndex].free == false and
        metainfo[bufferIndex].dirty == true) {
      Disk::writeBlock(blocks[bufferIndex], metainfo[bufferIndex].blockNum);
    }
  }
}

int StaticBuffer::getFreeBuffer(int blockNum) {

    if (blockNum < 0 || blockNum > DISK_BLOCKS) {
        return E_OUTOFBOUND;
    }
    
    // Increment the time stamp for all occupied buffers
    for (int i = 0; i < BUFFER_CAPACITY; i++) {
        if (!metainfo[i].free)
            metainfo[i].timeStamp++; // Increment the time stamp for buffers that are in use
    }

    int allocatedBuffer = -1; // Initialize variable to store the index of the allocated buffer


    // iterate through all the blocks in the StaticBuffer
    for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
        // find the first free block in the buffer (check metainfo)
        if(metainfo[bufferIndex].free){
            // assign allocatedBuffer = index of the free block
            allocatedBuffer=bufferIndex;
            break;
        }
    }

    // If no free buffer was found, perform a replacement strategy
    if (allocatedBuffer == -1) {
        int highestTimeStamp = 0; // Variable to track the buffer with the highest time stamp
        for (int i = 0; i < BUFFER_CAPACITY; i++) {
            // Find the buffer that has been in use the longest (highest time stamp)
            if (metainfo[i].timeStamp > highestTimeStamp) {
                highestTimeStamp = metainfo[i].timeStamp;
                allocatedBuffer = i; // Set the index of the buffer to be replaced
            }
        }

        // If the selected buffer is dirty (modified), write it back to disk
        if (metainfo[allocatedBuffer].dirty)
            Disk::writeBlock(StaticBuffer::blocks[allocatedBuffer], metainfo[allocatedBuffer].blockNum);
    }

    // Update the metadata for the allocated buffer
    metainfo[allocatedBuffer].free = false;         // Mark the buffer as occupied
    metainfo[allocatedBuffer].dirty = false;        // Reset the dirty flag
    metainfo[allocatedBuffer].blockNum = blockNum;  // Set the block number for the buffer
    metainfo[allocatedBuffer].timeStamp = 0;        // Reset the time stamp for the buffer

    return allocatedBuffer; // Return the index of the allocated buffer
}

/* Get the buffer index where a particular block is stored
   or E_BLOCKNOTINBUFFER otherwise
*/
int StaticBuffer::getBufferNum(int blockNum) {

    // Check if blockNum is valid (between zero and DISK_BLOCKS)
    // and return E_OUTOFBOUND if not valid.
    if(blockNum < 0 || blockNum >= DISK_BLOCKS) {
        return E_OUTOFBOUND;
    }

    // find and return the bufferIndex which corresponds to blockNum (check metainfo)
    for(int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
        if(metainfo[bufferIndex].blockNum == blockNum){
            return bufferIndex;
        }
    }

    // if block is not in the buffer
    return E_BLOCKNOTINBUFFER;
}

int StaticBuffer::setDirtyBit(int blockNum){
    // find the buffer index corresponding to the block using getBufferNum().
    int bufferNum = getBufferNum(blockNum);

    // if block is not present in the buffer (bufferNum = E_BLOCKNOTINBUFFER)
    //     return E_BLOCKNOTINBUFFER
    if (bufferNum == E_BLOCKNOTINBUFFER) {
        return E_BLOCKNOTINBUFFER;
    }

    // if blockNum is out of bound (bufferNum = E_OUTOFBOUND)
    //     return E_OUTOFBOUND
    if (bufferNum == E_OUTOFBOUND) {
        return E_OUTOFBOUND;
    }

    // else
    //     (the bufferNum is valid)
    //     set the dirty bit of that buffer to true in metainfo
    metainfo[bufferNum].dirty = true;

    // return SUCCESS
    return SUCCESS;
}