/*
 *  DIALOGS.H
 *
 *  Written on 10-Jul-94 by John Dennis and released to the public domain.
 *
 *  Definitions and prototypes for the dialog boxes.
 */

#ifndef __DIALOGS_H__
#define __DIALOGS_H__

extern dlgbox settings;

void WriteSettings(void);
void ReadSettings(void);
int GetString(char *title, char *msg, char *buf, int len);
void SetupButton(button * b, char *txt, int x, int y, unsigned char sel, unsigned char norm, unsigned char back);
int ChoiceBox(char *title, char *txt, char *b1, char *b2, char *b3);
void SetDlgColor(dlgbox * Dlg);
void SetDialogColors(void);

#endif
