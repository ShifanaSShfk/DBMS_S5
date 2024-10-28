#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"

/* ------------  INITIAL MAIN FORMAT  -----------------
int main(int argc, char *argv[]) {
  // Initialize the Run Copy of Disk 
  Disk disk_run;
  // StaticBuffer buffer;
  // OpenRelTable cache;

  return FrontendInterface::handleFrontend(argc, argv);
}
*/

#include <iostream>

/*
//  --- PRINT HELLO ---

int main(int argc, char *argv[]) {
  Disk disk_run;

// READ / WRITE IN BLOCK

  unsigned char buffer[BLOCK_SIZE];
//BLOCK_SIZE is a constant that has value 2048

  Disk::readBlock(buffer, 7000);
// 7000 is a random block number that's unused.

  char message[] = "hello";

//     dest ptr     src ptr  n(bytes)  --> copies
  memcpy(buffer + 20, message, 6);
// Now, buffer[20] = 'h', buffer[21] = 'e' ...

  Disk::writeBlock(buffer, 7000);


  unsigned char buffer2[BLOCK_SIZE];
  char message2[6];
  Disk::readBlock(buffer2, 7000);
  memcpy(message2, buffer2 + 20, 6);
  std::cout << message2;

  return 0;
}
*/

  // EXERCISES 1.1

int main(int argc, char *argv[]) {
    Disk disk_run;
    // BLOCK_SIZE is a constant that has value 2048
    const int BAM_SIZE = 4; 
    // Assuming the BAM requires 4 blocks
    unsigned char bamBuffer[BLOCK_SIZE * BAM_SIZE];
    
    // Read BAM Blocks
    for (int i = 0; i < BAM_SIZE; ++i) {
        Disk::readBlock(bamBuffer + i * BLOCK_SIZE, i);
    }

    // Print BAM Values
    std::cout << "Block Allocation Map values:" << std::endl;
    for (int i = 0; i < BLOCK_SIZE * BAM_SIZE; ++i) {
        std::cout << static_cast<int>(bamBuffer[i]) << " ";
        if ((i + 1) % 16 == 0) { // Print 16 values per line for better readability
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;

    // Check alignment with Disk Model
    // Here we expect specific values, such as 0 for free blocks and 1 for used blocks.
    // This part depends on the specific Disk Model you are using.
    // You can add conditions to verify if the values align with the expected model.

    return 0;
}