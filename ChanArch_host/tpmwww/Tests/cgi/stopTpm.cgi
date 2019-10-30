#!/usr/bin/bash
export TCL_LIBRARY=/p/tcl/v8_0_2/lib/tcl8.0
export TCLLIBPATH=/p/tk/v8_0_2/lib/tk8.0:/p/tcl/v8_0_2/lib/tcl8.0
export TCL_DIR=/p/tcl/v8_0_2    

echo "Content-type: text/html"
echo ""
echo "<HTML>"
echo "<HEAD>"
echo "<TITLE>stop the archiver</TITLE>"
echo "</HEAD>"
echo "<BODY>"
echo "<blockquote>"
echo "<H1>STOPPED THE TPM DATA ARCHIVER</H1>"
echo "<body BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#0000FF\" ALINK=\"#800080\" background=\"../blueback.jpg\">"
/p/tcl/v8_0_2/bin/tclsh stopTpm.tcl
echo "</blockquote>"
echo "</BODY>"
echo "</HTML>"
