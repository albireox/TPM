# Generate TPM documentation
# This places the documentation in <tpm_root>/doc.
# 
# Usage: gentpmdoc <tpm_root>
# 
cd $1/doc
#
echo "Concatenating ASCII database files..."
cat ../db/*.db | ./parsedb.sh | sort > db_all.csv
#
echo "Concatenating ChannelArchiver configuration files..."
cat ../ChanArch/config/*.cfg | grep -v '#' | grep -v '!' | sort > cfg_all.tbl
#
echo "Creating database/archiver cross-reference..."
wish -f crossref.tcl > tpm_crossref.csv
#
echo "Creating channel report tables..."
wish -f channel_report.tcl > tpm_channelreport.tex
#
echo "Creating PostScript version of documentation..."
latex tpm.tex
# run latex twice to resolve references
latex tpm.tex
dvips -f -t letter tpm.dvi > tpm.ps
rm -rf tpm.ps.gz
gzip tpm.ps
#
echo "Creating HTML version of documentation..."
rm -rf newhtml
~/latex2html-2002-2-1/latex2html tpm.tex -dir newhtml -mkdir -white
#
echo "Done."
exit
