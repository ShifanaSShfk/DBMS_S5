#include "BlockAccess.h"
#include <cstring>

//      Stage 4     :   linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op)

//      Stage 6     :   renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE])
                    //  renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE])

RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op) {
    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId, &prevRecId);
    int block, slot;
    if (prevRecId.block == -1 && prevRecId.slot == -1) {
        RelCatEntry relCatBuf;
        RelCacheTable::getRelCatEntry(relId, &relCatBuf);

        block = relCatBuf.firstBlk;
        slot = 0;
    }
    else {
        block = prevRecId.block;
        slot = prevRecId.slot + 1;
    }
    while (block != -1) {
        RecBuffer recBuffer(block);

        HeadInfo header;
        recBuffer.getHeader(&header);
        
        Attribute record[header.numAttrs];
        recBuffer.getRecord(record, slot);
        
        unsigned char slotMap[header.numSlots];
        recBuffer.getSlotMap(slotMap);


        if (slot >= header.numSlots) {
            block = header.rblock;
            slot = 0;
            continue;
        }

        if (slotMap[slot] == SLOT_UNOCCUPIED) {
            slot++;
            continue;
        }

        AttrCatEntry attrCatBuf;
        int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatBuf);

        int cmpVal = compareAttrs(record[attrCatBuf.offset], attrVal, attrCatBuf.attrType);

        if (
            (op == NE && cmpVal != 0) ||    // if op is "not equal to"
            (op == LT && cmpVal < 0) ||     // if op is "less than"
            (op == LE && cmpVal <= 0) ||    // if op is "less than or equal to"
            (op == EQ && cmpVal == 0) ||    // if op is "equal to"
            (op == GT && cmpVal > 0) ||     // if op is "greater than"
            (op == GE && cmpVal >= 0)       // if op is "greater than or equal to"
        ) {
            RecId searchIndex = {block, slot};
            RelCacheTable::setSearchIndex(relId, &searchIndex);
            return searchIndex;
        }
        
        slot++;
    }

    return RecId({-1, -1});
}

int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE]) {
    // Reset the search index of the relation catalog using RelCacheTable::resetSearchIndex()
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    // Set newRelationName with newName
    Attribute newRelationName;
    memcpy(newRelationName.sVal, newName, ATTR_SIZE);

    // Set oldRelationName with oldName
    Attribute oldRelationName;
    memcpy(oldRelationName.sVal, oldName, ATTR_SIZE);

    // Search the relation catalog for an entry with "RelName" = newRelationName
    RecId recId = BlockAccess::linearSearch(RELCAT_RELID, (char*)RELCAT_ATTR_RELNAME, newRelationName, EQ);

    // If a relation with name newName already exists (result of linearSearch is not {-1, -1}), return E_RELEXIST
    if (recId.block != -1 || recId.slot != -1)
        return E_RELEXIST;
         // Reset the search index of the relation catalog using RelCacheTable::resetSearchIndex()
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    // Search the relation catalog for an entry with "RelName" = oldRelationName
    recId = BlockAccess::linearSearch(RELCAT_RELID, (char*)RELCAT_ATTR_RELNAME, oldRelationName, EQ);

    // If relation with name oldName does not exist (result of linearSearch is {-1, -1}), return E_RELNOTEXIST
    if (recId.block == -1 && recId.slot == -1)
        return E_RELNOTEXIST;

    // Get the relation catalog record of the relation to rename using a RecBuffer on the relation catalog [RELCAT_BLOCK] 
    // and RecBuffer.getRecord function
    RecBuffer recBuffer(recId.block);

    Attribute record[RELCAT_NO_ATTRS];
    recBuffer.getRecord(record, recId.slot);

    // Update the relation name attribute in the record with newName (use RELCAT_REL_NAME_INDEX)
    memcpy(&record[RELCAT_REL_NAME_INDEX], &newRelationName, ATTR_SIZE);

    // Set back the record value using RecBuffer.setRecord
    recBuffer.setRecord(record, recId.slot);

    // Update all the attribute catalog entries in the attribute catalog corresponding
    // to the relation with relation name oldName to the relation name newName
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    while (true) {
        // LinearSearch on the attribute catalog for relName = oldRelationName
        RecId attrEntryId = BlockAccess::linearSearch(ATTRCAT_RELID, (char*)ATTRCAT_ATTR_RELNAME, oldRelationName, EQ);

        // If there are no more attributes left to check (linearSearch returned {-1, -1}), break the loop
        if (attrEntryId.block == -1 && attrEntryId.slot == -1)
            break;

        // Get the record using RecBuffer.getRecord
        RecBuffer attrCatRecBuffer(attrEntryId.block);
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatRecBuffer.getRecord(attrCatRecord, attrEntryId.slot);

        // Update the relName field in the record to newName
        memcpy(&attrCatRecord[ATTRCAT_REL_NAME_INDEX], &newRelationName, ATTR_SIZE);

        // Set back the record using RecBuffer.setRecord
        attrCatRecBuffer.setRecord(attrCatRecord, attrEntryId.slot);
    }

    // Return success status
    return SUCCESS;
}

