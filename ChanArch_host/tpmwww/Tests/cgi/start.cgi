#!/usr/bin/bash
echo "Content-type: text/html"
echo ""
echo "<HTML>"
echo "<HEAD>"
echo "<TITLE>Start the archiver</TITLE>"
echo "</HEAD>"

echo "<BODY>"
echo "<blockquote>"
echo "<H1>START THE TPM DATA ARCHIVER</H1>"
echo "<body BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#0000FF\" ALINK=\"#800080\" background=\"../blueback.jpg\">"
echo "<font COLOR=\"#000000\" FACE=\"Comic Sans MS, Arial, Helvetica\">"

echo "Please enter a subdirectory name under /mcptpm on sdsshost:"
echo "<FORM METHOD=\"GET\" ACTION=\"Restart.cgi\">"
echo "<INPUT TYPE=\"TEXT\" NAME=\"SUB\">"
echo "<INPUT TYPE=\"SUBMIT\">"
echo "</FORM>"
echo "</blockquote>"
echo "</BODY>"
echo "</HTML>"
