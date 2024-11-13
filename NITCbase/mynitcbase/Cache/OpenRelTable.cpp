#include "OpenRelTable.h"
#include <stdlib.h>
#include <cstring>

//      Stage 3     :   ORT
                    //  ~ORT

//      Stage 4     :   getRelId(char relName[ATTR_SIZE])

//      Stage 5     :   ORT         -- modified
                    //  ~ORT        -- modified
                    //  getFreeOpenRelTableEntry
                    //  getRelId    -- modified
                    //  openRel(char relName[ATTR_SIZE])
                    //  closeRel(int relId)

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];
AttrCacheEntry* createList(int length) {
    AttrCacheEntry* head = (AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));
    AttrCacheEntry* tail = head;
    for (int i = 1; i < length; i++) {
        tail->next = (AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));
        tail = tail->next;
    }
    tail->next = nullptr;
    return head;
}

void clearList(AttrCacheEntry* head) {
    for (AttrCacheEntry* it = head, *next; it != nullptr; it = next) {
        next = it->next;
        free(it);
    }
}

AttrCacheEntry *createAttrCacheEntryList(int size)
{
    AttrCacheEntry *head = nullptr, *curr = nullptr;
    head = curr = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    size--;
    while (size--)
    {
        curr->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
        curr = curr->next;
    }
    curr->next = nullptr;

    return head;
}

OpenRelTable::OpenRelTable() {
    // Initialize each entry in the relation and attribute cache tables to nullptr 
    // and mark all entries in the tableMetaInfo array as free.
    for (int i = 0; i < MAX_OPEN; i++) {
        RelCacheTable::relCache[i] = nullptr;  // Initialize relation cache entry to nullptr
        AttrCacheTable::attrCache[i] = nullptr; // Initialize attribute cache entry to nullptr
        OpenRelTable::tableMetaInfo[i].free = true; // Mark tableMetaInfo entry as free
    }

    // Initialize the relation catalog block and temporary storage for relation catalog records
    RecBuffer relCatBlock(RELCAT_BLOCK);
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    
    // Loop through each entry in the relation catalog (RELCAT_RELID to ATTRCAT_RELID)
    // and populate the relation cache with these entries.
    for (int i = RELCAT_RELID; i <= ATTRCAT_RELID; i++) {
        relCatBlock.getRecord(relCatRecord, i); // Fetch the i-th record from the relation catalog

        struct RelCacheEntry relCacheEntry; // Temporary storage for relation cache entry
        RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry); // Convert record to cache entry
        relCacheEntry.recId.block = RELCAT_BLOCK; // Assign block number to recId
        relCacheEntry.recId.slot = i; // Assign slot number to recId

        // Allocate memory for the cache entry and store it in the relation cache
        RelCacheTable::relCache[i] = (struct RelCacheEntry*) malloc(sizeof(RelCacheEntry));
        *(RelCacheTable::relCache[i]) = relCacheEntry; // Copy the constructed entry to the cache
        tableMetaInfo[i].free = false; // Mark this tableMetaInfo entry as occupied
        memcpy(tableMetaInfo[i].relName, relCacheEntry.relCatEntry.relName, ATTR_SIZE); // Store the relation name
    }

    // Initialize the attribute catalog block and temporary storage for attribute catalog records
    RecBuffer attrCatBlock(ATTRCAT_BLOCK);
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    
    // Create a linked list for storing attribute cache entries for the relation catalog attributes
    auto relCatListHead = createAttrCacheEntryList(RELCAT_NO_ATTRS);
    auto attrCacheEntry = relCatListHead;

    // Populate the attribute cache with entries corresponding to relation catalog attributes
    for (int i = 0; i < RELCAT_NO_ATTRS; i++) {
        attrCatBlock.getRecord(attrCatRecord, i); // Fetch the i-th record from the attribute catalog

        // Convert record to attribute cache entry and store in the linked list
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &(attrCacheEntry->attrCatEntry));
        (attrCacheEntry->recId).block = ATTRCAT_BLOCK; // Assign block number to recId
        (attrCacheEntry->recId).slot = i; // Assign slot number to recId
        
        attrCacheEntry = attrCacheEntry->next; // Move to the next entry in the list
    }
    AttrCacheTable::attrCache[RELCAT_RELID] = relCatListHead; // Store the head of the list in the attribute cache

    // Create a linked list for storing attribute cache entries for the attribute catalog attributes
    auto attrCatListHead = createAttrCacheEntryList(ATTRCAT_NO_ATTRS);
    attrCacheEntry = attrCatListHead;
    
    // Populate the attribute cache with entries corresponding to attribute catalog attributes
    for (int i = RELCAT_NO_ATTRS; i < RELCAT_NO_ATTRS + ATTRCAT_NO_ATTRS; i++) {
        attrCatBlock.getRecord(attrCatRecord, i); // Fetch the i-th record from the attribute catalog
        // Convert record to attribute cache entry and store in the linked list
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &(attrCacheEntry->attrCatEntry));
        (attrCacheEntry->recId).block = ATTRCAT_BLOCK; // Assign block number to recId
        (attrCacheEntry->recId).slot = i; // Assign slot number to recId

        attrCacheEntry = attrCacheEntry->next; // Move to the next entry in the list
    }
    AttrCacheTable::attrCache[ATTRCAT_RELID] = attrCatListHead; // Store the head of the list in the attribute cache
}


