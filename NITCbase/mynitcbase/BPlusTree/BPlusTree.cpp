// #include "BPlusTree.h"

// #include <cstring>
// #include <cstdio>
// #include <iostream>

// /*
//  * @brief 
//  * searches the relation specified to find the next record that satisfies the specified condition
//  * 
//  * The condition value is given by the argument attrVal. 
//  * This function returns the recId of the next record satisfying the condition. 
//  * The condition that is checked for is the following.
//  * 
//  * @param relId Relation Id of the relation containing the attribute with index
//  * @param attrName Attribute/column name (which has an index) to which condition need to be checked with.
//  * @param attrVal value of attribute that has to be checked against the operater
//  * @param op Conditional Operator ( can be one among EQ , LE , LT , GE , GT , NE )
//  * @return
//  * Returns the block number and slot number of the record corresponding to the next hit. 
//  * Returns {-1,-1} if no next hit.
// */ 
// RecId BPlusTree::bPlusSearch(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int op) {
//   // declare searchIndex which will be used to store search index for attrName.
//   IndexId searchIndex;

//   //get the search index corresponding to attribute with name attrName
//   AttrCacheTable::getSearchIndex(relId, attrName, &searchIndex);

//   AttrCatEntry attrCatEntry;
//   AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

//   // declare variables block and index which will be used during search
//   int block, index;

//   // search is done for the first time
//   if ( searchIndex.block == -1 || searchIndex.index == -1 ) {

//     // start the search from the first entry of root.
//     block = attrCatEntry.rootBlock;
//     index = 0;

//     if ( block == -1 ) return RecId{-1, -1};
//   }
//   // Not first time
//   else {
//     /*a valid searchIndex points to an entry in the leaf index of the attribute's
//     B+ Tree which had previously satisfied the op for the given attrVal.*/

//     block = searchIndex.block;
//     index = searchIndex.index + 1;  // search is resumed from the next index.

//     // load block into leaf using IndLeaf::IndLeaf().
//     IndLeaf leaf(block);

//     // declare leafHead which will be used to hold the header of leaf.
//     HeadInfo leafHead;
//     leaf.getHeader(&leafHead);

//     /* 
//       All the entries in the block has been searched; 
//       search from the beginning of the next leaf index block. 
//     */
//     if (index >= leafHead.numEntries) {

//       // update block to rblock of current block and index to 0.
//       block = leafHead.rblock;
//       index = 0;
//       // end of linked list reached - the search is done.
//       if (block == -1) return RecId{-1, -1};
//     }
//   }

//   /* 
//     Traverse down through all the internal nodes according to value of attrVal and the operator op 
//   */

//   /* (This section is only needed when
//       - search restarts from the root block (when searchIndex is reset by caller)
//       - root is not a leaf
//       If there was a valid search index, then we are already at a leaf block
//       and the test condition in the following loop will fail)
//   */

//   while(StaticBuffer::getStaticBlockType(block) == IND_INTERNAL ) {

//     // load the block into internalBlk using IndInternal::IndInternal().
//     IndInternal internalBlk(block);

//     HeadInfo intHead;
//     internalBlk.getHeader(&intHead);

//     // declare intEntry which will be used to store an entry of internalBlk.
//     InternalEntry intEntry;

//     if ( op == NE || op == LE || op == LT ) {
//       /*
//       - NE: need to search the entire linked list of leaf indices of the B+ Tree,
//       starting from the leftmost leaf index. Thus, always move to the left.

//       - LT and LE: the attribute values are arranged in ascending order in the
//       leaf indices of the B+ Tree. Values that satisfy these conditions, if
//       any exist, will always be found in the left-most leaf index. Thus,
//       always move to the left.
//       */

//       // load entry in the first slot of the block into intEntry
//       internalBlk.getEntry(&intEntry, 0);
//       block = intEntry.lChild;

//     } 
//     else {
//       /*
//       - EQ, GT and GE: move to the left child of the first entry that is
//       greater than (or equal to) attrVal
//       (we are trying to find the first entry that satisfies the condition.
//       since the values are in ascending order we move to the left child which
//       might contain more entries that satisfy the condition)
//       */

//       /*
//         traverse through all entries of internalBlk and find an entry that
//         satisfies the condition.
//         if op == EQ or GE, then intEntry.attrVal >= attrVal
//         if op == GT, then intEntry.attrVal > attrVal
//         Hint: the helper function compareAttrs() can be used for comparing
//       */
//       int ind = 0;
//       while ( ind < intHead.numEntries ){

//         internalBlk.getEntry(&intEntry, ind);
//         int ret = compareAttrs(intEntry.attrVal, attrVal, attrCatEntry.attrType );
//         if (((op==EQ || op==GE) && ret >= 0)|| (op==GT && ret > 0)) break;

//         ind++;
//       }
//       // such an entry is found - move to the left child of that entry
//       if ( ind != intHead.numEntries ) {
//         block = intEntry.lChild;
//       } 
//       // such an entry is not found - move to the right child of the last entry of the block
//       else{
//         internalBlk.getEntry(&intEntry, intHead.numEntries - 1);
//         block = intEntry.rChild;
//       }
//     }
//   }

//   // NOTE: `block` now has the block number of a leaf index block.

//   /******  Identify the first leaf index entry from the current position
//               that satisfies our condition (moving right)             ******/

//   while (block != -1) {
//     // load the block into leafBlk using IndLeaf::IndLeaf().
//     IndLeaf leafBlk(block);
//     HeadInfo leafHead;
//     leafBlk.getHeader(&leafHead);

//     // declare leafEntry which will be used to store an entry from leafBlk
//     Index leafEntry;
//     while ( index < leafHead.numEntries ) {
//       leafBlk.getEntry(&leafEntry, index);

//       // int cmpVal = /* comparison between leafEntry's attribute value
//       //                 and input attrVal using compareAttrs()*/
//       int cmpVal = compareAttrs(leafEntry.attrVal, attrVal, attrCatEntry.attrType);

//       // (entry satisfying the condition found)
//       if (
//         (op == EQ && cmpVal == 0) ||
//         (op == LE && cmpVal <= 0) ||
//         (op == LT && cmpVal <  0) ||
//         (op == GT && cmpVal >  0) ||
//         (op == GE && cmpVal >= 0) ||
//         (op == NE && cmpVal != 0)
//       ){
//         searchIndex.block = block;
//         searchIndex.index = index;
//         AttrCacheTable::setSearchIndex(relId, attrName, &searchIndex);
//         return RecId{leafEntry.block, leafEntry.slot};
//       } 
//       /*future entries will not satisfy EQ, LE, LT since the values
//           are arranged in ascending order in the leaves */
//       else if ((op == EQ || op == LE || op == LT) && cmpVal > 0) {
//         return RecId{-1, -1};
//       }
//       // search next index.
//       ++index;
//     }

//     /*only for NE operation do we have to check the entire linked list;
//     for all the other op it is guaranteed that the block being searched
//     will have an entry, if it exists, satisying that op. */
//     if (op != NE) break;

//     // For NE operation
//     block = leafHead.rblock;
//     index = 0;
//   }

//   // no entry satisying the op was found; return the recId {-1,-1}
//   return RecId {-1,-1};
// }

// // 
// // Used to initialize the rootBlock and call Insert for each record present in the relation
// // Iteratively inserts each record in the BPlusTree starting from relCache firstBlock

// int BPlusTree::bPlusCreate(int relId, char attrName[ATTR_SIZE]) {

//   int ret;
//   // no index allowed for relation and attribute cat
//   if ( relId == RELCAT_RELID || relId == ATTRCAT_RELID ) return E_NOTPERMITTED;

//   // Fetch attribute cat entry
//   AttrCatEntry attrCatBuf;
//   ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatBuf);
//   if ( ret != SUCCESS ) return ret;

//   // Index already exists
//   if ( attrCatBuf.rootBlock != -1 ) {
//     return SUCCESS;
//   }

//   /******Creating a new B+ Tree ******/

//   // get a free leaf block using constructor 1 to allocate a new block
//   IndLeaf rootBlockBuf;

