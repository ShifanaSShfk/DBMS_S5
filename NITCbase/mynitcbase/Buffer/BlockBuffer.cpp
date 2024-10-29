#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>

#include <iostream>

//  Stage 2     :   BB(blockNum)
                //  RB(int blockNum), 
                //  BB::getHeader(struct HeadInfo *head), 
                //  RB::getRecord(union Attribute *rec, int slotNum)

//  Stage 3     :
        //  || read from the BUFFER instead of the disk directly ||
        //  || uses loadBlockAndGetBufferPtr() method instead of Disk::readBlock() ||
        
                //  BB::getHeader       -- Modified
                //  RB::getRecord       -- Modified

                //  RB::setRecord(union Attribute *rec, int slotNum), *************DK!!!*************FROM WHERE THIS CAME
                //  BB::loadBlockAndGetBufferPtr(unsigned char **buffPtr)

//  Stage 4     :   RB::getSlotMap(unsigned char* slotMap)

//  Stage 6     :   loadBlockAndGetBufferPtr    -- modified
                //  setRecord                   -- modified

//  Stage 7     :   BB::setHeader(struct HeadInfo* head)
                //  BB::setBlockType(int blockType)
                //  BB::getFreeBlock(int blockType)
                //  BB (char blockType)
                //  RB ()
                //  RB::setSlotMap(unsigned char* slotMap)
                //  BB::getBlockNum()


#include <stdio.h>

int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {
  return attrType == NUMBER
             ? (attr1.nVal < attr2.nVal ? -1
                                        : (attr1.nVal > attr2.nVal ? 1 : 0))
             : strcmp(attr1.sVal, attr2.sVal);
}

BlockBuffer::BlockBuffer(int blockNum) {
    this->blockNum = blockNum;
}

BlockBuffer::BlockBuffer(char blocktype){
    // allocate a block on the disk and a buffer in memory to hold the new block of
    // given type using getFreeBlock function and get the return error codes if any.
	// * NOTE: this line should be changed later
	int blockType = blocktype == 'R' ? REC : UNUSED_BLK; 

	int blockNum = getFreeBlock(blockType);
	if (blockNum < 0 || blockNum >= DISK_BLOCKS) {
		std::cout << "Error: Block is not available\n";
		this->blockNum = blockNum;
		return;
	}

	// // int bufferIndex = StaticBuffer::getFreeBuffer(blockNum);
	// // if (bufferIndex < 0 || bufferIndex >= BUFFER_CAPACITY) {
	// // 	std::cout << "Error: Buffer is not available\n";
	// // 	return;
	// // }
		
    // set the blockNum field of the object to that of the allocated block
    // number if the method returned a valid block number,
    // otherwise set the error code returned as the block number.

	this->blockNum = blockNum;

    // (The caller must check if the constructor allocatted block successfully
    // by checking the value of block number field.)
}


RecBuffer::RecBuffer() : BlockBuffer('R'){}
// call parent non-default constructor with 'R' denoting record block.

RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}


/*
Used to get the header of the block into the location pointed to by `head`
NOTE: this function expects the caller to allocate memory for `head`
*/
int BlockBuffer::getHeader(struct HeadInfo* head) {
    
  unsigned char buffer[BLOCK_SIZE];
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);

  if (ret != SUCCESS) {
    return ret;   
    // return any errors that might have occured in the process
  }


  // read the block at this.blockNum into the buffer
  Disk::readBlock(buffer, this->blockNum);

  // populate the numEntries, numAttrs and numSlots fields in *head
  memcpy(&head->numSlots, buffer + 24, 4);
  memcpy(&head->numEntries, buffer + 16, 4);
  memcpy(&head->numAttrs, buffer + 20, 4);
  memcpy(&head->rblock, buffer + 12, 4);
  memcpy(&head->lblock, buffer + 8, 4);

  return SUCCESS;
}


int RecBuffer::getRecord(union Attribute* rec, int slotNum) {
  
  struct HeadInfo head;
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);

  if (ret != SUCCESS) {
    return ret;
  }

  // get the header using this.getHeader() function
  this->getHeader(&head);

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  unsigned char buffer[BLOCK_SIZE];

  // read the block at this.blockNum into a buffer
  Disk::readBlock(buffer, this->blockNum);

  /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
     - each record will have size attrCount * ATTR_SIZE
     - slotMap will be of size slotCount
  */
  int recordSize = attrCount * ATTR_SIZE;
  unsigned char *slotPointer = buffer + HEADER_SIZE + slotCount + (recordSize * slotNum);

  // load the record into the rec data structure
  memcpy(rec, slotPointer, recordSize);
  
  return SUCCESS;
}

int RecBuffer::setRecord(union Attribute* rec, int slotNum) {
    unsigned char* bufferPtr;

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    if (ret != SUCCESS)
        return ret;

    HeadInfo header;
    this->getHeader(&header);

    int numAttrs = header.numAttrs;
    int numSlots = header.numSlots;

    if (slotNum < 0 || slotNum >= numSlots)
        return E_OUTOFBOUND;
    
    int recordSize = numAttrs*ATTR_SIZE;
    unsigned char* recordPtr = bufferPtr + HEADER_SIZE + numSlots + slotNum*recordSize;

    memcpy(recordPtr, rec, recordSize);
    StaticBuffer::setDirtyBit(this->blockNum);

    return SUCCESS;
}

