
//      Stage 3     :   ORT
                    //  ~ORT

//      Stage 4     :   getRelId(char relName[ATTR_SIZE])

//      Stage 5     :   ORT         -- modified
                    //  ~ORT        -- modified
                    //  getFreeOpenRelTableEntry
                    //  getRelId    -- modified
                    //  openRel(char relName[ATTR_SIZE])
                    //  closeRel(int relId)

//      Stage 7     :   closeRel    -- modified

#include "OpenRelTable.h"

#include <cstring>
#include <cstdlib>
#include <iostream>

using namespace std;


OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];


OpenRelTable::OpenRelTable(){
    // initialize relCache and attrCache with nullptr
    for(int i=0;i < MAX_OPEN; i++){
        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
        tableMetaInfo[i].free = true;
    }

    /************ Setting up Relation Cache entries ************/
    // (we need to populate relation cache with entries for the relation catalog
    //  and attribute catalog.)

    /**** setting up Relation Catalog relation in the Relation Cache Table****/

    /*
        so here what we are doing is we want to add the relation catalog and attribute catalog data onto both relcache and attrcache
        Note: i have wrote the code in a way to understand it clearly so i am using different variables for each process....
        like relCatBlockRel,relCatBlockAttr,attrCatBlockRel,attrCatBlockAttr etc...
    */

    RecBuffer relCatBlock(RELCAT_BLOCK);

    for(int relIdx = RELCAT_RELID;relIdx<=ATTRCAT_RELID;relIdx++){

        Attribute relCatRecord[RELCAT_NO_ATTRS];

        relCatBlock.getRecord(relCatRecord,relIdx);

        struct RelCacheEntry relCacheEntry; //this is the input for the relCache array.....we populate this struct variable with its corresponding data and then add it to the relCacheTable

        RelCacheTable::recordToRelCatEntry(relCatRecord,&(relCacheEntry.relCatEntry)); //this converts the record to a attr catalog entry which is an entity in the struct AttrCAcheEntry

        /*
            here we set the block num and slot num for the Relation Catalog relation (the slot num is the position of the Relation relation in Relation catalog)
        */

        relCacheEntry.recId.block = RELCAT_BLOCK;
        relCacheEntry.recId.slot = relIdx;
        relCacheEntry.searchIndex = {-1,-1};

        RelCacheTable::relCache[relIdx] = new RelCacheEntry; //we allocate space to add the pointer for out AttrCacheEntry;
        *(RelCacheTable::relCache[relIdx]) = relCacheEntry;

        tableMetaInfo[relIdx].free = false;
        strcpy(tableMetaInfo[relIdx].relName,relCacheEntry.relCatEntry.relName);

    }


    // //----------students relation to relcache-------------//

    // //this is for the students relation

    // //here we are using the RecBuffer of relCatRecordRel block
    // relCatBlockRel.getRecord(relCatRecordRel,2);

    // RelCacheTable::recordToRelCatEntry(relCatRecordRel,&relCacheEntryRel.relCatEntry);

    // relCacheEntryRel.recId.block = RELCAT_BLOCK;
    // relCacheEntryRel.recId.slot = 2;
    // relCacheEntryRel.searchIndex = {-1,-1};

    // RelCacheTable::relCache[2] = new RelCacheEntry;
    // *(RelCacheTable::relCache[2]) = relCacheEntryRel;

    // //----------students relation to relcache done---------//


    /************ Setting up Attribute cache entries ************/
    // (we need to populate attribute cache with entries for the relation catalog
    //  and attribute catalog.)

    /**** setting up Relation Catalog relation in the Attribute Cache Table ****/

    RecBuffer attrCatBlockRel(ATTRCAT_BLOCK);

    Attribute attrCatRecordRel[ATTRCAT_NO_ATTRS];

    struct AttrCacheEntry* attrCacheEntryRel = new AttrCacheEntry;
    struct AttrCacheEntry* attrCacheEntryRelTemp =  attrCacheEntryRel; // (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    
    for(int i = 0;i < RELCAT_NO_ATTRS; i++){

        attrCatBlockRel.getRecord(attrCatRecordRel,i);

        //this converts the record to a attr catalog entry which is an entity in the struct AttrCacheEntry
        AttrCacheTable::recordToAttrCatEntry(attrCatRecordRel,&(attrCacheEntryRelTemp->attrCatEntry));

        attrCacheEntryRelTemp->recId.block = ATTRCAT_BLOCK;
        attrCacheEntryRelTemp->recId.slot = i;
        attrCacheEntryRelTemp->searchIndex = {-1,-1};

        if(i < RELCAT_NO_ATTRS-1) attrCacheEntryRelTemp->next = new AttrCacheEntry; // here we check if its the last iteration or not if it is we go to else scope and temp->next = nullptr
        else attrCacheEntryRelTemp->next = nullptr;

        attrCacheEntryRelTemp = attrCacheEntryRelTemp->next;
    
    }

    AttrCacheTable::attrCache[RELCAT_RELID] = attrCacheEntryRel; //here since the pointer is already allocated in heap we dont need to malloc again...just set the pointer :)

    /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/
    //same as the above code

    RecBuffer attrCatBlockAttr(ATTRCAT_BLOCK);

    Attribute attrCatRecordAttr[ATTRCAT_NO_ATTRS];

    struct AttrCacheEntry* attrCacheEntryAttr = new AttrCacheEntry;
    struct AttrCacheEntry* attrCacheEntryAttrTemp = attrCacheEntryAttr;

    int startAttrIndex = RELCAT_NO_ATTRS,lastAttrIndex = RELCAT_NO_ATTRS + ATTRCAT_NO_ATTRS;

    for(int i = startAttrIndex;i <= lastAttrIndex;i++){

        attrCatBlockAttr.getRecord(attrCatRecordAttr,i);
        AttrCacheTable::recordToAttrCatEntry(attrCatRecordAttr,&(attrCacheEntryAttrTemp->attrCatEntry));

        attrCacheEntryAttrTemp->recId.block = ATTRCAT_BLOCK;
        attrCacheEntryAttrTemp->recId.slot = i;
        attrCacheEntryAttrTemp->searchIndex = {-1,-1};

        if(i<lastAttrIndex) attrCacheEntryAttrTemp->next = new AttrCacheEntry;
        else attrCacheEntryAttrTemp->next = nullptr;

        attrCacheEntryAttrTemp = attrCacheEntryAttrTemp->next;
    }

    AttrCacheTable::attrCache[ATTRCAT_RELID] = attrCacheEntryAttr;

    //----------students attributes to attrcache-----------//

    // attrCacheEntryRel = new AttrCacheEntry;
    // attrCacheEntryRelTemp = attrCacheEntryRel;
    // for(int i=12;i<16;i++){

    //     attrCatBlockRel.getRecord(attrCatRecordRel,i);
    //     AttrCacheTable::recordToAttrCatEntry(attrCatRecordRel,&(attrCacheEntryRelTemp->attrCatEntry));

    //     attrCacheEntryRelTemp->recId.block = ATTRCAT_BLOCK;
    //     attrCacheEntryRelTemp->recId.slot = i;
    //     attrCacheEntryRelTemp->searchIndex = {-1,-1};

    //     if(i<15)attrCacheEntryRelTemp->next = new AttrCacheEntry;
    //     else attrCacheEntryRelTemp->next = nullptr;

    //     attrCacheEntryRelTemp = attrCacheEntryRelTemp->next;

    // }

    // AttrCacheTable::attrCache[2] = attrCacheEntryRel;

    //-----students attribute to attrcache done------------//
    

}
/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.
*/