//   // (if the block could not be allocated, the appropriate error code
//   //  will be stored in the blockNum member field of the object)

//   // declare rootBlock to store the blockNumber of the new leaf block
//   int rootBlock = rootBlockBuf.getBlockNum();

//   // if there is no more disk space for creating an index
//   if (rootBlock == E_DISKFULL) {
//       return E_DISKFULL;
//   }

//   // get the first block of records from relcache
//   RelCatEntry relCatEntry;
//   RelCacheTable::getRelCatEntry(relId, &relCatEntry);
//   int block = relCatEntry.firstBlk;

//   // update attrCatBuf.rootBlock 
//   attrCatBuf.rootBlock = rootBlock;
//   // set the attrCatEntry using AttrCacheTable::setAttrCatEntry()
//   ret = AttrCacheTable::setAttrCatEntry(relId, attrName, &attrCatBuf);


//   /** Traverse all the blocks in the relation and insert them one by one into the B+ Tree **/
//   while (block != -1) {
//     RecBuffer recBuf(block);

//     unsigned char slotMap[relCatEntry.numSlotsPerBlk];
//     recBuf.getSlotMap(slotMap);

//     for (int slot=0;slot<relCatEntry.numSlotsPerBlk;slot++)
//       if ( slotMap[slot] == SLOT_OCCUPIED ){

//         Attribute record[relCatEntry.numAttrs];
//         recBuf.getRecord(record, slot);

//         RecId recId{ block, slot};
//         // printf("%d %d\n", block, slot);
//         // insert the attribute value corresponding to attrName from the record into the B+ tree
//         ret = BPlusTree::bPlusInsert(relId, attrName, record[attrCatBuf.offset], recId );

//         // (note that bPlusInsert will destroy any existing bplus tree if
//         // insert fails i.e when disk is full)
//         // (unable to get enough blocks to build the B+ Tree.)
//         if (ret == E_DISKFULL) return E_DISKFULL;
//       }

//     // Traversing to next block    
//     HeadInfo head;
//     recBuf.getHeader(&head);
//     block = head.rblock;
//   }

//   return SUCCESS;
// }

// // Recursively release blocks till leaf nodes.

// int BPlusTree::bPlusDestroy(int rootBlockNum) {
//   if ( rootBlockNum < 0 || rootBlockNum >= DISK_BLOCKS ) return E_OUTOFBOUND;

//   int type = StaticBuffer::getStaticBlockType(rootBlockNum);

//   if (type == IND_LEAF) {
//     IndLeaf leafBuffer(rootBlockNum);
//     leafBuffer.releaseBlock();
//     return SUCCESS;

//   } 
//   else if (type == IND_INTERNAL) {
//     IndInternal internalBuffer(rootBlockNum);
    
//     HeadInfo intBlockHeader;
//     internalBuffer.getHeader(&intBlockHeader);

//     // Recursively delete all nodes 
//     /*iterate through all the entries of the internalBlk and destroy the lChild
//     of the first entry and rChild of all entries using BPlusTree::bPlusDestroy().
//     (the rchild of an entry is the same as the lchild of the next entry.
//       take care not to delete overlapping children more than once ) */

//     // Deleting left child of first entry
//     InternalEntry tempEntry;
//     internalBuffer.getEntry(&tempEntry, 0);
//     BPlusTree::bPlusDestroy(tempEntry.lChild);  

//     // Iterating and deleting right child of every entry
//     for ( int i=0 ; i<intBlockHeader.numEntries; i++ ){
//       InternalEntry tempEntry;
//       internalBuffer.getEntry(&tempEntry, i);
//       BPlusTree::bPlusDestroy(tempEntry.rChild);  
//     }

//     // Release this internal node
//     internalBuffer.releaseBlock();
//     return SUCCESS;

//   } 
//   else {
//     // block is not an index block
//     return E_INVALIDBLOCK;
//   }
// }

// //  Finds the leaf to insert and tries to insert if rootBlock exists

// int BPlusTree::bPlusInsert(int relId, char attrName[ATTR_SIZE], Attribute attrVal, RecId recId) {
//   // get the attribute cache entry corresponding to attrName
//   int ret;
//   AttrCatEntry attrCatBuf;
//   ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatBuf);
//   if ( ret != SUCCESS ) return ret;

//   /* rootBlock of B+ Tree (from attrCatEntry) */
//   int rootBlockNum = attrCatBuf.rootBlock;

//   // No root block has been assigned
//   if ( rootBlockNum == -1 ) return E_NOINDEX;

//   // find the leaf block to which insertion is to be done
//   int leafBlkNum = BPlusTree::findLeafToInsert(rootBlockNum, attrVal, attrCatBuf.attrType);

//   Index entry;
//   entry.attrVal = attrVal;
//   entry.block = recId.block;
//   entry.slot = recId.slot;

//   // insert the attrVal and recId to the leaf block at blockNum using the
//   // insertIntoLeaf() function.
//   // NOTE: the insertIntoLeaf() function will propagate the insertion to the
//   //       required internal nodes by calling the required helper functions
//   //       like insertIntoInternal() or createNewRoot()
//   ret = BPlusTree::insertIntoLeaf(relId, attrName, leafBlkNum, entry);

  
//   if ( ret == E_DISKFULL ) {
//     // destroy the existing B+ tree by passing the rootBlock to bPlusDestroy().
//     BPlusTree::bPlusDestroy(rootBlockNum);

//     // update root block to show index does not exist in attr cache
//     attrCatBuf.rootBlock = -1;
//     AttrCacheTable::setAttrCatEntry(relId, attrName, &attrCatBuf);

//     return E_DISKFULL;
//   }

//   return SUCCESS;
// }

// //  Traverse down following conditions from rootBlock until block type = LEAF 
// //  Block Number of Leaf
// int BPlusTree::findLeafToInsert(int rootBlock, Attribute attrVal, int attrType) {
//   int blockNum = rootBlock;
//   int type = StaticBuffer::getStaticBlockType(blockNum);

//   // Iterate through internal nodes 
//   while ( type != IND_LEAF ) {
//     IndInternal blockInternal(blockNum);

//     HeadInfo header;
//     blockInternal.getHeader(&header);

//     /* iterate through all the entries, to find the first entry whose
//           attribute value >= value to be inserted.  */
//     int i;
//     for ( i = 0; i < header.numEntries; i++ ){
//       InternalEntry entry;
//       blockInternal.getEntry(&entry, i);
//       if ( compareAttrs(entry.attrVal, attrVal, attrType) >= 0 ) break;
//     }

//     // Rightmost block
//     if ( i == header.numEntries ) {
//       InternalEntry entry;
//       blockInternal.getEntry(&entry, header.numEntries-1);
//       blockNum = entry.rChild;
//     } 
//     // Left child of block that satisfied
//     else {
//       InternalEntry entry;
//       blockInternal.getEntry(&entry, i);
//       blockNum = entry.lChild;
//     }

//     type = StaticBuffer::getStaticBlockType(blockNum);

//   }

//   return blockNum;
// }

// // Rearranges indices to fit new entry. If no space splitsLeaf and propagates up.

// int BPlusTree::insertIntoLeaf(int relId, char attrName[ATTR_SIZE], int blockNum, Index indexEntry) {

//   int ret;
//   AttrCatEntry attrCatBuf;
//   AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatBuf);

//   // declare an IndLeaf instance for the blockNum found
//   IndLeaf leafBlk(blockNum);
//   HeadInfo blockHeader;
//   leafBlk.getHeader(&blockHeader);

//   // the following variable will be used to store a list of index entries with
//   // existing indices + the new index to insert
//   Index indices[blockHeader.numEntries + 1];

//   /*
//   Iterate through all the entries in the block and copy them to the array indices.
//   Also insert `indexEntry` at appropriate position in the indices array maintaining
//   the ascending order.
//   - use IndLeaf::getEntry() to get the entry
//   - use compareAttrs() declared in BlockBuffer.h to compare two Attribute structs
//   */
//   bool inserted = false;
//   int j = 0;
//   for ( int i=0;i<blockHeader.numEntries; i++ ){
//     Index tempEntry;
//     leafBlk.getEntry(&tempEntry, i);