int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE]) {

    // Reset the search index of the relation catalog using RelCacheTable::resetSearchIndex()
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    // Set relNameAttr to relName
    Attribute relNameAttr;
    memcpy(relNameAttr.sVal, relName, ATTR_SIZE);

    // Search for the relation with name relName in the relation catalog using linearSearch()
    RecId recId = BlockAccess::linearSearch(RELCAT_RELID, (char*)RELCAT_ATTR_RELNAME, relNameAttr, EQ);

    // If the relation with name relName does not exist (search returns {-1,-1}), return E_RELNOTEXIST
    if (recId.block == -1 && recId.slot == -1)
        return E_RELNOTEXIST;

    // Reset the search index of the attribute catalog using RelCacheTable::resetSearchIndex()
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    // Declare variable attrToRenameRecId used to store the attr-cat recId of the attribute to rename
    RecId attrToRenameId = {-1, -1};
// Iterate over all Attribute Catalog Entry records corresponding to the relation to find the required attribute
    while (true) {
        // Linear search on the attribute catalog for RelName = relNameAttr
        RecId attrRecId = BlockAccess::linearSearch(ATTRCAT_RELID, (char*)ATTRCAT_ATTR_RELNAME, relNameAttr, EQ);

        // If there are no more attributes left to check (linearSearch returned {-1,-1}), break
        if (attrRecId.block == -1 && attrRecId.slot == -1)
            break;

        // Get the record from the attribute catalog using RecBuffer.getRecord into attrCatEntryRecord
        RecBuffer recBuffer(attrRecId.block);
        Attribute record[ATTRCAT_NO_ATTRS];
        recBuffer.getRecord(record, attrRecId.slot);

        // Extract the attribute name from the record
        char attrName[ATTR_SIZE];
        memcpy(attrName, record[ATTRCAT_ATTR_NAME_INDEX].sVal, ATTR_SIZE);

        // If attrCatEntryRecord.attrName = oldName, store its recId for renaming
        if (strcmp(attrName, oldName) == 0)
            attrToRenameId = attrRecId;

        // If attrCatEntryRecord.attrName = newName, return E_ATTREXIST as the new name already exists
        if (strcmp(attrName, newName) == 0)
            return E_ATTREXIST;
    }
// If no attribute with the old name was found, return E_ATTRNOTEXIST
    if (attrToRenameId.block == -1 && attrToRenameId.slot == -1)
        return E_ATTRNOTEXIST;

    // Update the entry corresponding to the attribute in the Attribute Catalog Relation.
    // Declare a RecBuffer for attrToRenameRecId.block and get the record at attrToRenameRecId.slot
    RecBuffer bufferToRename(attrToRenameId.block);
    Attribute recordToRename[ATTRCAT_NO_ATTRS];
    bufferToRename.getRecord(recordToRename, attrToRenameId.slot);

    // Update the AttrName of the record with newName
    memcpy(recordToRename[ATTRCAT_ATTR_NAME_INDEX].sVal, newName, ATTR_SIZE);

    // Set back the updated record using RecBuffer.setRecord
    bufferToRename.setRecord(recordToRename, attrToRenameId.slot);

    // Return success status
    return SUCCESS;
}