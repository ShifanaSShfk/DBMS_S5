//  Stage 10    :  bPlusSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op)

#include "BPlusTree.h"

#include <cstring>

int BPlusTree::numOfComparisons;

/* 
 * @brief 
 * searches the relation specified to find the next record that satisfies the specified condition
 * 
 * The condition value is given by the argument attrVal. 
 * This function returns the recId of the next record satisfying the condition. 
 * The condition that is checked for is the following.
 * 
 * @param relId Relation Id of the relation containing the attribute with index
 * @param attrName Attribute/column name (which has an index) to which condition need to be checked with.
 * @param attrVal value of attribute that has to be checked against the operater
 * @param op Conditional Operator ( can be one among EQ , LE , LT , GE , GT , NE )
 * @return
 * Returns the block number and slot number of the record corresponding to the next hit. 
 * Returns {-1,-1} if no next hit.
*/ 
RecId BPlusTree::bPlusSearch(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int op) {
  // declare searchIndex which will be used to store search index for attrName.
  IndexId searchIndex;

  //get the search index corresponding to attribute with name attrName
  AttrCacheTable::getSearchIndex(relId, attrName, &searchIndex);

  AttrCatEntry attrCatEntry;
  AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

  // declare variables block and index which will be used during search
  int block, index;

  // search is done for the first time
  if ( searchIndex.block == -1 || searchIndex.index == -1 ) {

    // start the search from the first entry of root.
    block = attrCatEntry.rootBlock;
    index = 0;

    if ( block == -1 ) return RecId{-1, -1};
  }
  // Not first time
  else {
    /*a valid searchIndex points to an entry in the leaf index of the attribute's
    B+ Tree which had previously satisfied the op for the given attrVal.*/

    block = searchIndex.block;
    index = searchIndex.index + 1;  // search is resumed from the next index.

    // load block into leaf using IndLeaf::IndLeaf().
    IndLeaf leaf(block);

    // declare leafHead which will be used to hold the header of leaf.
    HeadInfo leafHead;
    leaf.getHeader(&leafHead);

    /* 
      All the entries in the block has been searched; 
      search from the beginning of the next leaf index block. 
    */
    if (index >= leafHead.numEntries) {

      // update block to rblock of current block and index to 0.
      block = leafHead.rblock;
      index = 0;
      // end of linked list reached - the search is done.
      if (block == -1) return RecId{-1, -1};
    }
  }

  /* 
    Traverse down through all the internal nodes according to value of attrVal and the operator op 
  */

  /* (This section is only needed when
      - search restarts from the root block (when searchIndex is reset by caller)
      - root is not a leaf
      If there was a valid search index, then we are already at a leaf block
      and the test condition in the following loop will fail)
  */

  while(StaticBuffer::getStaticBlockType(block) == IND_INTERNAL ) {

    // load the block into internalBlk using IndInternal::IndInternal().
    IndInternal internalBlk(block);

    HeadInfo intHead;
    internalBlk.getHeader(&intHead);

    // declare intEntry which will be used to store an entry of internalBlk.
    InternalEntry intEntry;

    if ( op == NE || op == LE || op == LT ) {
      /*
      - NE: need to search the entire linked list of leaf indices of the B+ Tree,
      starting from the leftmost leaf index. Thus, always move to the left.

      - LT and LE: the attribute values are arranged in ascending order in the
      leaf indices of the B+ Tree. Values that satisfy these conditions, if
      any exist, will always be found in the left-most leaf index. Thus,
      always move to the left.
      */

      // load entry in the first slot of the block into intEntry
      internalBlk.getEntry(&intEntry, 0);
      block = intEntry.lChild;

    } 
    else {
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
      int ind = 0;
      while ( ind < intHead.numEntries ){

        internalBlk.getEntry(&intEntry, ind);
        int ret = compareAttrs(intEntry.attrVal, attrVal, attrCatEntry.attrType );
        BPlusTree::numOfComparisons++;
        if (((op==EQ || op==GE) && ret >= 0)|| (op==GT && ret > 0)) break;

        ind++;
      }
      // such an entry is found - move to the left child of that entry
      if ( ind != intHead.numEntries ) {
        block = intEntry.lChild;
      } 
      // such an entry is not found - move to the right child of the last entry of the block
      else{
        internalBlk.getEntry(&intEntry, intHead.numEntries - 1);
        block = intEntry.rChild;
      }
    }
  }

  // NOTE: `block` now has the block number of a leaf index block.

  /******  Identify the first leaf index entry from the current position
              that satisfies our condition (moving right)             ******/

  while (block != -1) {
    // load the block into leafBlk using IndLeaf::IndLeaf().
    IndLeaf leafBlk(block);
    HeadInfo leafHead;
    leafBlk.getHeader(&leafHead);

    // declare leafEntry which will be used to store an entry from leafBlk
    Index leafEntry;
    while ( index < leafHead.numEntries ) {
      leafBlk.getEntry(&leafEntry, index);

      // int cmpVal = /* comparison between leafEntry's attribute value
      //                 and input attrVal using compareAttrs()*/
      int cmpVal = compareAttrs(leafEntry.attrVal, attrVal, attrCatEntry.attrType);
      BPlusTree::numOfComparisons++;

      // (entry satisfying the condition found)
      if (
        (op == EQ && cmpVal == 0) ||
        (op == LE && cmpVal <= 0) ||
        (op == LT && cmpVal <  0) ||
        (op == GT && cmpVal >  0) ||
        (op == GE && cmpVal >= 0) ||
        (op == NE && cmpVal != 0)
      ){
        searchIndex.block = block;
        searchIndex.index = index;
        AttrCacheTable::setSearchIndex(relId, attrName, &searchIndex);
        return RecId{leafEntry.block, leafEntry.slot};
      } 
      /*future entries will not satisfy EQ, LE, LT since the values
          are arranged in ascending order in the leaves */
      else if ((op == EQ || op == LE || op == LT) && cmpVal > 0) {
        return RecId{-1, -1};
      }
      // search next index.
      ++index;
    }

    /*only for NE operation do we have to check the entire linked list;
    for all the other op it is guaranteed that the block being searched
    will have an entry, if it exists, satisying that op. */
    if (op != NE) break;

    // For NE operation
    block = leafHead.rblock;
    index = 0;
  }

  // no entry satisying the op was found; return the recId {-1,-1}
  return RecId {-1,-1};
}
