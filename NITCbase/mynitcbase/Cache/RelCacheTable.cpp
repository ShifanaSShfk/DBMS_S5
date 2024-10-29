

//      Stage 3     :   getRelCatEntry(int relId, RelCatEntry* relCatBuf)
                    //  recordToRelCatEntry(union Attribute record[RELCAT_NO_ATTRS], RelCatEntry* relCatEntry)

//      Stage 4     :   getSearchIndex(int relId, RecId* searchIndex)
                    //  setSearchIndex(int relId, RecId* searchIndex)
                    //  resetSearchIndex(int relId)
                    

//      Stage 7     :   setRelCatEntry(int relId, RelCatEntry *relCatBuf)
                    //  relCatEntryToRecord(RelCatEntry *relCatEntry, union Attribute record[RELCAT_NO_ATTRS])

#include "RelCacheTable.h"

#include <cstring>

RelCacheEntry* RelCacheTable::relCache[MAX_OPEN];

int RelCacheTable::getRelCatEntry(int relId,RelCatEntry* relCatBuf){
    if(relId < 0 || relId>=MAX_OPEN){
        return E_OUTOFBOUND;
    }

    if(relCache[relId] == nullptr){
        return E_RELNOTOPEN;
    }

    //here we get the relcatEntry from the cache (simulation of cache memory)

    relCatBuf->firstBlk =  relCache[relId]->relCatEntry.firstBlk;
    relCatBuf->lastBlk =  relCache[relId]->relCatEntry.lastBlk;
    relCatBuf->numAttrs =  relCache[relId]->relCatEntry.numAttrs;
    relCatBuf->numRecs =  relCache[relId]->relCatEntry.numRecs;
    relCatBuf->numSlotsPerBlk =  relCache[relId]->relCatEntry.numSlotsPerBlk;
    strcpy(relCatBuf->relName,(relCache[relId]->relCatEntry).relName);

    return SUCCESS;

}

int RelCacheTable::setRelCatEntry(int relId,RelCatEntry* relCatBuf){
    
    if(relId < 0 || relId >= MAX_OPEN)return E_OUTOFBOUND;
    if(relCache[relId] == nullptr)return E_RELNOTOPEN;

    // copy the relCatBuf to the corresponding Relation Catalog entry in
    // the Relation Cache Table.

    relCache[relId]->relCatEntry.firstBlk = relCatBuf->firstBlk;
    relCache[relId]->relCatEntry.lastBlk = relCatBuf->lastBlk ;
    relCache[relId]->relCatEntry.numAttrs = relCatBuf->numAttrs;
    relCache[relId]->relCatEntry.numRecs = relCatBuf->numRecs;
    relCache[relId]->relCatEntry.numSlotsPerBlk = relCatBuf->numSlotsPerBlk;
    strcpy((relCache[relId]->relCatEntry).relName,relCatBuf->relName);

     // set the dirty flag of the corresponding Relation Cache entry in
    // the Relation Cache Table.

    relCache[relId]->dirty = 1;

    return SUCCESS;

}

/* Converts a relation catalog record to RelCatEntry struct
    We get the record as Attribute[] from the BlockBuffer.getRecord() function.
    This function will convert that to a struct RelCatEntry type.
NOTE: this function expects the caller to allocate memory for `*relCatEntry`
*/
void RelCacheTable::recordToRelCatEntry(union Attribute record[RELCAT_NO_ATTRS],RelCatEntry* relCatEntry){
    
    strcpy(relCatEntry->relName,record[RELCAT_REL_NAME_INDEX].sVal);
    relCatEntry->numAttrs = (int)record[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    relCatEntry->numRecs = (int)record[RELCAT_NO_RECORDS_INDEX].nVal;
    relCatEntry->firstBlk = (int)record[RELCAT_FIRST_BLOCK_INDEX].nVal;
    relCatEntry->lastBlk = (int)record[RELCAT_LAST_BLOCK_INDEX].nVal;
    relCatEntry->numSlotsPerBlk = (int) record[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal;
    
}


void RelCacheTable::relCatEntryToRecord(RelCatEntry *relCatEntry, union Attribute record[RELCAT_NO_ATTRS]){

    record[RELCAT_FIRST_BLOCK_INDEX].nVal = relCatEntry->firstBlk;
    record[RELCAT_LAST_BLOCK_INDEX].nVal = relCatEntry->lastBlk;
    record[RELCAT_NO_ATTRIBUTES_INDEX].nVal = relCatEntry->numAttrs;
    record[RELCAT_NO_RECORDS_INDEX].nVal = relCatEntry->numRecs;
    record[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal = relCatEntry->numSlotsPerBlk;
    strcpy(record[RELCAT_REL_NAME_INDEX].sVal,relCatEntry->relName);  

}

/* will return the searchIndex for the relation corresponding to `relId
NOTE: this function expects the caller to allocate memory for `*searchIndex`
*/
int RelCacheTable::getSearchIndex(int relId,RecId* searchIndex){

    if(relId < 0 || relId >= MAX_OPEN){
        return E_OUTOFBOUND;
    }

    if(relCache[relId] == nullptr){
        return E_RELNOTOPEN;
    }

    *searchIndex = relCache[relId]->searchIndex;

    return SUCCESS;
}

int RelCacheTable::setSearchIndex(int relId,RecId* searchIndex){

    if(relId < 0 || relId >= MAX_OPEN){
        return E_OUTOFBOUND;
    }

    if(relCache[relId] == nullptr){
        return E_RELNOTOPEN;
    }

    (relCache[relId]->searchIndex).block = searchIndex->block;
    (relCache[relId]->searchIndex).slot = searchIndex->slot;

    return SUCCESS;

}

int RelCacheTable::resetSearchIndex(int relId){

    RecId temp = {-1,-1};

    return setSearchIndex(relId,&temp);
}