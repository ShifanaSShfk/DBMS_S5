#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"


/*
//  --- DISPLAYING (not using rblock)

  // create objects for the relation catalog and attribute catalog
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  // load the headers of both the blocks into relCatHeader and attrCatHeader.
  // (we will implement these functions later)
  relCatBuffer.getHeader(&relCatHeader);
  attrCatBuffer.getHeader(&attrCatHeader);

  // i = 0 to total relation count 
  for (int i = 0; i < relCatHeader.numEntries; i++) {

    Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

    relCatBuffer.getRecord(relCatRecord, i);

    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    // j = 0 to number of entries in the attribute catalog 
    for (int j = 0; j < attrCatHeader.numEntries; j++) {

      // declare attrCatRecord and load the attribute catalog entry into it
      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      attrCatBuffer.getRecord(attrCatRecord, j);
      
      // attribute catalog entry corresponds to the current relation 
      if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0) {
        const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
        // get the attribute name 
        printf("  %s: %s\n",attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
      }
    }
    printf("\n");
  }

  return 0;
}
*/



/*
//  --- DISPLAYING (using rblock) --- EXERCISE 1

int main(int argc, char *argv[]) {
  Disk disk_run;
  // Buffer Initialization -- RecBuffer objects
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);


  // Header Info
  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;
  
  // Original attribute catalog header
  HeadInfo attrCatHeader_store;

  relCatBuffer.getHeader(&relCatHeader);
  attrCatBuffer.getHeader(&attrCatHeader);
  
  attrCatHeader_store = attrCatHeader;


  int total_no_of_rel = relCatHeader.numEntries;
  int no_of_attr;
  int count;
  
  
  // Loop through Relations
  // Total number of relations 
  for(int i = 0; i < total_no_of_rel; i++) {
  
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBuffer.getRecord(relCatRecord, i);
    
    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);
    no_of_attr = relCatRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    count = 0;
    
    // Loop through Attributes
    while(count < no_of_attr){
    
    	//std::cout<<"entry = "<<attrCatHeader.numEntries<<" slots="<<attrCatHeader.numSlots;
    	    		
    	for(int j = 0; j < attrCatHeader.numEntries; j++) {
      		Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      		attrCatBuffer.getRecord(attrCatRecord, j);

      if(!strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal)) {
        count++;
        const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM":"STR";
        printf(" %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
      }
    }
    if(count < no_of_attr){
    	attrCatBuffer = RecBuffer(attrCatHeader.rblock);
    	attrCatBuffer.getHeader(&attrCatHeader);
    	}
    }
    attrCatHeader = attrCatHeader_store;
    printf("\n");
  }

  // return FrontendInterface::handleFrontend(argc, argv);
  return 0;
}
*/


/*
//  --- DISPLAY ATTRIBUTE NAME CHANGE --- EXERCISE 2
*/
void printAllRelationsAndAttributes() {
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  HeadInfo relCatHeader;

  relCatBuffer.getHeader(&relCatHeader);

  for (int i = 0; i < relCatHeader.numEntries; i++) {
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBuffer.getRecord(relCatRecord, i);


    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    int attrCatBlockNumber = ATTRCAT_BLOCK;
    while (attrCatBlockNumber != -1) {
      RecBuffer attrCatBuffer(attrCatBlockNumber);
      HeadInfo attrCatHeader;
      attrCatBuffer.getHeader(&attrCatHeader);
      attrCatBlockNumber = attrCatHeader.rblock;


      for (int j = 0; j < attrCatHeader.numEntries; j++) {
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatBuffer.getRecord(attrCatRecord, j);
        if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal))
          continue;
          

        const char* attrType = (attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER)
          ? "NUM"
          : "STR";

        printf("%s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
      }
    } 
    printf("\n");
  }
}


// rename class attr to batch and print all 
void exercise2() {
  const char targetRelation[] = "Students";
  int attrCatBlockNumber = ATTRCAT_BLOCK;
  while (attrCatBlockNumber != -1) {
    RecBuffer attrCatBuffer(attrCatBlockNumber);
    HeadInfo attrCatHeader;
    attrCatBuffer.getHeader(&attrCatHeader);
    attrCatBlockNumber = attrCatHeader.rblock;
    for (int j = 0; j < attrCatHeader.numEntries; j++) {
      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      attrCatBuffer.getRecord(attrCatRecord, j);
      if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, targetRelation) == 0) {
        if (strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, "Class") == 0) {
          unsigned char buffer[BLOCK_SIZE];
          Disk::readBlock(buffer, ATTRCAT_BLOCK);
          memcpy(buffer + 52 + 96*j + 16, "Batch", ATTR_SIZE);
          Disk::writeBlock(buffer, ATTRCAT_BLOCK);

          printAllRelationsAndAttributes();
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
  // Initialize the Run Copy of Disk 
  Disk disk_run;
  exercise2();
  printAllRelationsAndAttributes();

  return FrontendInterface::handleFrontend(argc, argv);
}
