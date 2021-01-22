/*
 *  VSEV.H
 *
 *  Written by Paul Edwards and released to the public domain.
 *
 *  Version7 nodelist processor.
 */

#ifndef __VSEV_H__
#define __VSEV_H__

typedef struct
{
    FILE *         fp;
    int            error;
    short          point;
    short          zone;
    short          net;
    short          node;
    short          hub;
    unsigned short cost;
    unsigned short fee;
    unsigned short flags;
    unsigned char  modem;
    unsigned char  phoneLen;
    unsigned char  passwordLen;
    unsigned char  boardLen;
    unsigned char  sysopLen;
    unsigned char  miscLen;
    unsigned char  packLen;
    unsigned char  baud;
    unsigned char  packData[256];
    unsigned char  unpackData[400];
    unsigned char  phone[100];
    unsigned char  password[30];
    unsigned char  board[100];
    unsigned char  sysop[100];
    unsigned char  misc[100];
} VSEV;
int vsevGetInfo(VSEV * vsev, char * dataFile, long offset);

#endif // ifndef __VSEV_H__
