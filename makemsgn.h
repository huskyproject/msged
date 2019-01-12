/*
 *  MAKEMSGN.H
 *
 *  Released to the public domain.
 */

#ifndef __MAKEMSGN_H__
#define __MAKEMSGN_H__

msg *duplicatemsg(msg * from);
void newmsg(void);
void reply(void);
void quote(void);
void reply_oarea(void);
void followup(void);
void replyextra(void);
void change(void);
int ChangeAttrib(msg * m);
char *subj_lookup(char *isto);
/* void GetAddress(FIDO_ADDRESS * addr, char *from, char *subj);
   int ChangeName(FIDO_ADDRESS * addr, char *from, char *subj, int y); */
int ChangeAddress(FIDO_ADDRESS * addr, int y, int nm_len);
int ChangeSubject(char *subj);
int EditHeader(msg * m);
void clear_attributes(struct _attributes *h);
void save(msg * m);

#endif
