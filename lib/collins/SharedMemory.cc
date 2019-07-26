
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "collins/SharedMemory.h"
#include <string.h>
#include <stdlib.h>

// This code is valid only if USE_SHARED_EVENTS is defined
#ifdef USE_SHARED_EVENTS

using namespace std;
using namespace spear;

//
// We create SEGMENT_COUNT shared memory segments, 
// each one of size SEGMENT_SIZE
//
#define SEGMENT_KEY 20000
#define SEGMENT_SIZE (32 * 1024 * 1024)
#define SEGMENT_COUNT 5

//
// The shared memory segments are attached at the end of 
// the heap address space (2 GB)
// 
#define HEAP_ADDRESS_END (2 * 1024 * 1024 * 1024)
#define SEGMENT_ADDRESS ((void *) (HEAP_ADDRESS_END - (SEGMENT_SIZE * SEGMENT_COUNT)))

std::vector<char *> SharedMemory::segmentAddresses;
std::vector<unsigned int *> SharedMemory::segmentUses;
unsigned int SharedMemory::segmentIndex = 0;

void * SharedMemory::make(bool createNew)
{
  // if shared segments already exist, access them read-only
  int getMode = 0444;
  int attachMode = SHM_RND | SHM_RDONLY;

  // if shared segments are created anew, access them read-write
  if(createNew == true){
    getMode = 0777 | IPC_CREAT;
    attachMode = SHM_RND;
  }

  bool success = true;

  // create SEGMENT_COUNT segments
  for(int i = 0; i < SEGMENT_COUNT; i ++){
    // system id of the current segment
    int id;

    // get/create the current segment
    if((id = shmget(SEGMENT_KEY + i, SEGMENT_SIZE, getMode)) < 0){
      success = false;
      break;
    }

    // local address of the current segment
    void * address = NULL;

    // attach the crt segment to the local address space
    if((address = (char *) 
	shmat(id, 
	      (void *) (((char *) SEGMENT_ADDRESS + (SEGMENT_SIZE * i))),
	      attachMode)) == (void *) -1){
      success = false;
      break;
    }

    // zero the allocated segment
    if(createNew){
      memset(address, 0, SEGMENT_SIZE);
    }

    if(createNew){
      cerr << "Shared segment #" << i 
	   << " mapped at address " << address << endl;
    }

    //
    // the first sizeof(unsigned int) bytes in each segment 
    // store the number of used bytes in the corresponding segment
    //
    segmentUses.push_back((unsigned int *) address);

    //
    // space following the first sizeof(unsigned int) bytes in each segment
    // is used for data allocation
    //
    segmentAddresses.push_back(((char *) address) + sizeof(unsigned int));
  }

  if(success == false){
    return NULL;
  }

  // start allocating from the first chunk
  segmentIndex = 0;

  // this is the first valid address
  return segmentAddresses[0];
}

void * SharedMemory::alloc(size_t size)
{
  // the current segment has ended
  if((* segmentUses[segmentIndex]) + size >= 
     SEGMENT_SIZE - sizeof(unsigned int)){

    // move to the next segment
    segmentIndex ++;
    
    // no other segments are available
    if(segmentIndex >= SEGMENT_COUNT){
      cerr << "Out of shared memory!" << endl;
      exit(13);
    }
  }

  // memory allocation
  void * address = 
    segmentAddresses[segmentIndex] + (* segmentUses[segmentIndex]);

  // increment the usage count in this segment
  (* segmentUses[segmentIndex]) += size;

  return address;
}

#endif // USE_SHARED_EVENTS
