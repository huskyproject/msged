
allmaps = \
 1125_866.chs    1125_asc.chs    1125_ibm.chs    1125_is5.chs    1125_koi.chs\
 850_asc.chs     850_ibm.chs     850_iso.chs     866_1125.chs    866_asc.chs \
 866_ibm.chs     866_is5.chs     866_koi.chs     can_ibm.chs     dut_ibm.chs \
 fin_ibm.chs     frn_ibm.chs     grm_ibm.chs     ibm_850.chs     ibm_asc.chs \
 ibm_iso.chs     ibm_mac.chs     ibm_swe.chs     ibm_vt1.chs     is5_1125.chs\
 is5_866.chs     is5_asc.chs     iso_850.chs     iso_asc.chs     iso_ibm.chs \
 iso_mac.chs     itl_ibm.chs     koi_1125.chs    koi_866.chs     koi_asc.chs \
 mac_850.chs     mac_ibm.chs     mac_iso.chs     nor_ibm.chs     prt_ibm.chs \
 spn_ibm.chs     swe_ibm.chs     swi_ibm.chs     uk_ibm.chs      \
 850_865.chs     865_866.chs     865_ibm.chs     866_865.chs  iso_865.chs \
 865_850.chs     865_asc.chs     865_iso.chs     ibm_865.chs


.PHONY: maps

default: maps

.c.o:
	$(CC) -c $<

makemaps: makemaps.o
	$(CC) -o makemaps makemaps.o
	$(CC) -o makemaps.exe makemaps.o


maps: 	makemaps readmaps.437 readmaps.850 readmaps.866 readmaps.koi readmaps.ukr \
	readmaps.is1 readmaps.is5 readmaps.865

readmaps.437:
	makemaps CP437 $(allmaps)
	-rm writmaps.437
	-rm readmaps.437
	mv writmaps.dat writmaps.437
	mv readmaps.dat readmaps.437

readmaps.850:
	makemaps CP850 $(allmaps)
	-rm writmaps.850
	-rm readmaps.850
	mv writmaps.dat writmaps.850
	mv readmaps.dat readmaps.850

readmaps.865:
	makemaps CP865 $(allmaps)
	-rm writmaps.865
	-rm readmaps.865
	mv writmaps.dat writmaps.865
	mv readmaps.dat readmaps.865

readmaps.866:
	makemaps CP866 $(allmaps)
	-rm writmaps.866
	-rm readmaps.866
	mv writmaps.dat writmaps.866
	mv readmaps.dat readmaps.866

readmaps.koi:
	makemaps KOI8-R $(allmaps)
	-rm writmaps.koi
	-rm readmaps.koi
	mv writmaps.dat writmaps.koi
	mv readmaps.dat readmaps.koi

readmaps.ukr:
	makemaps CP1125 $(allmaps)
	-rm writmaps.ukr
	-rm readmaps.ukr
	mv writmaps.dat writmaps.ukr
	mv readmaps.dat readmaps.ukr

readmaps.is1:
	makemaps LATIN-1 $(allmaps)
	-rm writmaps.is1
	-rm readmaps.is1
	mv writmaps.dat writmaps.is1
	mv readmaps.dat readmaps.is1

readmaps.is5:
	makemaps ISO-5 $(allmaps)
	-rm writmaps.is5
	-rm readmaps.is5
	mv writmaps.dat writmaps.is5
	mv readmaps.dat readmaps.is5

clean:
	-rm makemaps$(OBJ)

distclean: clean
	-rm readmaps.*
	-rm writmaps.*

