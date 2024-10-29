#include "Schema.h"

#include <cmath>
#include <cstring>


//      Stage 5     :   openRel(char relName[ATTR_SIZE])
                    //  closeRel(char relName[ATTR_SIZE])

//      Stage 6     :   renameRel(char oldRelName[ATTR_SIZE], char newRelName[ATTR_SIZE])
                    //  renameAttr(char *relName, char *oldAttrName, char *newAttrName)

int Schema::renameRel(char oldRelName[ATTR_SIZE], char newRelName[ATTR_SIZE]){
    if(strcmp(oldRelName,RELCAT_RELNAME) == 0 || strcmp(newRelName,RELCAT_RELNAME) == 0 || strcmp(oldRelName,ATTRCAT_RELNAME) == 0 || strcmp(newRelName,ATTRCAT_RELNAME) == 0){
        return E_NOTPERMITTED;
    }

    int relId = OpenRelTable::getRelId(oldRelName);

    if(relId != E_RELNOTOPEN){
        return E_RELOPEN;
    }

    int retVal = BlockAccess::renameRelation(oldRelName,newRelName);

    return retVal;

}

int Schema::renameAttr(char relName[ATTR_SIZE], char oldAttrName[ATTR_SIZE], char newAttrName[ATTR_SIZE]){
    
    if(strcmp(relName,RELCAT_RELNAME) == 0 || strcmp(relName,ATTRCAT_RELNAME) == 0){
        return E_NOTPERMITTED;
    }

    int relId = OpenRelTable::getRelId(relName);

    if(relId != E_RELNOTOPEN){

        return E_RELOPEN;
    }

    int retVal = BlockAccess::renameAttribute(relName,oldAttrName,newAttrName);

    return retVal;

}

int Schema::openRel(char relName[ATTR_SIZE]){
    int ret = OpenRelTable::openRel(relName);
    // the OpenRelTable::openRel() function returns the rel-id if successful
    // a valid rel-id will be within the range 0 <= relId < MAX_OPEN and any
    // error codes will be negative
    if(ret >=0){
        return SUCCESS;
    }

    //otherwise it returns an error message
    return ret;
}

int Schema::closeRel(char relName[ATTR_SIZE]){

    // this function returns the rel-id of a relation if it is open 
    int relId = OpenRelTable::getRelId(relName);

    // or E_RELNOTOPEN if it is not. we will implement this later.
    
    if(relId < 0 || relId >= MAX_OPEN){
        return E_RELNOTOPEN;
    }

    return OpenRelTable::closeRel(relId);
}

int Schema::createRel(char relName[],int nAttrs, char attrs[][ATTR_SIZE],int attrtype[]){

    // declare variable relNameAsAttribute of type Attribute
    // copy the relName into relNameAsAttribute.sVal

    Attribute relNameAsAttribute;
    strcpy(relNameAsAttribute.sVal,relName);

    // declare a variable targetRelId of type RecId

    RecId targetRelId = {-1,-1};

    // Reset the searchIndex using RelCacheTable::resetSearhIndex()
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    // Search the relation catalog (relId given by the constant RELCAT_RELID)
    // for attribute value attribute "RelName" = relNameAsAttribute using
    // BlockAccess::linearSearch() with OP = EQ

    targetRelId = BlockAccess::linearSearch(RELCAT_RELID,RELCAT_ATTR_RELNAME,relNameAsAttribute,EQ);

    // if a relation with name `relName` already exists  ( linearSearch() does
    //                                                     not return {-1,-1} )
    //     return E_RELEXIST;
    if(targetRelId.block != -1 && targetRelId.slot != -1)return E_RELEXIST;

    // compare every pair of attributes of attrNames[] array
    // if any attribute names have same string value,
    //     return E_DUPLICATEATTR (i.e 2 attributes have same value)

    for(int i=0;i<nAttrs;i++){
        for(int j=0;j<nAttrs;j++){
            if(i==j)continue;
            if(strcmp(attrs[i],attrs[j]) == 0)return E_DUPLICATEATTR;
        }
    }

    /* declare relCatRecord of type Attribute which will be used to store the
       record corresponding to the new relation which will be inserted
       into relation catalog */

    Attribute relCatRecord[RELCAT_NO_ATTRS];
    // fill relCatRecord fields as given below
    strcpy(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,attrs[0]);                             // offset RELCAT_REL_NAME_INDEX: relName
    relCatRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal = nAttrs;                                // offset RELCAT_NO_ATTRIBUTES_INDEX: numOfAttributes
    relCatRecord[RELCAT_NO_RECORDS_INDEX].nVal = 0;                                        // offset RELCAT_NO_RECORDS_INDEX: 0
    relCatRecord[RELCAT_FIRST_BLOCK_INDEX].nVal = -1;                                      // offset RELCAT_FIRST_BLOCK_INDEX: -1
    relCatRecord[RELCAT_LAST_BLOCK_INDEX].nVal = -1;                                       // offset RELCAT_LAST_BLOCK_INDEX: -1
    relCatRecord[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal = floor((2016 / (16 * nAttrs + 1)));// offset RELCAT_NO_SLOTS_PER_BLOCK_INDEX: floor((2016 / (16 * nAttrs + 1)))
    // (number of slots is calculated as specified in the physical layer docs)

    // retVal = BlockAccess::insert(RELCAT_RELID(=0), relCatRecord);
    int retVal = BlockAccess::insert(RELCAT_RELID,relCatRecord);
    // if BlockAccess::insert fails return retVal
    // (this call could fail if there is no more space in the relation catalog)

    if(retVal != SUCCESS)return retVal;

    // iterate through 0 to numOfAttributes - 1 :
    for(int i=0;i<nAttrs;i++){
        /* declare Attribute attrCatRecord[6] to store the attribute catalog
           record corresponding to i'th attribute of the argument passed*/
        Attribute attrCatRecord[6];

        // (where i is the iterator of the loop)
        // fill attrCatRecord fields as given below
        strcpy(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,relName);    // offset ATTRCAT_REL_NAME_INDEX: relName
        strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,attrs[i]);  // offset ATTRCAT_ATTR_NAME_INDEX: attrNames[i]
        attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal = attrtype[i];     // offset ATTRCAT_ATTR_TYPE_INDEX: attrTypes[i]
        attrCatRecord[ATTRCAT_PRIMARY_FLAG_INDEX].nVal = -1;           // offset ATTRCAT_PRIMARY_FLAG_INDEX: -1
        attrCatRecord[ATTRCAT_ROOT_BLOCK_INDEX].nVal = -1;             // offset ATTRCAT_ROOT_BLOCK_INDEX: -1
        attrCatRecord[ATTRCAT_OFFSET_INDEX].nVal = i;                  // offset ATTRCAT_OFFSET_INDEX: i

        retVal = BlockAccess::insert(ATTRCAT_RELID, attrCatRecord);
        /* if attribute catalog insert fails:
             delete the relation by calling deleteRel(targetrel) of schema layer
             return E_DISKFULL
             // (this is necessary because we had already created the
             //  relation catalog entry which needs to be removed)
        */
       if(retVal !=SUCCESS)return retVal;

    }
    return SUCCESS;
}