#!/usr/bin/bash
echo "Content-type: text/html"
echo ""
echo "<HTML>"
echo "<HEAD>"
echo "<body BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#0000FF\" ALINK=\"#800080\">"
echo "<font COLOR=\"#000000\" FACE=\"Comic Sans MS, Arial, Helvetica\">"
                                                                 
echo "<TITLE>TPM Channel Information</TITLE>"
echo "</HEAD>"
 
echo "<BODY>"
echo "<blockquote>"
read PV
echo "<H1>TPM Channel Information<br>$PV</H1>"
pvfull=`echo $PV | sed 's/PV=//'`
pv=`echo $pvfull | awk -F. '{print $1}'`
pv_csv=`grep "$pv," /p/tpmbase/ChanArch/tpmwww/tpm.csv`
echo "<table border>"
echo "<tr bgcolor=\"#aaaaff\"><td><b>ATTRIBUTE</b></td><td><b>VALUE</b></td></tr>"
echo $pv_csv | awk -F, '{
    printf "<tr><td>NAME</td><td>%s</td></tr>\n", $1;
    printf "<tr><td>ARCHIVED</td><td>%s</td></tr>\n", $2;
    printf "<tr><td>SYSTEM</td><td>%s</td></tr>\n", $3;
    printf "<tr><td>SUBSYSTEM</td><td>%s</td></tr>\n", $4;
    printf "<tr><td>DESC</td><td>%s</td></tr>\n",  $5;
    printf "<tr><td>RTYP</td><td>%s</td></tr>\n", $6;
    printf "<tr><td>SCAN</td><td>%s</td></tr>\n", $7;
    printf "<tr><td>DTYP</td><td>%s</td></tr>\n", $8;
    printf "<tr><td>INP</td><td>%s</td></tr>\n", $9;
    printf "<tr><td>OUT</td><td>%s</td></tr>\n", $10;
}'
echo "</table>"

echo "<br>"
echo "<a href=\"/tpm/ChannelGuide.html#getinfo\">GO BACK</a>"
echo "</blockquote>"
echo "</BODY>"
echo "</HTML>"                    
