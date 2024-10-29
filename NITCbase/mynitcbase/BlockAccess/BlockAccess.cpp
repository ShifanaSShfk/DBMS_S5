#include "BlockAccess.h"
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>


//      Stage 4     :   linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op)

//      Stage 6     :   renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE])
                    //  renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE])

//      Stage 7     :   insert(int relId, Attribute* record)

#include "BlockAccess.h"

#include <cstring>
#include <iostream>


int BlockAccess::renameRelation(char oldName[ATTR_SIZE],char newName[ATTR_SIZE]){
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    char relcat_attr_relname[ATTR_SIZE] = RELCAT_ATTR_RELNAME;

    Attribute newRelationName;    // set newRelationName with newName

    strcpy(newRelationName.sVal,newName);

    // search the relation catalog for an entry with "RelName" = newRelationName

    RecId recId = linearSearch(RELCAT_RELID,relcat_attr_relname,newRelationName,EQ);

    if(recId.block != -1 && recId.slot != -1){
        return E_RELEXIST;
    }

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */

    Attribute oldRelationName;    // set oldRelationName with oldName

    strcpy(oldRelationName.sVal,oldName);

    // search the relation catalog for an entry with "RelName" = oldRelationName

    recId = {-1,-1};

    recId = linearSearch(RELCAT_RELID,relcat_attr_relname,oldRelationName,EQ);

    if(recId.block == -1 && recId.slot == -1){
        return E_RELNOTEXIST;
    }

    /* get the relation catalog record of the relation to rename using a RecBuffer
       on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
    */
    /* update the relation name attribute in the record with newName.
       (use RELCAT_REL_NAME_INDEX) */
    // set back the record value using RecBuffer.setRecord

    Attribute rec[RELCAT_NO_ATTRS];

    RecBuffer recBuffer(RELCAT_BLOCK);

    recBuffer.getRecord(rec,recId.slot);

    strcpy(rec[RELCAT_REL_NAME_INDEX].sVal,newRelationName.sVal);

    recBuffer.setRecord(rec,recId.slot);

     /*
    update all the attribute catalog entries in the attribute catalog corresponding
    to the relation with relation name oldName to the relation name newName
    */

    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    int numberOfAttributes = (int) rec[RELCAT_NO_ATTRIBUTES_INDEX].nVal;

    //for i = 0 to numberOfAttributes :
    //    linearSearch on the attribute catalog for relName = oldRelationName
    //    get the record using RecBuffer.getRecord
    //
    //    update the relName field in the record to newName
    //    set back the record using RecBuffer.setRecord

    recId = {-1,-1};

    for(int idx=0;idx<numberOfAttributes;idx++){

        recId = linearSearch(ATTRCAT_RELID,relcat_attr_relname,oldRelationName,EQ);

        RecBuffer recBuffer(recId.block);

        Attribute attrRec[ATTRCAT_NO_ATTRS];

        recBuffer.getRecord(attrRec,recId.slot);

        strcpy(attrRec[ATTRCAT_REL_NAME_INDEX].sVal,newRelationName.sVal);

        recBuffer.setRecord(attrRec,recId.slot);

    }

    return SUCCESS;

}

