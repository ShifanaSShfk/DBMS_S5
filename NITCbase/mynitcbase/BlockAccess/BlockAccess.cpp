

//      Stage 4     :   linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op)

//      Stage 6     :   renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE])
                    //  renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE])

//      Stage 7     :   insert(int relId, Attribute* record)

//      Stage 9     :   project(int relId, Attribute *record)

//      Stage 10    :   search          -- modified for b+s

//      Stage 11    :   insert              --  modified for b+s
                    //  deleteRelation      --  modified for b+s

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
    
    // std::cout<<std::endl;
    // std::cout<<relCatEntry.numSlotsPerBlk<<relCatEntry.relName<<"-"<<std::endl;
    // for(int i=0;i<relCatEntry.numSlotsPerBlk;i++){
    //     std::cout<<(int)slotMap[i]-48<<" ";
    // }
    // std::cout<<std::endl;
    
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

     /* B+ Tree Insertions */
    // (the following section is only relevant once indexing has been implemented)

    int flag = SUCCESS;
    // Iterate over all the attributes of the relation
    AttrCatEntry attrCatEntry;
    for(int attrOffset = 0;attrOffset<numOfAttributes;attrOffset++){ // (let attrOffset be iterator ranging from 0 to numOfAttributes-1)
        // get the attribute catalog entry for the attribute from the attribute cache
        // (use AttrCacheTable::getAttrCatEntry() with args relId and attrOffset)
        AttrCacheTable::getAttrCatEntry(relId,attrOffset,&attrCatEntry);
        // get the root block field from the attribute catalog entry

        // if index exists for the attribute(i.e. rootBlock != -1)
        if(attrCatEntry.rootBlock!=-1){
            /* insert the new record into the attribute's bplus tree using
             BPlusTree::bPlusInsert()*/
            int retVal = BPlusTree::bPlusInsert(relId, attrCatEntry.attrName,
                                                record[attrOffset], rec_id);

            if (retVal == E_DISKFULL) {
                //(index for this attribute has been destroyed)
                // flag = E_INDEX_BLOCKS_RELEASED
                flag = E_INDEX_BLOCKS_RELEASED;
                }
        }
    }

    return flag;
}



/*
NOTE: This function will copy the result of the search to the `record` argument.
      The caller should ensure that space is allocated for `record` array
      based on the number of attributes in the relation.
*/

int BlockAccess::search(  int relId, 
                          Attribute *record, 
                          char attrName[ATTR_SIZE], 
                          Attribute attrVal, 
                          int op ) {

  RecId recId;
  
  AttrCatEntry attrCatEntry;
  int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);
  if ( ret != SUCCESS ) return ret;

  // Linear search if no root block mentioned
  if ( attrCatEntry.rootBlock == -1 ) {
    recId = linearSearch(relId, attrName, attrVal, op);
  }
  // Index exists for this attribute
  else {
    recId = BPlusTree::bPlusSearch(relId, attrName, attrVal, op);
  }

  // No record found
  if ( recId.slot == -1 && recId.block == -1 )
    return E_NOTFOUND;

  //Fetch the required record
  RecBuffer block(recId.block);
  block.getRecord(record, recId.slot);

  return SUCCESS;
}


