#!/usr/local/bin/bash
echo "Content-type: text/html"
echo ""
echo "<HTML>"
echo "<HEAD>"
echo "<TITLE>Archive Files</TITLE>"
echo "</HEAD>"

echo "<BODY>"
echo "<blockquote>"
echo "<H1>TPM DATA ARCHIVE ACCESS</H1>"
echo "<body BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#0000FF\" ALINK=\"#800080\" background=\"../blueback.jpg\">"
echo "<font COLOR=\"#000000\" FACE=\"Comic Sans MS, Arial, Helvetica\">"

echo "Please select one of the TPM archives currently available on sdsshost:"
echo "<FORM METHOD=\"GET\" ACTION=\"CGIExport.cgi\">"
echo "<SELECT NAME=\"DIRECTORY\">"
for fn in `ls -r /mcptpm/*/freq_directory`;
do echo "<OPTION> $fn"; 
done
echo "</SELECT>"
echo "<INPUT TYPE=\"SUBMIT\" NAME=\"COMMAND\" VALUE=\"START\">"
echo "</FORM>"
echo "<blockquote>"
echo "</BODY>"
echo "</HTML>"