int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE]){
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute relNameAttr;    // set relNameAttr with newName

    strcpy(relNameAttr.sVal,relName);

    char relcat_attr_relname[ATTR_SIZE] = RELCAT_ATTR_RELNAME;

    // search the relation catalog for an entry with "RelName" = relNameAttr

    RecId recId = linearSearch(RELCAT_RELID,relcat_attr_relname,relNameAttr,EQ);

    if(recId.block == -1 && recId.slot == -1){
        return E_RELNOTEXIST;
    }

    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    /* declare variable attrToRenameRecId used to store the attr-cat recId
    of the attribute to rename */
    RecId attrToRenameRecId{-1, -1};
    recId = {-1,-1};
    Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];

    /* iterate over all Attribute Catalog Entry record corresponding to the
       relation to find the required attribute */
    
    
    
    while (true) {

        // linear search on the attribute catalog for RelName = relNameAttr
        recId = linearSearch(ATTRCAT_RELID,relcat_attr_relname,relNameAttr,EQ);
        
        if(recId.block == -1 && recId.slot == -1){
            // if there are no more attributes left to check (linearSearch returned {-1,-1})
            //     break;
            break;
        }

        RecBuffer recBuffer(recId.block);
        /* Get the record from the attribute catalog using RecBuffer.getRecord
          into attrCatEntryRecord */
        recBuffer.getRecord(attrCatEntryRecord,recId.slot);

        if(strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,oldName) == 0){
            // if attrCatEntryRecord.attrName = oldName
            //     attrToRenameRecId = block and slot of this record
            attrToRenameRecId.block = recId.block,attrToRenameRecId.slot = recId.slot;
        }

        if(strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName) == 0){
            // if attrCatEntryRecord.attrName = newName
            // return E_ATTREXIST;
            return E_ATTREXIST;
        }
        
    }
    if(attrToRenameRecId.block == -1 && attrToRenameRecId.slot == -1){
        // if attrToRenameRecId == {-1, -1}
        //     return E_ATTRNOTEXIST;
        return E_ATTRNOTEXIST;
    }

    // Update the entry corresponding to the attribute in the Attribute Catalog Relation.
    /*   declare a RecBuffer for attrToRenameRecId.block and get the record at
         attrToRenameRecId.slot */
    //   update the AttrName of the record with newName
    //   set back the record with RecBuffer.setRecord

    RecBuffer recBuffer(attrToRenameRecId.block);

    recBuffer.getRecord(attrCatEntryRecord,attrToRenameRecId.slot);

    strcpy(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName);

    recBuffer.setRecord(attrCatEntryRecord,attrToRenameRecId.slot);

    return SUCCESS;
    
    
}

RecId BlockAccess::linearSearch(int relId,char attrName[ATTR_SIZE],union Attribute attrVal,int op){
    
    RecId prevRecId;

    RelCacheTable::getSearchIndex(relId,&prevRecId);

    RecId currRecId = {-1,-1};

    

    if(prevRecId.block == -1 && prevRecId.slot == -1){
        
        RelCatEntry relCatEntry;

        RelCacheTable::getRelCatEntry(relId,&relCatEntry);

        currRecId.block = relCatEntry.firstBlk;

        currRecId.slot = 0;

    }else{

        currRecId.block = prevRecId.block;

        currRecId.slot = prevRecId.slot + 1;

    }

    /* The following code searches for the next record in the relation
       that satisfies the given condition
       We start from the record id (block, slot) and iterate over the remaining
       records of the relation
    */

   RelCatEntry relCatBuf;

   RelCacheTable::getRelCatEntry(relId,&relCatBuf);
    
    while(currRecId.block != -1){
        /* create a RecBuffer object for block (use RecBuffer Constructor for
           existing block) */
        RecBuffer recBlock(currRecId.block);
        HeadInfo head;
        recBlock.getHeader(&head);

        // get the record with id (block, slot) using RecBuffer::getRecord()
        // get header of the block using RecBuffer::getHeader() function
        // get slot map of the block using RecBuffer::getSlotMap() function
        
        Attribute record[head.numAttrs]; //this is done so as to get the correct space we need..this info is accquired from the meta data of the block
        recBlock.getRecord(record,currRecId.slot);

        unsigned char slotMap[head.numSlots];
        recBlock.getSlotMap(slotMap);
        
        if(currRecId.slot >= relCatBuf.numSlotsPerBlk){
            currRecId.block = head.rblock;
            currRecId.slot = 0;
            continue;
        }
        if(slotMap[currRecId.slot] == SLOT_UNOCCUPIED){
            currRecId.slot++;
            continue;
        }


        /* use the attribute offset to get the value of the attribute from
           current record
        */

        AttrCatEntry attrCatBuf;
    
        AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatBuf);
        
        int attrOffset = attrCatBuf.offset;

        /* Next task is to check whether this record satisfies the given condition.
           It is determined based on the output of previous comparison and
           the op value received.
           The following code sets the cond variable if the condition is satisfied.
        */

        int cmpVal = compareAttrs(record[attrOffset],attrVal,attrCatBuf.attrType);


        
        if((op == NE && cmpVal != 0) ||    // if op is "not equal to"
            (op == LT && cmpVal < 0) ||     // if op is "less than"
            (op == LE && cmpVal <= 0) ||    // if op is "less than or equal to"
            (op == EQ && cmpVal == 0) ||    // if op is "equal to"
            (op == GT && cmpVal > 0) ||     // if op is "greater than"
            (op == GE && cmpVal >= 0) ){
            
                 /*
                set the search index in the relation cache as
                the record id of the record that satisfies the given condition
                (use RelCacheTable::setSearchIndex function)
                */
               
                RelCacheTable::setSearchIndex(relId,&currRecId);

                return currRecId;
            }

            currRecId.slot = currRecId.slot + 1;
    }

    // no record in the relation with Id relid satisfies the given condition
    RelCacheTable::resetSearchIndex(relId);
    return RecId{-1,-1};
}

