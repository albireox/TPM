#!/usr/bin/bash
echo "Content-type: text/html"
echo ""
echo "<HTML>"
echo "<HEAD>"
echo "<TITLE>restart the archiver</TITLE>"
echo "</HEAD>"

echo "<BODY>"
echo "<blockquote>"
sub=`echo $QUERY_STRING | awk -F= '{print $2}'`
echo "Not presently implemented.<br> "
echo "If it was we'd now be logging in /mcptpm/$sub/freq_directory." 
echo "</blockquote>"
echo "</BODY>"
echo "</HTML>"
