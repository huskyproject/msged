/*
 *  MXBT.H
 *
 *  Written by Paul Edwards and released to the public domain.
 *
 *  See MXBT.C for more details.
 */

#ifndef __MXBT_H__
#define __MXBT_H__

struct mxbt_ctlrec
{
    /*
     *  I have commented out the below, and read it in separately, so
     *  as to minimize alignment problems.  The definition of this
     *  file format is now "The control record consists of two parts, a
     *  short integer followed by the following structure, stored in
     *  native format.  The index and leaf records remain in native
     *  format".
     */
    /* unsigned short recsize; */
    long           indexStart;
    long           rootStart;
    long           lastBlock;
    long           firstLeaf;
    long           lastLeaf;
    long           freeList;
    unsigned short levels;
    unsigned short xor;
};

struct mxbt_leafrec
{
    long           recType;
    long           prev;
    long           next;
    short          keyCount;
    unsigned short keyStart;
    struct
    {
        unsigned short offset;
        unsigned short len;
        long           value;
    } keys[1];
};

struct mxbt_indexrec
{
    long           recType;
    long           prev;
    long           next;
    short          keyCount;
    unsigned short keyStart;
    struct
    {
        unsigned short offset;
        unsigned short len;
        long           value;
        long           lower;
    } keys[1];
};

typedef struct
{
    int    error;
    FILE * fp;
    long   value;
    int (* compareF)(void * testKey, void * searchKey, int len);
    void * searchK;
    union
    {
        char intbuf[512];
        long x;
    } myunion;
    char *                 buf;
    struct mxbt_ctlrec     control;
    struct mxbt_indexrec * index;
    struct mxbt_leafrec *  leaf;
    unsigned short         recSize;
    long                   recordNum;
} MXBT;
long mxbtOneSearch(MXBT * mxbt, char * indexFile, void * searchKey, int (* compare)(void * testKey,
                                                                                    void * searchKey,
                                                                                    int len));

#endif // ifndef __MXBT_H__