//     if ( compareAttrs(indexEntry.attrVal, tempEntry.attrVal, attrCatBuf.attrType) >= 0 ){
//       indices[j++] = tempEntry;
//     }
//     else if ( !inserted ){
//       indices[j++] = indexEntry;
//       indices[j++] = tempEntry;
//       inserted = true;
//     }
//     else indices[j++] = tempEntry;
//   }
//   if ( !inserted ) indices[ blockHeader.numEntries ] = indexEntry;


//   // leaf block has not reached max limit
//   if (blockHeader.numEntries != MAX_KEYS_LEAF) {
//     blockHeader.numEntries++;
//     leafBlk.setHeader(&blockHeader);

//     // iterate through all the entries of the array `indices` and populate the
//     // entries of block with them using IndLeaf::setEntry().
//     for (int i=0;i<blockHeader.numEntries;i++){
//       leafBlk.setEntry(&indices[i], i);
//     }
//     return SUCCESS;
//   }

//   // If we reached here, the `indices` array has more than entries than can fit
//   // in a single leaf index block. Therefore, we will need to split the entries
//   // in `indices` between two leaf blocks. We do this using the splitLeaf() function.
//   // This function will return the blockNum of the newly allocated block or
//   // E_DISKFULL if there are no more blocks to be allocated.

//   int newRightBlk = splitLeaf(blockNum, indices);
//   if ( newRightBlk == E_DISKFULL ) return E_DISKFULL;

//   //the current leaf block was not the root
//   if ( blockHeader.pblock != -1 ) {

//     // insert the middle value from `indices` into the parent block using the
//     // insertIntoInternal() function. (i.e the last value of the left block)
//     InternalEntry tempEntry;
//     tempEntry.attrVal = indices[MIDDLE_INDEX_LEAF].attrVal;
//     tempEntry.lChild = blockNum;
//     tempEntry.rChild = newRightBlk;

//     // the middle value will be at index 31 (given by constant MIDDLE_INDEX_LEAF)
//     ret = insertIntoInternal(relId, attrName, blockHeader.pblock, tempEntry);
//     if ( ret != SUCCESS ) return ret;

//   } 
//   // the current block was the root block and is now split. a new internal index
//   // block needs to be allocated and made the root of the tree.
//   else {

//     ret = createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal, blockNum, newRightBlk);
//     if ( ret != SUCCESS ) return ret;

//   }
//   return SUCCESS;
// }

// //  Allocate new Right Block and set indices in both left and right leaf blocks
// //  Right Block Number
// int BPlusTree::splitLeaf(int leafBlockNum, Index indices[]) {

//   IndLeaf rightBlk;

//   IndLeaf leftBlk(leafBlockNum);

//   int rightBlkNum = rightBlk.getBlockNum();
//   int leftBlkNum = leafBlockNum;

//   //(failed to obtain a new leaf index block because the disk is full)
//   if ( rightBlkNum == E_DISKFULL ) {
//     return E_DISKFULL;
//   }

//   HeadInfo leftBlkHeader, rightBlkHeader;
//   rightBlk.getHeader(&rightBlkHeader);
//   leftBlk.getHeader(&leftBlkHeader);

//   // set rightBlkHeader with the following values
//   rightBlkHeader.numEntries = (MAX_KEYS_LEAF+1)/2; // (MAX_KEYS_LEAF+1)/2 = 32
//   rightBlkHeader.pblock = leftBlkHeader.pblock;
//   rightBlkHeader.lblock = leftBlkNum;
//   rightBlkHeader.rblock = leftBlkHeader.rblock;
//   rightBlk.setHeader(&rightBlkHeader);

//   // set leftBlkHeader with the following values
//   leftBlkHeader.numEntries = (MAX_KEYS_LEAF+1)/2; // (MAX_KEYS_LEAF+1)/2 = 32
//   leftBlkHeader.rblock = rightBlkNum;
//   leftBlk.setHeader(&leftBlkHeader);

//   for (int i=0; i<(MAX_KEYS_LEAF+1)/2; i++){
//     leftBlk.setEntry( &indices[i], i );
//   }

//   for (int i = (MAX_KEYS_LEAF+1)/2; i <= MAX_KEYS_LEAF; i++){
//     rightBlk.setEntry( &indices[i], i - (MAX_KEYS_LEAF+1)/2 );
//   }

//   return rightBlkNum;
// }

// //  Rearranges indices to fit new entry. If no space splitsInternal and propagates up
// int BPlusTree::insertIntoInternal(  int relId, 
//                                     char attrName[ATTR_SIZE], 
//                                     int intBlockNum, 
//                                     InternalEntry intEntry ) {

//   int ret;
//   AttrCatEntry attrCatBuf;
//   AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatBuf);

//   IndInternal blockInt(intBlockNum);
//   HeadInfo blockHeader;
//   blockInt.getHeader(&blockHeader);

//   // declare internalEntries to store all existing entries + the new entry
//   InternalEntry internalEntries[blockHeader.numEntries + 1];

//   /*
//   Iterate through all the entries in the block and copy them to the array
//   `internalEntries`. Insert `indexEntry` at appropriate position in the
//   array maintaining the ascending order.
//       - use IndInternal::getEntry() to get the entry
//       - use compareAttrs() to compare two structs of type Attribute
  

//   Update the lChild of the internalEntry immediately following the newly added
//   entry to the rChild of the newly added entry.
//   */

//   bool inserted = false;
//   int j = 0;
//   for ( int i=0;i<blockHeader.numEntries; i++ ){
//     InternalEntry tempEntry;
//     blockInt.getEntry(&tempEntry, i);

//     if ( compareAttrs(intEntry.attrVal, tempEntry.attrVal, attrCatBuf.attrType) >= 0 ){
//       internalEntries[j++] = tempEntry;
//     }
//     else if ( !inserted ){
//       tempEntry.lChild = intEntry.rChild;
//       internalEntries[j++] = intEntry;
//       internalEntries[j++] = tempEntry;
//       inserted = true;
//     }
//     else internalEntries[j++] = tempEntry;
//   }
//   if ( !inserted ) internalEntries[ blockHeader.numEntries ] = intEntry;





//   // (internal index block has not reached max limit)
//   if (blockHeader.numEntries != MAX_KEYS_INTERNAL) {
//     blockHeader.numEntries++;
//     blockInt.setHeader(&blockHeader);

//     // iterate through all entries in internalEntries array and populate the
//     // entries of intBlk with them using IndInternal::setEntry().
//     for (int i=0;i<blockHeader.numEntries;i++){
//       blockInt.setEntry(&internalEntries[i], i);
//     }
//     return SUCCESS;
//   }

//   // If we reached here, the `internalEntries` array has more than entries than
//   // can fit in a single internal index block. Therefore, we will need to split
//   // the entries in `internalEntries` between two internal index blocks. We do
//   // this using the splitInternal() function.
//   // This function will return the blockNum of the newly allocated block or
//   // E_DISKFULL if there are no more blocks to be allocated.

//   int newRightBlk = splitInternal(intBlockNum, internalEntries);

//   if ( newRightBlk == E_DISKFULL ) {
//     // Using bPlusDestroy(), destroy the right subtree, rooted at intEntry.rChild.
//     // This corresponds to the tree built up till now that has not yet been
//     // connected to the existing B+ Tree
//     bPlusDestroy(intEntry.rChild);
//     return E_DISKFULL;
//   }

//   if ( blockHeader.pblock != -1 ) {

//     // insert the middle value from `internalEntries` into the parent block
//     // using the insertIntoInternal() function (recursively).
//     InternalEntry internEntry;
//     internEntry.attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal;
//     internEntry.lChild = intBlockNum;
//     internEntry.rChild = newRightBlk;

//     ret = insertIntoInternal(relId, attrName, blockHeader.pblock , internEntry);
//     if ( ret != SUCCESS ) return ret;