int OpenRelTable::getRelId(char relName[ATTR_SIZE]){

    for(int idx=0;idx<MAX_OPEN;idx++){
        if(!tableMetaInfo[idx].free && strcmp(tableMetaInfo[idx].relName,relName) == 0){
            return idx;
        }
    }

    return E_RELNOTOPEN;

}

int OpenRelTable::openRel(char relName[ATTR_SIZE]){

    // let relId be used to store the free slot.
   
    int relId = getRelId(relName);

    

    if(relId != E_RELNOTOPEN){
        return relId;
    }

    /* find a free slot in the Open Relation Table
    using OpenRelTable::getFreeOpenRelTableEntry(). */
    
    relId = getFreeOpenRelTableEntry();

    if(relId == E_CACHEFULL){
        return relId;
    }

    /****** Setting up Relation Cache entry for the relation ******/

    /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().
    */
    // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.

    RecId relcatRecId;

    Attribute relAttr;

    strcpy(relAttr.sVal,relName);

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    char relcat_attr_relname[ATTR_SIZE] = RELCAT_ATTR_RELNAME;

    relcatRecId = BlockAccess::linearSearch(RELCAT_RELID,relcat_attr_relname,relAttr,EQ);


    if(relcatRecId.block == -1 && relcatRecId.slot == -1){
        // (the relation is not found in the Relation Catalog.)// (the relation is not found in the Relation Catalog.)
        return E_RELNOTEXIST;
    }

    /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
    */

    RecBuffer blockBuffer(relcatRecId.block);

    HeadInfo head;

    blockBuffer.getHeader(&head);

    Attribute relRecord[head.numAttrs];

    blockBuffer.getRecord(relRecord,relcatRecId.slot);

    RelCacheEntry* relCacheEntry = new RelCacheEntry;

    RelCacheTable::recordToRelCatEntry(relRecord,&(relCacheEntry->relCatEntry));

    relCacheEntry->recId.block = relcatRecId.block;
    relCacheEntry->recId.slot = relcatRecId.slot;

    RelCacheTable::relCache[relId] = relCacheEntry;

    /****** Setting up Attribute Cache entry for the relation ******/

    // let listHead be used to hold the head of the linked list of attrCache entries.
    
    AttrCacheEntry* listHead = new AttrCacheEntry;
    
    AttrCacheEntry* temp = listHead;

    /*iterate over all the entries in the Attribute Catalog corresponding to each
    attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
    care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
    corresponding to Attribute Catalog before the first call to linearSearch().*/

    /* let attrcatRecId store a valid record id an entry of the relation, relName,
    in the Attribute Catalog.*/


    
    {
        RecId attrcatRecId;

        int attrSize = relCacheEntry->relCatEntry.numAttrs;

        Attribute rec[attrSize];

        RelCacheTable::resetSearchIndex(ATTRCAT_RELID);


        while(attrSize--){

            attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID,relcat_attr_relname,relAttr,EQ);

            RecBuffer blockBuffer(attrcatRecId.block);

            blockBuffer.getRecord(rec,attrcatRecId.slot);

            AttrCacheTable::recordToAttrCatEntry(rec,&(temp->attrCatEntry));

            temp->recId.block = attrcatRecId.block;
            temp->recId.slot = attrcatRecId.slot;

            if(attrSize > 0)temp->next = new AttrCacheEntry;
            else temp->next = nullptr;

            temp = temp->next;

        }

        AttrCacheTable::attrCache[relId] = listHead;

    }

    /****** Setting up metadata in the Open Relation Table for the relation******/

    // update the relIdth entry of the tableMetaInfo with free as false and
    // relName as the input.
    

    tableMetaInfo[relId].free = false;
    strcpy(tableMetaInfo[relId].relName,relName);

    return relId;

}

