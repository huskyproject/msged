/*
 *  NSHOW.H
 *
 *  Released to the public domain.
 *
 *  Prototypes for NSHOW.C.
 */

#ifndef __NSHOW_H__
#define __NSHOW_H__

extern int groupmove;
int InitScreen(void);
void AddHG(HotGroup * h, int num, int id, int x1, int y1, int x2, int y2);
void BuildHotSpots(void);
void KillHotSpots(void);
void DrawHeader(void);
void ClearScreen(void);
void ClearMsgScreen(void);
void ShowNewArea(void);
int OpenMsgWnd(int wid, int dep, char * title, char * msg, int x, int y);
void SendMsgWnd(char * msg, int y);
int CloseMsgWnd(void);
void MakeMsgAttrs(char * buf, struct _attributes * att, int scanned, int times_read);
void ShowAddress(FIDO_ADDRESS * addr, int y);
void ShowNameAddress(char * name, FIDO_ADDRESS * addr, int y, int newrcvd, int nm_only);
void ShowSubject(char * subj);
void ShowAttrib(msg * m);
void ShowMsgHeader(msg * m);
void PutLine(LINE * l, int y);
void MsgScroll(int Dir);
void Go_Up(void);
void Go_Dwn(void);
void Go_PgDwn(void);
void Go_PgUp(void);
void RefreshMsg(LINE * line, int y);

#endif // ifndef __NSHOW_H__
