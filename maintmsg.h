/*
 *  MAINTMSG.H
 *
 *  Released to the public domain.
 *
 *  Prototypes for MAINTMSG.C.
 */

#ifndef __MAINTMSG_H__
#define __MAINTMSG_H__

void deletemsg(void);
int forward_msg(int to_area);
int redirect_msg(int to_area);
int copy_msg(int to_area);
int move_msg(int to_area);
void movemsg(void);

#endif
