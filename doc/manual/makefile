

html: msged_toc.html

dvi: msged.dvi

info: msged.info

# The DOC target does only work on Tobias Ernst's OS/2 machine ...
doc: msged.doc

# The INF target does only work on OS/2 ...
inf: msged.inf

msged_toc.html: msged.texi
	texi2html -split_node -number -menu msged.texi

msged.dvi: msged.texi
	texi2dvi msged.texi

msged.info: msged.texi
	makeinfo --no-split  msged.texi

msged.doc:
	texi2txt msged.texi

almostclean:
	-rm msged.fns
	-rm *~
	-rm msged.aux
	-rm msged.toc
	-rm msged.cp
	-rm msged.fn
	-rm msged.vr
	-rm msged.kr
	-rm msged.ke
	-rm msged.tp
	-rm msged.kes
	-rm msged.krs
	-rm msged.ky
	-rm msged.pg
	-rm msged.cps
	-rm msged.log
	-rm msged.fns
	-rm *~

clean: almostclean
	-rm msged.dvi
	-rm msged.inf
	-rm msged*html
	-rm msged.ps
	-rm msged.doc
	-rm msged.info*


# The OS/2 INF viewer fails if some of the files from the DVI generating
# process are still lying around. (I have not yet figured out which).
# Therefore, we make a clean before we make the INF file.

msged.inf: msged.texi almostclean
	texi2i msged.texi