//   } 
//   else {
//     // the current block was the root block and is now split. a new internal index
//     // block needs to be allocated and made the root of the tree.
//     // To do this, call the createNewRoot() function with the following arguments
//     ret = createNewRoot(relId, attrName,
//                   internalEntries[MIDDLE_INDEX_INTERNAL].attrVal,
//                   intBlockNum, newRightBlk);
//     if ( ret != SUCCESS ) return ret;
//   }
//   return SUCCESS;
// }

// //   Allocate new Right Block and set entries in both left and right leaf blocks
// //   Right Block Number
// int BPlusTree::splitInternal(int intBlockNum, InternalEntry internalEntries[]) {
//   IndInternal rightBlk;
//   IndInternal leftBlk(intBlockNum);

//   int rightBlkNum = rightBlk.getBlockNum();
//   int leftBlkNum = intBlockNum;

//   //(failed to obtain a new internal index block because the disk is full)
//   if ( rightBlkNum == E_DISKFULL ) {
//     return E_DISKFULL;
//   }

//   // get the headers of left block and right block
//   HeadInfo leftBlkHeader, rightBlkHeader;
//   rightBlk.getHeader(&rightBlkHeader);
//   leftBlk.getHeader(&leftBlkHeader);

//   rightBlkHeader.numEntries = (MAX_KEYS_INTERNAL)/2; // (MAX_KEYS_INTERNAL)/2 = 50
//   rightBlkHeader.pblock = leftBlkHeader.pblock;
//   rightBlk.setHeader(&rightBlkHeader);

//   leftBlkHeader.numEntries = (MAX_KEYS_INTERNAL)/2; // (MAX_KEYS_INTERNAL)/2 = 50
//   leftBlk.setHeader(&leftBlkHeader);

//   /*
//   - set the first 50 entries of leftBlk = index 0 to 49 of internalEntries
//     array
//   - set the first 50 entries of newRightBlk = entries from index 51 to 100
//     of internalEntries array using IndInternal::setEntry().
//     (index 50 will be moving to the parent internal index block)
//   */
//   for (int i = 0; i < (MAX_KEYS_INTERNAL)/2; i++ ){
//     leftBlk.setEntry(&internalEntries[i], i);
//     rightBlk.setEntry(&internalEntries[i + (MAX_KEYS_INTERNAL)/2 + 1], i);
//   }
//   int type = StaticBuffer::getStaticBlockType(internalEntries[0].rChild);

//   // Reassigning parents from left Child to Right Child for the entries that got shifted
//   for ( int i = (MAX_KEYS_INTERNAL)/2; i <= MAX_KEYS_INTERNAL; i++ ) {
//     BlockBuffer child(internalEntries[i].rChild);

//     HeadInfo childheader;
//     child.getHeader(&childheader);
//     childheader.pblock = rightBlkNum;
//     child.setHeader(&childheader);
//   }

//   return rightBlkNum;
// }

// //
// /// Initializes new rootBlock of type Internal Node. Sets up LChild, RChild pointers and entry.
// /// Also sets up the parent pointers for LChild and RChild. Reassigns rootBlock in attrCache.

// //   Status Code
// int BPlusTree::createNewRoot( int relId, 
//                               char attrName[ATTR_SIZE], 
//                               Attribute attrVal, 
//                               int lChild, 
//                               int rChild ) {

//   int ret;
//   AttrCatEntry attrCatBuf;
//   AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatBuf);

//   IndInternal newRootBlk;

//   int newRootBlkNum = newRootBlk.getBlockNum();

//   // (failed to obtain an empty internal index block because the disk is full)
//   if (newRootBlkNum == E_DISKFULL) {
//     // Using bPlusDestroy(), destroy the right subtree, rooted at rChild.
//     // This corresponds to the tree built up till now that has not yet been
//     // connected to the existing B+ Tree
//     BPlusTree::bPlusDestroy(rChild);
//     return E_DISKFULL;
//   }

//   // update the header of the new block with numEntries = 1 using
//   // BlockBuffer::getHeader() and BlockBuffer::setHeader()
//   HeadInfo newBlkHeader;
//   newRootBlk.getHeader(&newBlkHeader);
//   newBlkHeader.numEntries = 1;
//   newRootBlk.setHeader(&newBlkHeader);

//   InternalEntry internalEntry;
//   internalEntry.lChild = lChild;
//   internalEntry.rChild = rChild;
//   internalEntry.attrVal = attrVal;
//   newRootBlk.setEntry(&internalEntry, 0);

//   BlockBuffer lChildBlk(lChild);
//   BlockBuffer rChildBlk(rChild);

//   HeadInfo lChildHeader, rChildHeader;

//   lChildBlk.getHeader(&lChildHeader);
//   lChildHeader.pblock = newRootBlkNum;
//   lChildBlk.setHeader(&lChildHeader);

//   rChildBlk.getHeader(&rChildHeader);
//   rChildHeader.pblock = newRootBlkNum;
//   rChildBlk.setHeader(&rChildHeader);

//   attrCatBuf.rootBlock = newRootBlkNum;
//   AttrCacheTable::setAttrCatEntry(relId, attrName, &attrCatBuf);

//   return SUCCESS;
// }




//---------------------------------------
//  Stage 10    :  bPlusSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op)

#include "BPlusTree.h"
#include <iostream>
#include <cstring>

void printVal(Attribute attrVal,int type){
    if(type == NUMBER)std::cout<<attrVal.nVal<<"\n";
    else std::cout<<attrVal.sVal<<"\n";
}

