
  Build MsgEd 6.3 from source code.

There are two kinds of Makefiles: One for use with other software of the
husky project (Makefile) and one for standalone-compiling of MsgEd TE
(makefile.*).

To build MsgEd 6.3 with Husky you should using husky libraries following
versions: smapi 2.5-current and (possible) fidoconfig 1.9-current.
To use the Husky-Makefile, you have to create a Makefile-Config (huskymak.cfg)
in the directory above the MsgEd sources.
A sample and some notes on the new Makefile (README.Makefiles) are included
in the husky-common package which can be found on
"http://husky.sourceforge.net/".

- sms, Sascha.Silbe@ldknet.org, 2:2435/424