int BlockAccess::deleteRelation(char relName[ATTR_SIZE]) {

    // if the relation to delete is either Relation Catalog or Attribute Catalog,
    //     return E_NOTPERMITTED
    // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
    // you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)

    if(strcmp(relName,RELCAT_RELNAME) == 0 || strcmp(relName,ATTRCAT_RELNAME) == 0){
        return E_NOTPERMITTED;
    }
        
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    int relId = RELCAT_RELID;

    RelCacheTable::resetSearchIndex(relId);

    Attribute relNameAttr; // (stores relName as type union Attribute)
    // assign relNameAttr.sVal = relName
    strcpy(relNameAttr.sVal,relName);

    //  linearSearch on the relation catalog for RelName = relNameAttr

    char relcat_attr_relname[ATTR_SIZE] = RELCAT_ATTR_RELNAME;

    RecId recId = linearSearch(relId,relcat_attr_relname,relNameAttr,EQ);

    // if the relation does not exist (linearSearch returned {-1, -1})
    //     return E_RELNOTEXIST
    if(recId.block == -1 || recId.slot == -1)return E_RELNOTEXIST;
     Attribute relCatEntryRecord[RELCAT_NO_ATTRS];

    /* store the relation catalog record corresponding to the relation in
       relCatEntryRecord using RecBuffer.getRecord */
    RecBuffer blockBuffer(recId.block);

    blockBuffer.getRecord(relCatEntryRecord,recId.slot);

    /* get the first record block of the relation (firstBlock) using the
       relation catalog entry record */
    /* get the number of attributes corresponding to the relation (numAttrs)
       using the relation catalog entry record */
    int numAttrs = relCatEntryRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    int blockNum = relCatEntryRecord[RELCAT_FIRST_BLOCK_INDEX].nVal;

    /*
        Delete all the record blocks of the relation
    */

    HeadInfo blockHeader;

    while(blockNum != -1){
        // for each record block of the relation:
        //     get block header using BlockBuffer.getHeader
        //     get the next block from the header (rblock)
        //     release the block using BlockBuffer.releaseBlock
        //
        //     Hint: to know if we reached the end, check if nextBlock = -1
        RecBuffer blockBuffer(blockNum);
        blockBuffer.getHeader(&blockHeader);
        blockNum = blockHeader.rblock;
        blockBuffer.releaseBlock();

    }


    /***
        Deleting attribute catalog entries corresponding the relation and index
        blocks corresponding to the relation with relName on its attributes
    ***/

    // reset the searchIndex of the attribute catalog

    int numberOfAttributesDeleted = 0;

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    while(true) {
        RecId attrCatRecId;
        // attrCatRecId = linearSearch on attribute catalog for RelName = relNameAttr
        attrCatRecId = linearSearch(ATTRCAT_RELID,relcat_attr_relname,relNameAttr,EQ);

        // if no more attributes to iterate over (attrCatRecId == {-1, -1})
        //     break;
        if(attrCatRecId.block == -1 || attrCatRecId.slot == -1)break;

        numberOfAttributesDeleted++;

        // create a RecBuffer for attrCatRecId.block
        // get the header of the block
        // get the record corresponding to attrCatRecId.slot
        RecBuffer blockBuffer(attrCatRecId.block);

        blockBuffer.getHeader(&blockHeader);

        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

        blockBuffer.getRecord(attrCatRecord,recId.slot);

        // declare variable rootBlock which will be used to store the root
        // block field from the attribute catalog record.
        int rootBlock = attrCatRecord[ATTRCAT_ROOT_BLOCK_INDEX].nVal;  // (This will be used later to delete any indexes if it exists)

        // Update the Slotmap for the block by setting the slot as SLOT_UNOCCUPIED
        // Hint: use RecBuffer.getSlotMap and RecBuffer.setSlotMap

        unsigned char slotMap[blockHeader.numSlots];

        blockBuffer.getSlotMap(slotMap);
        slotMap[attrCatRecId.slot] = SLOT_UNOCCUPIED;
        blockBuffer.setSlotMap(slotMap);

        blockHeader.numEntries--;
        /* Decrement the numEntries in the header of the block corresponding to
           the attribute catalog entry and then set back the header
           using RecBuffer.setHeader */
        blockBuffer.setHeader(&blockHeader);

        /* If number of entries become 0, releaseBlock is called after fixing
           the linked list.
        */
        if (blockHeader.numEntries == 0) {

            /* Standard Linked List Delete for a Block
               Get the header of the left block and set it's rblock to this
               block's rblock
            */

            // create a RecBuffer for lblock and call appropriate methods
            RecBuffer lBlock(blockHeader.lblock);
            HeadInfo lHeader;
            lBlock.getHeader(&lHeader);

            if (blockHeader.rblock!=-1) {
                /* Get the header of the right block and set it's lblock to
                   this block's lblock */
                // create a RecBuffer for rblock and call appropriate methods-
                RecBuffer rBlock(blockHeader.rblock);
                HeadInfo rHeader;
                rBlock.getHeader(&rHeader);

                lHeader.rblock = blockHeader.rblock;
                rHeader.lblock = blockHeader.lblock;
                
                rBlock.setHeader(&rHeader);

            } else {
                // (the block being released is the "Last Block" of the relation.)
                /* update the Relation Catalog entry's LastBlock field for this
                   relation with the block number of the previous block. */
                Attribute relRecord[blockHeader.numAttrs];
                RecBuffer relBlock(RELCAT_BLOCK);
                relBlock.getRecord(relRecord,recId.slot); //recId.slot contains the slot num for the relation in the relation catalog;
                relRecord[RELCAT_LAST_BLOCK_INDEX].nVal = blockHeader.lblock;
                relBlock.setRecord(relRecord,recId.slot);
                
            }

            // (Since the attribute catalog will never be empty(why?), we do not
            //  need to handle the case of the linked list becoming empty - i.e
            //  every block of the attribute catalog gets released.)

            // call releaseBlock()
            blockBuffer.releaseBlock();
        }

        // (the following part is only relevant once indexing has been implemented)
        // if index exists for the attribute (rootBlock != -1), call bplus destroy
        if (rootBlock != -1) {
            // delete the bplus tree rooted at rootBlock using BPlusTree::bPlusDestroy()
            BPlusTree::bPlusDestroy(rootBlock);
        }

    } 
    /*** Delete the entry corresponding to the relation from relation catalog ***/
    // Fetch the header of Relcat block

    RecBuffer relCatBlock(RELCAT_BLOCK);
    HeadInfo relHead;
    relCatBlock.getHeader(&relHead);

    /* Decrement the numEntries in the header of the block corresponding to the
       relation catalog entry and set it back */
    relHead.numEntries--;

    relCatBlock.setHeader(&relHead);

    /* Get the slotmap in relation catalog, update it by marking the slot as
       free(SLOT_UNOCCUPIED) and set it back. */
    
    unsigned char slotMap[SLOTMAP_SIZE_RELCAT_ATTRCAT];
    relCatBlock.getSlotMap(slotMap);
    slotMap[recId.slot] = SLOT_UNOCCUPIED;
    relCatBlock.setSlotMap(slotMap);

    /*** Updating the Relation Cache Table ***/

    /** Update relation catalog record entry (number of records in relation
        catalog is decreased by 1) **/
    // Get the entry corresponding to relation catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)

    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(RELCAT_RELID,&relCatEntry);
    relCatEntry.numRecs--;
    RelCacheTable::setRelCatEntry(RELCAT_RELID,&relCatEntry);


    /** Update attribute catalog entry (number of records in attribute catalog
        is decreased by numberOfAttributesDeleted) **/
    // i.e., #Records = #Records - numberOfAttributesDeleted

    // Get the entry corresponding to attribute catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)     

    RelCatEntry attrCatEntry;
    RelCacheTable::getRelCatEntry(ATTRCAT_RELID,&attrCatEntry);
    attrCatEntry.numRecs -= numberOfAttributesDeleted;
    RelCacheTable::setRelCatEntry(ATTRCAT_RELID,&attrCatEntry);

    return SUCCESS;


}