RecId BPlusTree::bPlusSearch(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int op) {

    // declare searchIndex which will be used to store search index for attrName.
    IndexId searchIndex;

    /* get the search index corresponding to attribute with name attrName
       using AttrCacheTable::getSearchIndex(). */

    AttrCacheTable::getSearchIndex(relId,attrName,&searchIndex);


    
    AttrCatEntry attrCatEntry;
    /* load the attribute cache entry into attrCatEntry using
     AttrCacheTable::getAttrCatEntry(). */

    AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);

    int type = attrCatEntry.attrType;

    // declare variables block and index which will be used during search
    int block, index;

    if (searchIndex.block == -1 && searchIndex.index == -1) {
        // (search is done for the first time)

        // start the search from the first entry of root.
        block = attrCatEntry.rootBlock;
        index = 0;

        /* attrName doesn't have a B+ tree (block == -1)*/
        if (block == -1) {
            return RecId{-1, -1};
        }

    } else {    
        /*a valid searchIndex points to an entry in the leaf index of the attribute's
        B+ Tree which had previously satisfied the op for the given attrVal.*/

        block = searchIndex.block;
        index = searchIndex.index + 1;  // search is resumed from the next index.

        // load block into leaf using IndLeaf::IndLeaf().
        IndLeaf leaf(block);

        // declare leafHead which will be used to hold the header of leaf.
        HeadInfo leafHead;

        // load header into leafHead using BlockBuffer::getHeader().

        leaf.getHeader(&leafHead);

        if (index >= leafHead.numEntries) {
            /* (all the entries in the block has been searched; search from the
            beginning of the next leaf index block. */

            // update block to rblock of current block and index to 0.

            block = leafHead.rblock;
            index = 0;

            if (block == -1) {
                // (end of linked list reached - the search is done.)
                return RecId{-1, -1};
            }
        }
    }

    /******  Traverse through all the internal nodes according to value
             of attrVal and the operator op                             ******/

    /* (This section is only needed when
        - search restarts from the root block (when searchIndex is reset by caller)
        - root is not a leaf
        If there was a valid search index, then we are already at a leaf block
        and the test condition in the following loop will fail)
    */

    /* block is of type IND_INTERNAL */
    while(StaticBuffer::getStaticBlockType(block) == IND_INTERNAL){
        // load the block into internalBlk using IndInternal::IndInternal().
        IndInternal internalBlk(block);

        HeadInfo intHead;

        // load the header of internalBlk into intHead using BlockBuffer::getHeader()

        internalBlk.getHeader(&intHead);

        // declare intEntry which will be used to store an entry of internalBlk.
        InternalEntry intEntry;

        /* op is one of NE, LT, LE */
        if (op == NE || op == LT || op == LE) {

            /*
            - NE: need to search the entire linked list of leaf indices of the B+ Tree,
            starting from the leftmost leaf index. Thus, always move to the left.

            - LT and LE: the attribute values are arranged in ascending order in the
            leaf indices of the B+ Tree. Values that satisfy these conditions, if
            any exist, will always be found in the left-most leaf index. Thus,
            always move to the left.
            */

            // load entry in the first slot of the block into intEntry
            // using IndInternal::getEntry().

            internalBlk.getEntry(&intEntry,0);
            block = intEntry.lChild;

        } else {
            /*
            - EQ, GT and GE: move to the left child of the first entry that is
            greater than (or equal to) attrVal
            (we are trying to find the first entry that satisfies the condition.
            since the values are in ascending order we move to the left child which
            might contain more entries that satisfy the condition)
            */

            /*
             traverse through all entries of internalBlk and find an entry that
             satisfies the condition.
             if op == EQ or GE, then intEntry.attrVal >= attrVal
             if op == GT, then intEntry.attrVal > attrVal
             Hint: the helper function compareAttrs() can be used for comparing
            */
            int i = 0;
            bool found = false;
            for (; i < intHead.numEntries; i++){
                internalBlk.getEntry(&intEntry, i);

                int cmpVal = compareAttrs(intEntry.attrVal, attrVal,type);

                if (
                    (op == EQ && cmpVal >= 0) ||
                    (op == GT && cmpVal > 0) ||
                    (op == GE && cmpVal >= 0)){
                    // (entry satisfying the condition found)

                    // move to the left child of that entry
                    found = true;

                    break;
                }
            }

            if (found){
                // move to the left child of that entry
                block = intEntry.lChild;
            }else{
                // move to the right child of the last entry of the block
                // i.e numEntries - 1 th entry of the block

                block = intEntry.rChild;
            }
        }
    }

    // NOTE: `block` now has the block number of a leaf index block.

    /******  Identify the first leaf index entry from the current position
                that satisfies our condition (moving right)             ******/

    while (block != -1) {
        std::cout<<"bptree:167 block no. is "<<block<<std::endl;
        // load the block into leafBlk using IndLeaf::IndLeaf().
        IndLeaf leafBlk(block);
        HeadInfo leafHead;

        // load the header to leafHead using BlockBuffer::getHeader().

        leafBlk.getHeader(&leafHead);

        // declare leafEntry which will be used to store an entry from leafBlk
        Index leafEntry;


        std::cout<<"bptree:180 numEntries is: "<<leafHead.numEntries<<std::endl;
        /*index < numEntries in leafBlk*/
        while (index < leafHead.numEntries) {

            // load entry corresponding to block and index into leafEntry
            // using IndLeaf::getEntry().

            leafBlk.getEntry(&leafEntry,index);

            std::cout<<"bptree:189 leafEntry ";printVal(leafEntry.attrVal,type);

            int cmpVal = compareAttrs(leafEntry.attrVal,attrVal,type);/* comparison between leafEntry's attribute value
                            and input attrVal using compareAttrs()*/

            if (
                (op == EQ && cmpVal == 0) ||
                (op == LE && cmpVal <= 0) ||
                (op == LT && cmpVal < 0) ||
                (op == GT && cmpVal > 0) ||
                (op == GE && cmpVal >= 0) ||
                (op == NE && cmpVal != 0)
            ) {
                // (entry satisfying the condition found)

                // set search index to {block, index}

                searchIndex = IndexId{block, index};

                AttrCacheTable::setSearchIndex(relId,attrName,&searchIndex);

                // return the recId {leafEntry.block, leafEntry.slot}.

                return RecId{leafEntry.block,leafEntry.slot};

            } else if ((op == EQ || op == LE || op == LT) && cmpVal > 0) {
                /*future entries will not satisfy EQ, LE, LT since the values
                    are arranged in ascending order in the leaves */

                return RecId{-1, -1};
            }

            // search next index.
            ++index;
        }

        /*only for NE operation do we have to check the entire linked list;
        for all the other op it is guaranteed that the block being searched
        will have an entry, if it exists, satisying that op. */
        if (op != NE) {
            break;
        }

        // block = next block in the linked list, i.e., the rblock in leafHead.
        // update index to 0.

        block = leafHead.rblock;
        index = 0;
    }

    // no entry satisying the op was found; return the recId {-1,-1}

    return RecId{-1,-1};
}

int BPlusTree::bPlusCreate(int relId, char attrName[ATTR_SIZE]) {

    // if relId is either RELCAT_RELID or ATTRCAT_RELID:
    //     return E_NOTPERMITTED;

    if(relId == RELCAT_RELID || relId == ATTRCAT_RELID) return E_NOTPERMITTED;


    // get the attribute catalog entry of attribute `attrName`
    // using AttrCacheTable::getAttrCatEntry()

    AttrCatEntry attrCatEntry;

    int ret = AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);

    // if getAttrCatEntry fails
    //     return the error code from getAttrCatEntry

    if(ret != SUCCESS){
        std::cout<<"bplus create error"<<std::endl;
        return ret;
    }

    if (attrCatEntry.rootBlock != -1 /* an index already exists for the attribute (check rootBlock field) */) {
        return SUCCESS;
    }

    /******Creating a new B+ Tree ******/

    // get a free leaf block using constructor 1 to allocate a new block
    IndLeaf rootBlockBuf;

    // (if the block could not be allocated, the appropriate error code
    //  will be stored in the blockNum member field of the object)

    // declare rootBlock to store the blockNumber of the new leaf block
    int rootBlock = rootBlockBuf.getBlockNum();

    // if there is no more disk space for creating an index
    if (rootBlock == E_DISKFULL) {
        return E_DISKFULL;
    }

    attrCatEntry.rootBlock = rootBlock;
    AttrCacheTable::setAttrCatEntry(relId,attrName,&attrCatEntry);

    RelCatEntry relCatEntry;

    // load the relation catalog entry into relCatEntry
    // using RelCacheTable::getRelCatEntry().

    RelCacheTable::getRelCatEntry(relId,&relCatEntry);

    int block = relCatEntry.firstBlk;  /* first record block of the relation */

    int offset=0; 

    /***** Traverse all the blocks in the relation and insert them one
           by one into the B+ Tree *****/

    int count = 1;

    std::cout<<"block no."<<block<<std::endl;

    while (block != -1) {

        // declare a RecBuffer object for `block` (using appropriate constructor)

        RecBuffer recBuffer(block);

        unsigned char slotMap[relCatEntry.numSlotsPerBlk];

        // load the slot map into slotMap using RecBuffer::getSlotMap().

        recBuffer.getSlotMap(slotMap);

        // for every occupied slot of the block

        for(int slot=0;slot<relCatEntry.numSlotsPerBlk;slot++){
            
            if(slotMap[slot]==SLOT_UNOCCUPIED) continue;

            Attribute record[relCatEntry.numAttrs];

            // load the record corresponding to the slot into `record`
            // using RecBuffer::getRecord().

            recBuffer.getRecord(record,slot);

            // declare recId and store the rec-id of this record in it
            // RecId recId{block, slot};

            RecId recId{block,slot};

            // insert the attribute value corresponding to attrName from the record
            // into the B+ tree using bPlusInsert.
            // (note that bPlusInsert will destroy any existing bplus tree if
            // insert fails i.e when disk is full)
            int retVal = bPlusInsert(relId, attrName, record[attrCatEntry.offset], recId);

            std::cout<<"no of slots per block is = "<<relCatEntry.numSlotsPerBlk<<" bPlus inserted on "<<count++<<" elements and no. of slots = "<<slot<<std::endl;

            if (retVal == E_DISKFULL) {
                // (unable to get enough blocks to build the B+ Tree.)
                return E_DISKFULL;
            }
        }

        // get the header of the block using BlockBuffer::getHeader()

        HeadInfo header;

        recBuffer.getHeader(&header);

        // set block = rblock of current block (from the header)

        block = header.rblock;
        std::cout<<"block is" << block << "\n";
    }

    return SUCCESS;
}

