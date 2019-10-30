tar -cvf doc.tar \
	aastex.cls aastexug.sty \
	tpm.tex alarms.tex developers.tex \
	displays.tex interfaces.tex subsystems.tex \
	gentpmdoc.sh parsedb.sh \
	crossref.tcl channel_report.tcl \
	tpm_dl.ps \
	mktar.sh
gzip doc.tar