int OpenRelTable::getFreeOpenRelTableEntry(){
    
    /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/

    for(int relId =0;relId <MAX_OPEN;relId ++){
        if(tableMetaInfo[relId].free){
            return relId;
        }
    }

    // if found return the relation id, else return E_CACHEFULL.
    return E_CACHEFULL;
}

void freeAttrCacheEntry(AttrCacheEntry* head){
    if(!head)return;
    freeAttrCacheEntry(head->next);
    delete head;
}

int OpenRelTable::closeRel(int relId){
    if(relId == RELCAT_RELID || relId == ATTRCAT_RELID){
        return E_NOTPERMITTED;
    }
    if(relId < 0 || relId >= MAX_OPEN){
        return E_OUTOFBOUND;
    }

    if(tableMetaInfo[relId].free){
        return E_RELNOTOPEN;
    }

    /****** Releasing the Relation Cache entry of the relation ******/

    if (RelCacheTable::relCache[relId]->dirty){

        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */

        Attribute rec[RELCAT_NO_ATTRS];

        RelCacheTable::relCatEntryToRecord(&(RelCacheTable::relCache[relId]->relCatEntry),rec);

        // declaring an object of RecBuffer class to write back to the buffer
        RecId recId = RelCacheTable::relCache[relId]->recId;

        RecBuffer relCatBlock(recId.block);

        // Write back to the buffer using relCatBlock.setRecord() with recId.slot

        relCatBlock.setRecord(rec,recId.slot);

    }

    /****** Releasing the Attribute Cache entry of the relation ******/

    delete RelCacheTable::relCache[relId];
    freeAttrCacheEntry(AttrCacheTable::attrCache[relId]);

    AttrCacheTable::attrCache[relId] = nullptr;
    RelCacheTable::relCache[relId] = nullptr;
    
    tableMetaInfo[relId].free = true;

    return SUCCESS;


}