OpenRelTable::~OpenRelTable() {
    // close all open relations
    for (int i = 2; i < MAX_OPEN; i++) {
        if (!tableMetaInfo[i].free){
            OpenRelTable::closeRel(i);
        }
        // we will implement this function later
    }


    // free the memory allocated for rel-id 0 and 1 in the caches
    for (int i = 0; i < MAX_OPEN; i++) {
        free(RelCacheTable::relCache[i]);
        clearList(AttrCacheTable::attrCache[i]);

        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
    }
}


/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.
*/
int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {
     /* traverse through the tableMetaInfo array,
    find the entry in the Open Relation Table corresponding to relName.*/
    for (int i = 0; i < MAX_OPEN; i++) {
        if (!tableMetaInfo[i].free && strcmp(relName, tableMetaInfo[i].relName) == 0){
            return i;
        }
    }

    // if found return the relation id, else indicate that the relation do not
    // have an entry in the Open Relation Table.
    return E_RELNOTOPEN;
}

int OpenRelTable::getFreeOpenRelTableEntry() {
    /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/
    for (int i = 2; i < MAX_OPEN; i++) {
        if (tableMetaInfo[i].free)
            return i;
    }
    // if found return the relation id, else return E_CACHEFULL.
    return E_CACHEFULL;
}


int OpenRelTable::openRel(char relName[ATTR_SIZE]) {
    int alreadyExists = OpenRelTable::getRelId(relName);

    if (alreadyExists >= 0)
        return alreadyExists;

    int freeSlot = OpenRelTable::getFreeOpenRelTableEntry();


    if (freeSlot == E_CACHEFULL)
        return E_CACHEFULL;

    Attribute relNameAttribute;
    memcpy(relNameAttribute.sVal, relName, ATTR_SIZE);
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, (char*)RELCAT_ATTR_RELNAME, relNameAttribute, EQ);

    if (relcatRecId.block == -1 && relcatRecId.slot == -1)
        return E_RELNOTEXIST;
    RecBuffer recBuffer(relcatRecId.block);

    Attribute record[RELCAT_NO_ATTRS];
    recBuffer.getRecord(record, relcatRecId.slot);

    RelCatEntry relCatEntry;

    RelCacheTable::recordToRelCatEntry(record, &relCatEntry);
    RelCacheTable::relCache[freeSlot] = (RelCacheEntry*) malloc(sizeof(RelCacheEntry));

    RelCacheTable::relCache[freeSlot]->recId = relcatRecId;
    RelCacheTable::relCache[freeSlot]->relCatEntry = relCatEntry;


    int numAttrs = relCatEntry.numAttrs;
    AttrCacheEntry* listHead = createList(numAttrs);
    AttrCacheEntry* node = listHead;

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    while(true) {
        RecId searchRes = BlockAccess::linearSearch(ATTRCAT_RELID, (char*)ATTRCAT_ATTR_RELNAME, relNameAttribute, EQ);

        if (searchRes.block != -1 && searchRes.slot != -1) {
            Attribute attrcatRecord[ATTRCAT_NO_ATTRS];

            RecBuffer attrRecBuffer(searchRes.block);

            attrRecBuffer.getRecord(attrcatRecord, searchRes.slot);

            AttrCatEntry attrCatEntry;
            AttrCacheTable::recordToAttrCatEntry(attrcatRecord, &attrCatEntry);

            node->recId = searchRes;
            node->attrCatEntry = attrCatEntry;
            node = node->next;
        }
        else 
            break;
    }

    AttrCacheTable::attrCache[freeSlot] = listHead;

    OpenRelTable::tableMetaInfo[freeSlot].free = false;
    memcpy(OpenRelTable::tableMetaInfo[freeSlot].relName, relCatEntry.relName, ATTR_SIZE);

    return freeSlot;

}


int OpenRelTable::closeRel(int relId) {
    if (relId == RELCAT_RELID || relId == ATTRCAT_RELID)
        return E_NOTPERMITTED;

    if (relId < 0 || relId >= MAX_OPEN)
        return E_OUTOFBOUND;

    if (OpenRelTable::tableMetaInfo[relId].free)
        return E_RELNOTOPEN;

    OpenRelTable::tableMetaInfo[relId].free = true;
    free(RelCacheTable::relCache[relId]);
    clearList(AttrCacheTable::attrCache[relId]);

    RelCacheTable::relCache[relId] = nullptr;
    AttrCacheTable::attrCache[relId] = nullptr;

    return SUCCESS;
}