int BPlusTree::bPlusDestroy(int rootBlockNum) {

    if (rootBlockNum < 0 || rootBlockNum >= DISK_BLOCKS /*rootBlockNum lies outside the valid range [0,DISK_BLOCKS-1]*/) {
        return E_OUTOFBOUND;
    }

    int type = StaticBuffer::getStaticBlockType(rootBlockNum); /* type of block (using StaticBuffer::getStaticBlockType())*/

    if (type == IND_LEAF) {

        // declare an instance of IndLeaf for rootBlockNum using appropriate
        // constructor

        IndLeaf indLeaf(rootBlockNum);

        // release the block using BlockBuffer::releaseBlock().

        indLeaf.releaseBlock();

        return SUCCESS;

    } else if (type == IND_INTERNAL) {

        // declare an instance of IndInternal for rootBlockNum using appropriate
        // constructor

        IndInternal indInternal(rootBlockNum);

        HeadInfo header;

        // load the header of the block using BlockBuffer::getHeader().

        indInternal.getHeader(&header);

        /*iterate through all the entries of the internalBlk and destroy the lChild
        of the first entry and rChild of all entries using BPlusTree::bPlusDestroy().
        (the rchild of an entry is the same as the lchild of the next entry.
         take care not to delete overlapping children more than once ) */

        InternalEntry entry;

        indInternal.getEntry(&entry,0);

        if(entry.lChild != -1) {
            int ret = bPlusDestroy(entry.lChild);

            if(ret != SUCCESS) return ret;
        } 

        for(int i=0;i<header.numEntries;i++){

            indInternal.getEntry(&entry,i);

            int ret = bPlusDestroy(entry.rChild);
            
            if(ret != SUCCESS) return ret;

        }

        // release the block using BlockBuffer::releaseBlock().

        indInternal.releaseBlock();

        return SUCCESS;

    } else {
        // (block is not an index block.)
        return E_INVALIDBLOCK;
    }

}

int BPlusTree::bPlusInsert(int relId, char attrName[ATTR_SIZE], Attribute attrVal, RecId recId) {


    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().

    AttrCatEntry attrCatEntry;

    int ret = AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);

    // if getAttrCatEntry() failed
    //     return the error code

    if(ret != SUCCESS){

        std::cout<<"error in bPlusInsert"<<std::endl;
        return ret;

    }

    int blockNum = attrCatEntry.rootBlock; /* rootBlock of B+ Tree (from attrCatEntry) */

    if (blockNum == -1) {  /* there is no index on attribute (rootBlock is -1) */
        return E_NOINDEX;
    }

    // find the leaf block to which insertion is to be done using the
    // findLeafToInsert() function

    int leafBlkNum = findLeafToInsert(blockNum, attrVal, attrCatEntry.attrType);

    std::cout<<"bptree:465->leafBlkNum "<<leafBlkNum<<"\n";

    // insert the attrVal and recId to the leaf block at blockNum using the
    // insertIntoLeaf() function.

    // declare a struct Index with attrVal = attrVal, block = recId.block and
    // slot = recId.slot to pass as argument to the function.

    Index entry;

    entry.attrVal = attrVal;entry.block = recId.block; entry.slot=recId.slot;

    // std::cout<<"bp 477: "<<attrVal.nVal<<"\n";

    ret = insertIntoLeaf(relId, attrName, leafBlkNum, entry);

    // NOTE: the insertIntoLeaf() function will propagate the insertion to the
    //       required internal nodes by calling the required helper functions
    //       like insertIntoInternal() or createNewRoot()

    if (ret == E_DISKFULL/*insertIntoLeaf() returns E_DISKFULL */) {
        // destroy the existing B+ tree by passing the rootBlock to bPlusDestroy().

        bPlusDestroy(blockNum);

        // update the rootBlock of attribute catalog cache entry to -1 using
        // AttrCacheTable::setAttrCatEntry().

        attrCatEntry.rootBlock = -1;

        AttrCacheTable::setAttrCatEntry(relId,attrName,&attrCatEntry);

        return E_DISKFULL;
    }

    std::cout<<"----bplus insert success----\n";
    
    return SUCCESS;
}

int BPlusTree::findLeafToInsert(int rootBlock, Attribute attrVal, int attrType) {

    int blockNum = rootBlock;

    while (StaticBuffer::getStaticBlockType(blockNum) != IND_LEAF) {  // use StaticBuffer::getStaticBlockType()
        
        // declare an IndInternal object for block using appropriate constructor

        IndInternal indInternal(blockNum);

        // get header of the block using BlockBuffer::getHeader()

        HeadInfo header;

        indInternal.getHeader(&header);

        InternalEntry entry;

        /* iterate through all the entries, to find the first entry whose
             attribute value >= value to be inserted.
             NOTE: the helper function compareAttrs() declared in BlockBuffer.h
                   can be used to compare two Attribute values. */

        bool found = false;

        int i;

        for(i=0;i<header.numEntries;i++){
            
            indInternal.getEntry(&entry,i);

            if(compareAttrs(entry.attrVal,attrVal,attrType) >= 0){
                found = true;
                break;
            }    

        }
        
        if (!found) { /*no such entry is found*/

            // set blockNum = rChild of (nEntries-1)'th entry of the block
            InternalEntry entry;
            indInternal.getEntry(&entry,header.numEntries-1);

            blockNum = entry.rChild;
            // (i.e. rightmost child of the block)

        } else {

            // set blockNum = lChild of the entry that was found
            InternalEntry entry;
            indInternal.getEntry(&entry,i);
            blockNum = entry.lChild;

        }
    }

    return blockNum;
}

int BPlusTree::insertIntoLeaf(int relId, char attrName[ATTR_SIZE], int blockNum, Index indexEntry) {

    std::cout<<"insert into leaf \n";
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().

    AttrCatEntry attrCatEntry;

    AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);

    // declare an IndLeaf instance for the block using appropriate constructor

    IndLeaf leafBlock(blockNum);

    HeadInfo blockHeader;

    // store the header of the leaf index block into blockHeader
    // using BlockBuffer::getHeader()

    leafBlock.getHeader(&blockHeader);

    // the following variable will be used to store a list of index entries with
    // existing indices + the new index to insert

    Index indices[blockHeader.numEntries + 1];

    // std::cout<<"numEntries: "<<blockHeader.numEntries<<"\n";

    /*
    Iterate through all the entries in the block and copy them to the array indices.
    Also insert `indexEntry` at appropriate position in the indices array maintaining
    the ascending order.
    - use IndLeaf::getEntry() to get the entry
    - use compareAttrs() declared in BlockBuffer.h to compare two Attribute structs
    */

    int index = 0;
    bool isNotAdded = true;
    for(int i=0;i<blockHeader.numEntries;i++){

            Index temp;

            leafBlock.getEntry(&temp,i);

            if(compareAttrs(temp.attrVal,indexEntry.attrVal,attrCatEntry.attrType) > 0 && isNotAdded){
                indices[index++] = indexEntry;
                // std::cout<<"bp608:index entry ";printVal(indexEntry.attrVal,attrCatEntry.attrType);
                isNotAdded = false;
            }

            indices[index++] = temp;

    }
    if(isNotAdded) indices[index] = indexEntry;

    if (blockHeader.numEntries < MAX_KEYS_LEAF) {
        // (leaf block has not reached max limit)

        // increment blockHeader.numEntries and update the header of block
        // using BlockBuffer::setHeader().

        blockHeader.numEntries++;

        leafBlock.setHeader(&blockHeader);

        // iterate through all the entries of the array `indices` and populate the
        // entries of block with them using IndLeaf::setEntry().

        for(int i=0;i<blockHeader.numEntries;i++){
            leafBlock.setEntry(&indices[i],i);
            std::cout<<"bp 633: block: "<<blockNum<<" slot "<<i<<" ";printVal(indices[i].attrVal,attrCatEntry.attrType);
        }

        return SUCCESS;
    }

    // If we reached here, the `indices` array has more than entries than can fit
    // in a single leaf index block. Therefore, we will need to split the entries
    // in `indices` between two leaf blocks. We do this using the splitLeaf() function.
    // This function will return the blockNum of the newly allocated block or
    // E_DISKFULL if there are no more blocks to be allocated.

    int newRightBlk = splitLeaf(blockNum, indices);

    // if splitLeaf() returned E_DISKFULL
    //     return E_DISKFULL

    if(newRightBlk == E_DISKFULL) return E_DISKFULL;

    /* the current leaf block was not the root */
    if (blockHeader.pblock != -1 ) {  // check pblock in header

        // insert the middle value from `indices` into the parent block using the
        // insertIntoInternal() function. (i.e the last value of the left block)
        
        // the middle value will be at index 31 (given by constant MIDDLE_INDEX_LEAF)

        // create a struct InternalEntry with attrVal = indices[MIDDLE_INDEX_LEAF].attrVal,
        // lChild = currentBlock, rChild = newRightBlk and pass it as argument to
        // the insertIntoInternalFunction as follows

        InternalEntry entry;

        entry.attrVal = indices[MIDDLE_INDEX_LEAF].attrVal;

        entry.lChild = blockNum;

        entry.rChild = newRightBlk;

        int ret = insertIntoInternal(relId,attrName,blockHeader.pblock,entry);

        if(ret == E_DISKFULL){

            std::cout<<"Disk full while inserting->not root"<<std::endl;

            return E_DISKFULL;
        }

        // insertIntoInternal(relId, attrName, parent of current block, new internal entry)

    } else {

        // the current block was the root block and is now split. a new internal index
        // block needs to be allocated and made the root of the tree.
        // To do this, call the createNewRoot() function with the following arguments

        // createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal,
        //               current block, new right block)

       int ret = createNewRoot(relId,attrName,indices[MIDDLE_INDEX_LEAF].attrVal,blockNum,newRightBlk);

       if(ret == E_DISKFULL){

            std::cout<<"Disk full while inserting->root"<<std::endl;

            return E_DISKFULL;
       }
    }

    // if either of the above calls returned an error (E_DISKFULL), then return that
    // else return SUCCESS

    return SUCCESS;
}

