
#ifndef SPEAR_SHARED_MEMORY_H
#define SPEAR_SHARED_MEMORY_H

// Only use this if USE_SHARED_EVENTS is defined
#ifdef USE_SHARED_EVENTS

#include <vector>

namespace spear
{

class SharedMemory
{

public:

    /** Creates the shared memory segments, or links to the existing ones */
    static void * make(bool);

    /** malloc from shared memory */
    static void * alloc(size_t);

    /** free shared memory */
    static void free(void *) {};

private:

    /** Addresses where the shared segments are mapped in the local space */
    static std::vector<char *> segmentAddresses;

    /** How much is used from each segment */
    static std::vector<unsigned int *> segmentUses;

    /** Segment to be used for the next memory allocation */
    static unsigned int segmentIndex;
};

} // end namespace spear
 
#endif /* USE_SHARED_EVENTS */

#endif /* SPEAR_SHARED_MEMORY_H */
