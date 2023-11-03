#include "mfs.h"

struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
  if (dirp == NULL || dirp->di == NULL) {
    return NULL;
  }

  // No more entries to read
  // TODO: Add totalEntries as a field in struct fdDir in mfs.h
  if (dirp->dirEntryPosition >= dirp->totalEntries) {
    return NULL;
  }

  // TODO: But what is reclen field for? When iterating over directory entries
  // in a filesystem, you need to know how far to move the pointer to get to
  // the next entry. This is particularly crucial for filesystems where entries
  // may have variable lengths due to differences in file name lengths or other
  // metadata.

  // Assuming that dirp->di points to an array of fs_diriteminfo
  // structures. Then we should fetch the directory entry corresponding to
  // dirEntryPosition
  struct fs_diriteminfo *currentEntry = &dirp->di[dirp->dirEntryPosition];

  // Increment the position for the next call
  dirp->dirEntryPosition++;

  // Return the current entry
  return currentEntry;
}