int BPlusTree::splitLeaf(int leafBlockNum, Index indices[]) {

    std::cout<<"splitting leaf "<<leafBlockNum<<"\n";
    // declare rightBlk, an instance of IndLeaf using constructor 1 to obtain new
    // leaf index block that will be used as the right block in the splitting

    IndLeaf rightBlk;

    // std::cout<<"-------1-----\n";

    IndLeaf leftBlk(leafBlockNum);


    

    // declare leftBlk, an instance of IndLeaf using constructor 2 to read from
    // the existing leaf block

    int rightBlkNum = rightBlk.getBlockNum();
    int leftBlkNum = leftBlk.getBlockNum();

    if (rightBlkNum == E_DISKFULL) { /* newly allocated block has blockNum E_DISKFULL */
        //(failed to obtain a new leaf index block because the disk is full)

        std::cout<<"diskfull in splitleaf"<<std::endl;
        return E_DISKFULL;
    }

    // std::cout<<"-------1-----\n";

    HeadInfo leftBlkHeader, rightBlkHeader;
    // get the headers of left block and right block using BlockBuffer::getHeader()

    leftBlk.getHeader(&leftBlkHeader);
    rightBlk.getHeader(&rightBlkHeader);

    // set rightBlkHeader with the following values
    // - number of entries = (MAX_KEYS_LEAF+1)/2 = 32,
    // - pblock = pblock of leftBlk
    // - lblock = leftBlkNum
    // - rblock = rblock of leftBlk
    // and update the header of rightBlk using BlockBuffer::setHeader()

    rightBlkHeader.numEntries = (MAX_KEYS_LEAF+1)/2;
    rightBlkHeader.pblock = leftBlkHeader.pblock;
    rightBlkHeader.lblock = leftBlkNum; // all the blocks of the same level and of the same parent are connected
    rightBlkHeader.rblock = leftBlkHeader.rblock;
    rightBlkHeader.blockType = leftBlkHeader.blockType;

    rightBlk.setHeader(&rightBlkHeader);

    // std::cout<<"-------2-----\n";

    // set leftBlkHeader with the following values
    // - number of entries = (MAX_KEYS_LEAF+1)/2 = 32
    // - rblock = rightBlkNum
    // and update the header of leftBlk using BlockBuffer::setHeader() */

    leftBlkHeader.numEntries = (MAX_KEYS_LEAF+1)/2;
    leftBlkHeader.rblock = rightBlkNum;

    leftBlk.setHeader(&leftBlkHeader);

    // set the first 32 entries of leftBlk = the first 32 entries of indices array
    // and set the first 32 entries of newRightBlk = the next 32 entries of
    // indices array using IndLeaf::setEntry().

    for(int slot=0;slot<32;slot++){
        leftBlk.setEntry(&indices[slot],slot);
        rightBlk.setEntry(&indices[MIDDLE_INDEX_LEAF+ 1 + slot],slot);
    }

    // std::cout<<"-------3-----\n";

    return rightBlkNum;
}

int BPlusTree::insertIntoInternal(int relId, char attrName[ATTR_SIZE], int intBlockNum, InternalEntry intEntry) {

    std::cout<<"insert into internal\n";
    
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().

    AttrCatEntry attrCatEntry;

    AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);

    // declare intBlk, an instance of IndInternal using constructor 2 for the block
    // corresponding to intBlockNum

    IndInternal intBlk(intBlockNum);

    HeadInfo blockHeader;
    // load blockHeader with header of intBlk using BlockBuffer::getHeader().

    intBlk.getHeader(&blockHeader);

    // declare internalEntries to store all existing entries + the new entry
    InternalEntry internalEntries[blockHeader.numEntries + 1];

    /*
    Iterate through all the entries in the block and copy them to the array
    `internalEntries`. Insert `indexEntry` at appropriate position in the
    array maintaining the ascending order.
        - use IndInternal::getEntry() to get the entry
        - use compareAttrs() to compare two structs of type Attribute

    Update the lChild of the internalEntry immediately following the newly added
    entry to the rChild of the newly added entry.
    */

    InternalEntry entry;

    int index=0;
    int insertedIndex = -1;

    for(int slot=0;slot<blockHeader.numEntries;slot++){
        
        intBlk.getEntry(&entry,slot);

        if(compareAttrs(entry.attrVal,intEntry.attrVal,attrCatEntry.attrType) > 0 && insertedIndex == -1){
            insertedIndex = index;
            internalEntries[index++] = intEntry;
        }

        internalEntries[index++] = entry;

    }

    if(insertedIndex == -1) {
        internalEntries[blockHeader.numEntries] = intEntry;
        insertedIndex = blockHeader.numEntries;
    }

    // setting the previous entry's rChild to lChild of `intEntry`
    if(insertedIndex > 0 ){
        internalEntries[insertedIndex-1].rChild = intEntry.lChild;
    }

    // setting the following entry's lChild to rChild of `intEntry`

    if(insertedIndex < blockHeader.numEntries){
        internalEntries[insertedIndex+1].lChild = intEntry.rChild;
    }

    if (blockHeader.numEntries < MAX_KEYS_INTERNAL) {

        // (internal index block has not reached max limit)

        // increment blockheader.numEntries and update the header of intBlk
        // using BlockBuffer::setHeader().

        blockHeader.numEntries++;

        intBlk.setHeader(&blockHeader);

        // iterate through all entries in internalEntries array and populate the
        // entries of intBlk with them using IndInternal::setEntry().

        for(int slot=0;slot<blockHeader.numEntries;slot++){
            intBlk.setEntry(&internalEntries[slot],slot);
        }

        return SUCCESS;
    }

    // If we reached here, the `internalEntries` array has more than entries than
    // can fit in a single internal index block. Therefore, we will need to split
    // the entries in `internalEntries` between two internal index blocks. We do
    // this using the splitInternal() function.
    // This function will return the blockNum of the newly allocated block or
    // E_DISKFULL if there are no more blocks to be allocated.

    int newRightBlk = splitInternal(intBlockNum, internalEntries);

    if (newRightBlk == E_DISKFULL) { /* splitInternal() returned E_DISKFULL */

        // Using bPlusDestroy(), destroy the right subtree, rooted at intEntry.rChild.
        // This corresponds to the tree built up till now that has not yet been
        // connected to the existing B+ Tree

        bPlusDestroy(intEntry.rChild);

        return E_DISKFULL;
    }
    
    int ret;

    if ( blockHeader.pblock != -1) {  // (check pblock in header) /* the current block was not the root */
        
        // insert the middle value from `internalEntries` into the parent block
        // using the insertIntoInternal() function (recursively).

        // the middle value will be at index 50 (given by constant MIDDLE_INDEX_INTERNAL)

        // create a struct InternalEntry with lChild = current block, rChild = newRightBlk
        // and attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal
        // and pass it as argument to the insertIntoInternalFunction as follows

        // insertIntoInternal(relId, attrName, parent of current block, new internal entry)
        
        InternalEntry middleEntry;

        middleEntry.lChild = intBlockNum,middleEntry.rChild = newRightBlk;
        middleEntry.attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal;
        
        ret = insertIntoInternal(relId,attrName,blockHeader.pblock,middleEntry);


    } else {

        // the current block was the root block and is now split. a new internal index
        // block needs to be allocated and made the root of the tree.
        // To do this, call the createNewRoot() function with the following arguments

        // createNewRoot(relId, attrName,
        //               internalEntries[MIDDLE_INDEX_INTERNAL].attrVal,
        //               current block, new right block)

        ret = createNewRoot(relId,attrName,internalEntries[MIDDLE_INDEX_INTERNAL].attrVal,intBlockNum,newRightBlk);
    }

    // if either of the above calls returned an error (E_DISKFULL), then return that
    // else return SUCCESS

    return ret;

}