int BlockAccess::insert(int relId,union Attribute* record){
    
    // get the relation catalog entry from relation cache
    // ( use RelCacheTable::getRelCatEntry() of Cache Layer)
    RelCatEntry relCatEntry;

    // std::cout<<record[0].sVal<<std::endl;

    RelCacheTable::getRelCatEntry(relId,&relCatEntry);
    
    int blockNum =  relCatEntry.firstBlk; /* first record block of the relation (from the rel-cat entry)*/;

    // rec_id will be used to store where the new record will be inserted
    RecId rec_id = {-1, -1};

    int numOfSlots = relCatEntry.numSlotsPerBlk /* number of slots per record block */;
    int numOfAttributes = relCatEntry.numAttrs /* number of attributes of the relation */;

    int prevBlockNum = -1 /* block number of the last element in the linked list = -1 */;

    /*
        Traversing the linked list of existing record blocks of the relation
        until a free slot is found OR
        until the end of the list is reached
    */
    while (blockNum != -1) {
        // create a RecBuffer object for blockNum (using appropriate constructor!)
        RecBuffer blockBuffer(blockNum);
        HeadInfo head;
        
        // get header of block(blockNum) using RecBuffer::getHeader() function
        
        blockBuffer.getHeader(&head);

        // get slot map of block(blockNum) using RecBuffer::getSlotMap() function
        unsigned char slotMap[head.numSlots];

        blockBuffer.getSlotMap(slotMap);

        // search for free slot in the block 'blockNum' and store it's rec-id in rec_id
        // (Free slot can be found by iterating over the slot map of the block)
        /* slot map stores SLOT_UNOCCUPIED if slot is free and
           SLOT_OCCUPIED if slot is occupied) */
        
        for(int i=0;i<head.numSlots;i++){

            if(slotMap[i] == SLOT_UNOCCUPIED){
                rec_id.block = blockNum;
                rec_id.slot = i;
                break;
            }
        }

        /* if a free slot is found, set rec_id and discontinue the traversal
           of the linked list of record blocks (break from the loop) */
        
        if(rec_id.block != -1 && rec_id.slot != -1) break;
        
        /* otherwise, continue to check the next block by updating the
           block numbers as follows:
              update prevBlockNum = blockNum
              update blockNum = header.rblock (next element in the linked
                                               list of record blocks)
        */
       prevBlockNum = blockNum;
       blockNum = head.rblock;

    }

    //  if no free slot is found in existing record blocks (rec_id = {-1, -1})
    if(rec_id.block == -1 && rec_id.slot == -1){
        
        // if relation is RELCAT, do not allocate any more blocks
        //     return E_MAXRELATIONS;
        if(relId == RELCAT_RELID)return E_MAXRELATIONS;

        
        // Otherwise,
        // get a new record block (using the appropriate RecBuffer constructor!)
        // get the block number of the newly allocated block
        // (use BlockBuffer::getBlockNum() function)
        // let ret be the return value of getBlockNum() function call

        RecBuffer blockBuffer;

        int ret = blockBuffer.getBlockNum();

        if(ret == E_DISKFULL) return E_DISKFULL;

        // Assign rec_id.block = new block number(i.e. ret) and rec_id.slot = 0

        rec_id.block = ret,rec_id.slot = 0;

        /*
            set the header of the new record block such that it links with
            existing record blocks of the relation
            set the block's header as follows:
            blockType: REC, pblock: -1
            lblock
                  = -1 (if linked list of existing record blocks was empty
                         i.e this is the first insertion into the relation)
                  = prevBlockNum (otherwise),
            rblock: -1, numEntries: 0,
            numSlots: numOfSlots, numAttrs: numOfAttributes
            (use BlockBuffer::setHeader() function)
        */

        HeadInfo head;

        head.blockType = REC;
        head.pblock = head.rblock = -1;
        head.lblock = prevBlockNum;
        head.numEntries = 0;
        head.numAttrs = numOfAttributes,head.numSlots = numOfSlots;

        blockBuffer.setHeader(&head);

        /*
            set block's slot map with all slots marked as free
            (i.e. store SLOT_UNOCCUPIED for all the entries)
            (use RecBuffer::setSlotMap() function)
        */

        unsigned char slotMap[numOfSlots];

        for(int i=0;i<numOfSlots;i++){
            slotMap[i] = SLOT_UNOCCUPIED;
        }

        blockBuffer.setSlotMap(slotMap);

        if (prevBlockNum != -1){

            // create a RecBuffer object for prevBlockNum
            // get the header of the block prevBlockNum and
            // update the rblock field of the header to the new block
            // number i.e. rec_id.block
            // (use BlockBuffer::setHeader() function)

            RecBuffer prevBlock(prevBlockNum);
            HeadInfo prevHead;

            prevBlock.getHeader(&prevHead);

            prevHead.rblock = rec_id.block;

            prevBlock.setHeader(&prevHead);

        }else{
            // update first block field in the relation catalog entry to the
            // new block (using RelCacheTable::setRelCatEntry() function)

            relCatEntry.firstBlk = rec_id.block;

        }

        relCatEntry.lastBlk = rec_id.block;

        RelCacheTable::setRelCatEntry(relId,&relCatEntry);

    }
    // create a RecBuffer object for rec_id.block
    // insert the record into rec_id'th slot using RecBuffer.setRecord())
    RecBuffer currBlockBuffer(rec_id.block);
    currBlockBuffer.setRecord(record,rec_id.slot);

    /* update the slot map of the block by marking entry of the slot to
       which record was inserted as occupied) */
    
    unsigned char slotMap[numOfSlots];
    // (ie store SLOT_OCCUPIED in free_slot'th entry of slot map)
    // (use RecBuffer::getSlotMap() and RecBuffer::setSlotMap() functions)
    
    currBlockBuffer.getSlotMap(slotMap);
    slotMap[rec_id.slot] = SLOT_OCCUPIED;
    currBlockBuffer.setSlotMap(slotMap);
    

    // increment the numEntries field in the header of the block to
    // which record was inserted
    // (use BlockBuffer::getHeader() and BlockBuffer::setHeader() functions)
    HeadInfo currHeader;
    currBlockBuffer.getHeader(&currHeader);
    currHeader.numEntries++;
    currBlockBuffer.setHeader(&currHeader);
    
    

    
    
    // Increment the number of records field in the relation cache entry for
    // the relation. (use RelCacheTable::setRelCatEntry function)

    RelCacheTable::getRelCatEntry(relId,&relCatEntry);
    relCatEntry.numRecs++;
    RelCacheTable::setRelCatEntry(relId,&relCatEntry);

    return SUCCESS;
}