/*
NOTE: the caller is expected to allocate space for the argument `record` based
      on the size of the relation. This function will only copy the result of
      the projection onto the array pointed to by the argument.
*/

int BlockAccess::project(int relId,Attribute *record){
    // get the previous search index of the relation relId from the relation
    // cache (use RelCacheTable::getSearchIndex() function)

    // declare block and slot which will be used to store the record id of the
    // slot we need to check.

    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId,&prevRecId);
    int block=-1,slot=-1;

    /* if the current search index record is invalid(i.e. = {-1, -1})
       (this only happens when the caller reset the search index)
    */

   if (prevRecId.block == -1 && prevRecId.slot == -1){
        // (new project operation. start from beginning)

        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)

        // block = first record block of the relation
        // slot = 0
        RelCatEntry relCatEntry;
        RelCacheTable::getRelCatEntry(relId,&relCatEntry);
        block = relCatEntry.firstBlk;
        slot=0;

    }else{
        // (a project/search operation is already in progress)

        // block = previous search index's block
        // slot = previous search index's slot + 1
        block = prevRecId.block;
        slot = prevRecId.slot + 1;
    }

    // The following code finds the next record of the relation
    /* Start from the record id (block, slot) and iterate over the remaining
       records of the relation */

    while (block != -1){
        // create a RecBuffer object for block (using appropriate constructor!)

        // get header of the block using RecBuffer::getHeader() function
        // get slot map of the block using RecBuffer::getSlotMap() function

        RecBuffer recBuffer(block);
        HeadInfo head;
        recBuffer.getHeader(&head);
        unsigned char slotMap[head.numSlots];
        recBuffer.getSlotMap(slotMap);
        if(slot >= head.numSlots){
            // (no more slots in this block)
            // update block = right block of block
            // update slot = 0
            // (NOTE: if this is the last block, rblock would be -1. this would
            //        set block = -1 and fail the loop condition )
            block = head.rblock;
            slot=0;
        }else if (slotMap[slot] == SLOT_UNOCCUPIED){ // (i.e slot-th entry in slotMap contains SLOT_UNOCCUPIED)
            slot++;
            // increment slot
        }else {
            // (the next occupied slot / record has been found)
            
            break;
        }
    }

    if (block == -1){
        // (a record was not found. all records exhausted)
        return E_NOTFOUND;
    }

    // declare nextRecId to store the RecId of the record found
    RecId nextRecId{block, slot};

    // set the search index to nextRecId using RelCacheTable::setSearchIndex

    RelCacheTable::setSearchIndex(relId,&nextRecId);
    /* Copy the record with record id (nextRecId) to the record buffer (record)
       For this Instantiate a RecBuffer class object by passing the recId and
       call the appropriate method to fetch the record
    */
    RecBuffer recBuffer(nextRecId.block);
    recBuffer.getRecord(record,nextRecId.slot);

   

    return SUCCESS;

}