int BPlusTree::splitInternal(int intBlockNum, InternalEntry internalEntries[]) {

    std::cout<<"splitting internal\n";

    // declare rightBlk, an instance of IndInternal using constructor 1 to obtain new
    // internal index block that will be used as the right block in the splitting

    IndInternal rightBlk,leftBlk(intBlockNum);

    std::cout<<"941 intBlockNum= "<<intBlockNum<<"\n";

    // declare leftBlk, an instance of IndInternal using constructor 2 to read from
    // the existing internal index block

    int rightBlkNum = rightBlk.getBlockNum(); /* block num of right blk */
    int leftBlkNum = leftBlk.getBlockNum(); /* block num of left blk */

    if (rightBlkNum == E_DISKFULL ) { /* newly allocated block has blockNum E_DISKFULL */
        std::cout << "(failed to obtain a new internal index block because the disk is full)" << std::endl;
        return E_DISKFULL;
    }

    HeadInfo leftBlkHeader, rightBlkHeader;

    // get the headers of left block and right block using BlockBuffer::getHeader()

    leftBlk.getHeader(&leftBlkHeader);
    rightBlk.getHeader(&rightBlkHeader);

    // set rightBlkHeader with the following values
    // - number of entries = (MAX_KEYS_INTERNAL)/2 = 50
    // - pblock = pblock of leftBlk
    // and update the header of rightBlk using BlockBuffer::setHeader()

    rightBlkHeader.numEntries = MAX_KEYS_INTERNAL/2;
    rightBlkHeader.pblock = leftBlkHeader.pblock;

    rightBlk.setHeader(&rightBlkHeader);

    // set leftBlkHeader with the following values
    // - number of entries = (MAX_KEYS_INTERNAL)/2 = 50
    // and update the header using BlockBuffer::setHeader()

    leftBlkHeader.numEntries = MAX_KEYS_INTERNAL/2;

    leftBlk.setHeader(&leftBlkHeader);

    /*
    - set the first 50 entries of leftBlk = index 0 to 49 of internalEntries
      array
    - set the first 50 entries of newRightBlk = entries from index 51 to 100
      of internalEntries array using IndInternal::setEntry().
      (index 50 will be moving to the parent internal index block)
    */

    for(int slot=0;slot<MAX_KEYS_INTERNAL/2;slot++){
        leftBlk.setEntry(&internalEntries[slot],slot);
        rightBlk.setEntry(&internalEntries[slot + MIDDLE_INDEX_INTERNAL + 1],slot);
    }


    int type = StaticBuffer::getStaticBlockType(internalEntries[0].lChild); /* block type of a child of any entry of the internalEntries array */;
    //            (use StaticBuffer::getStaticBlockType())


    for (int i=0;i< rightBlkHeader.numEntries;i++) { /* each child block of the new right block */
        // declare an instance of BlockBuffer to access the child block using
        // constructor 2
        BlockBuffer childBlk(internalEntries[i + MIDDLE_INDEX_INTERNAL + 1].lChild);
        // std::cout<<"1000 lchild= "<<internalEntries[i + MIDDLE_INDEX_INTERNAL +1].lChild<<"\n";
        HeadInfo header;

        childBlk.getHeader(&header);

        header.pblock = rightBlkNum;

        childBlk.setHeader(&header);

        // update pblock of the block to rightBlkNum using BlockBuffer::getHeader()
        // and BlockBuffer::setHeader().
    }

    BlockBuffer childBlk(internalEntries[MAX_KEYS_INTERNAL].rChild);
    // std::cout<<"1000 lchild= "<<internalEntries[MAX_KEYS_INTERNAL].rChild<<"\n";
    HeadInfo header;

    childBlk.getHeader(&header);

    header.pblock = rightBlkNum;

    childBlk.setHeader(&header);


    return rightBlkNum;
}

int BPlusTree::createNewRoot(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int lChild, int rChild) {
    
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().

    AttrCatEntry attrCatEntry;

    AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);

    // declare newRootBlk, an instance of IndInternal using appropriate constructor
    // to allocate a new internal index block on the disk
    IndInternal newRootBlk;

    int newRootBlkNum = newRootBlk.getBlockNum();

    if (newRootBlkNum == E_DISKFULL) {

        // (failed to obtain an empty internal index block because the disk is full)

        // Using bPlusDestroy(), destroy the right subtree, rooted at rChild.
        // This corresponds to the tree built up till now that has not yet been
        // connected to the existing B+ Tree
        bPlusDestroy(rChild);

        return E_DISKFULL;
    }

    // update the header of the new block with numEntries = 1 using
    // BlockBuffer::getHeader() and BlockBuffer::setHeader()
    HeadInfo header;
    newRootBlk.getHeader(&header);
    header.numEntries = 1;
    newRootBlk.setHeader(&header);

    // create a struct InternalEntry with lChild, attrVal and rChild from the
    // arguments and set it as the first entry in newRootBlk using IndInternal::setEntry()
    InternalEntry entry;
    entry.attrVal = attrVal;
    entry.lChild = lChild;
    entry.rChild = rChild;
    newRootBlk.setEntry(&entry,0);

    BlockBuffer leftchild(lChild),rightchild(rChild);

    // declare BlockBuffer instances for the `lChild` and `rChild` blocks using
    // appropriate constructor and update the pblock of those blocks to `newRootBlkNum`

    HeadInfo lHeader,rHeader;

    leftchild.getHeader(&lHeader);
    lHeader.pblock = newRootBlkNum;
    leftchild.setHeader(&lHeader);

    rightchild.getHeader(&rHeader);
    rHeader.pblock = newRootBlkNum;
    rightchild.setHeader(&rHeader);
    // using BlockBuffer::getHeader() and BlockBuffer::setHeader()

    attrCatEntry.rootBlock = newRootBlkNum;

    AttrCacheTable::setAttrCatEntry(relId,attrName,&attrCatEntry);

    // update rootBlock = newRootBlkNum for the entry corresponding to `attrName`
    // in the attribute cache using AttrCacheTable::setAttrCatEntry().

    return SUCCESS;
}