OpenRelTable::~OpenRelTable() {
    // free all the memory that you allocated in the constructor

    // close all open relations (from rel-id = 2 onwards. Why?)

    for (int i = 2; i < MAX_OPEN; ++i) {
        if (!tableMetaInfo[i].free) {
            closeRel(i); // we will implement this function later
        }
    }

    /**** Closing the catalog relations in the relation cache ****/

    //releasing the relation cache entry of the attribute catalog

    RelCacheEntry RelCacheEntry;

    if (RelCacheTable::relCache[ATTRCAT_RELID]->dirty) {

        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */

        Attribute rec[RELCAT_NO_ATTRS];
        RelCacheTable::relCatEntryToRecord(&(RelCacheTable::relCache[ATTRCAT_RELID]->relCatEntry),rec);

        // declaring an object of RecBuffer class to write back to the buffer

        RecId recId = RelCacheTable::relCache[ATTRCAT_RELID]->recId;
        RecBuffer relCatBlock(recId.block);

        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
        relCatBlock.setRecord(rec,recId.slot);

    }

    //releasing the relation cache entry of the relation catalog

    if(RelCacheTable::relCache[RELCAT_RELID]->dirty) {

        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */

        Attribute rec[RELCAT_NO_ATTRS];
        RelCacheTable::relCatEntryToRecord(&(RelCacheTable::relCache[RELCAT_RELID]->relCatEntry),rec);

        // declaring an object of RecBuffer class to write back to the buffer
        RecId recId = RelCacheTable::relCache[RELCAT_RELID]->recId;
        RecBuffer relCatBlock(recId.block);

        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
        relCatBlock.setRecord(rec,recId.slot);
    }
    
    // free the memory dynamically allocated to this RelCacheEntry
    
    //here we free all the elements from the relcache array
    for(int relCacheIdx=0;relCacheIdx<MAX_OPEN;relCacheIdx++){
        if(RelCacheTable::relCache[relCacheIdx]!=nullptr) delete RelCacheTable::relCache[relCacheIdx];
        
    }

    // here we recursively free all allocated memory as it is of linked list type and we have to traverse the whole list to free the the list
    // using the freeAttrCacheEntry function
    for(int attrCacheIdx=0;attrCacheIdx<MAX_OPEN;attrCacheIdx++) freeAttrCacheEntry(AttrCacheTable::attrCache[attrCacheIdx]);

    
}