/*
Used to load a block to the buffer and get a pointer to it.
NOTE: this function expects the caller to allocate memory for the argument
*/
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {
  // check whether the block is already present in the buffer using StaticBuffer.getBufferNum()
  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

  if (bufferNum == E_BLOCKNOTINBUFFER) {
    bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

    if (bufferNum == E_OUTOFBOUND) {
      return E_OUTOFBOUND;
    }

    Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
  }

  else {
        for (int i = 0; i < BUFFER_CAPACITY; i++) {
            if (!StaticBuffer::metainfo[i].free)
                StaticBuffer::metainfo[i].timeStamp++;
        }

        StaticBuffer::metainfo[bufferNum].timeStamp = 0;
    }
    

    // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr

    // return SUCCESS;

    *buffPtr = StaticBuffer::blocks[bufferNum];

    return SUCCESS;
}

int RecBuffer::getSlotMap(unsigned char* slotMap) {
    unsigned char* bufferPtr;

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    if (ret != SUCCESS)
        return ret;

    struct HeadInfo head;
    getHeader(&head);

    int slotCount = head.numSlots;

    unsigned char* slotMapInBuffer = bufferPtr + HEADER_SIZE;

    memcpy(slotMap, slotMapInBuffer, slotCount);

    return SUCCESS;

}

int BlockBuffer::setHeader(struct HeadInfo *head) {

  unsigned char *bufferPtr;
  int bufferreturn = loadBlockAndGetBufferPtr(&bufferPtr);
  if (bufferreturn != SUCCESS) {
    return bufferreturn;
  }
  // get the starting address of the buffer containing the block using
  // loadBlockAndGetBufferPtr(&bufferPtr).

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  // return the value returned by the call.

  // cast bufferPtr to type HeadInfo*
  struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;

  // copy the fields of the HeadInfo pointed to by head (except reserved) to
  // the header of the block (pointed to by bufferHeader)
  //(hint: bufferHeader->numSlots = head->numSlots )
  bufferHeader->numSlots = head->numSlots;
  bufferHeader->lblock = head->lblock;
  bufferHeader->numEntries = head->numEntries;
  bufferHeader->pblock = head->pblock;
  bufferHeader->rblock = head->rblock;
  bufferHeader->blockType = head->blockType;
  bufferHeader->numAttrs=head->numAttrs;

  // update dirty bit by calling StaticBuffer::setDirtyBit()
  // if setDirtyBit() failed, return the error code
  int setDirty = StaticBuffer::setDirtyBit(this->blockNum);
  return setDirty;

  // return SUCCESS;
}

int BlockBuffer::setBlockType(int blockType) {

  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return SUCCESS;
  }
  /* get the starting address of the buffer containing the block
     using loadBlockAndGetBufferPtr(&bufferPtr). */

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  // return the value returned by the call.

  // store the input block type in the first 4 bytes of the buffer.
  // (hint: cast bufferPtr to int32_t* and then assign it)
  *((int32_t *)bufferPtr) = blockType;

  // update the StaticBuffer::blockAllocMap entry corresponding to the
  // object's block number to `blockType`.
  StaticBuffer::blockAllocMap[this->blockNum] = blockType;
  // update dirty bit by calling StaticBuffer::setDirtyBit()
  return StaticBuffer::setDirtyBit(this->blockNum);
  // if setDirtyBit() failed
  // return the returned value from the call

  // return SUCCESS
}

int BlockBuffer::getFreeBlock(int blockType) {
  int blockNum;
  for (blockNum = 0; blockNum < DISK_BLOCKS; blockNum++) {
    if (StaticBuffer::blockAllocMap[blockNum] == UNUSED_BLK) {
      break;
    }
  }
  // iterate through the StaticBuffer::blockAllocMap and find the block number
  // of a free block in the disk.
  if (blockNum == DISK_BLOCKS)
    return E_DISKFULL;

  // if no block is free, return E_DISKFULL.

  // set the object's blockNum to the block number of the free block.
  this->blockNum = blockNum;

  // find a free buffer using StaticBuffer::getFreeBuffer() .
  int bufferNum = StaticBuffer::getFreeBuffer(blockNum);
  if (bufferNum < 0 or bufferNum >= BUFFER_CAPACITY) {
    printf("Error:buffer is full\n");
    return bufferNum;
  }
  // initialize the header of the block passing a struct HeadInfo with values
  // pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
  // to the setHeader() function.
  struct HeadInfo header;
  header.lblock = header.pblock = header.rblock = -1;
  header.numAttrs = header.numEntries = header.numSlots = 0;
  setHeader(&header);

  // update the block type of the block to the input block type using
  // setBlockType().
  setBlockType(blockType);
  return blockNum;

  // return block number of the free block.
}

int RecBuffer::setSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;
  /* get the starting address of the buffer containing the block using
     loadBlockAndGetBufferPtr(&bufferPtr). */
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }
  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  // return the value returned by the call.

  // get the header of the block using the getHeader() function
  HeadInfo header;
  getHeader(&header);
  int numSlots = header.numSlots; /* the number of slots in the block */
  ;
  memcpy(bufferPtr + HEADER_SIZE, slotMap, numSlots);
  // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
  // argument `slotMap` to the buffinter replacing the existing slotmap.
  // Note that size of slotmap is `numSlots`
  ret = StaticBuffer::setDirtyBit(this->blockNum);
  // update dirty bit using StaticBuffer::setDirtyBit
  // if setDirtyBit failed, return the value returned by the call
  return SUCCESS;

  // return SUCCESS
}


int BlockBuffer::getBlockNum() {
    return this->blockNum;  
}

