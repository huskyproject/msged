
  Build MsgEd 6.2 from source code.

There are two kinds of Makefiles: One for use with other software of the
Husky project (Makefile) and one for standalone-compiling of MsgEd TE
(makefile.*).

To build MsgEd 6.2 with Husky you should using husky libraries following
versions: smapi 2.4-stable and fidoconfig 1.4-stable.
To use the Husky-Makefile, you have to create a Makefile-Config (huskymak.cfg,
example is distributed in huskybse package of Husky) in the directory above
the MsgEd sources.
A sample and some notes on the new Makefile (README.Makefiles) are included
in the husky-common package which can be found on
"http://husky.sourceforge.net/".

- sms, Sascha.Silbe@ldknet.org, 2:2435/424
- Stas Degteff, 2:5080/102
