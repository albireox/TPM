# Open the archiver configuration file
proc read_arlist {} {
    set fileId [open cfg_all.tbl r]
    set arlist [split [read $fileId] \n]
    close $fileId

    return $arlist
}

# Now check the database entries
proc read_dblist {} {
    global dblist
    set rlist ""
    set fileId [open db_all.csv r]

    while {[gets $fileId line] > 0} {
	regsub -all "^ " $line {} line
	regsub -all ", " $line "," line
	set fields [split $line ,]
	set rname [lindex $fields 0]
	lappend rlist $rname
	set dblist($rname,DESC) [lindex $fields 1]
	set dblist($rname,SCAN) [lindex $fields 2]
	set dblist($rname,DTYP) [lindex $fields 3]
	set dblist($rname,INP) [lindex $fields 4]
	set dblist($rname,OUT) [lindex $fields 5]
     }
     close $fileId

     return $rlist 
}

# do the output in a reasonable format
proc dooutput { dbtmp rname archivedstat } {
    upvar $dbtmp dblist
    puts -nonewline "$rname,"
    puts -nonewline "$dblist($rname,DESC),"
    puts -nonewline "$dblist($rname,SCAN),"
    puts -nonewline "$dblist($rname,DTYP),"
    puts -nonewline "$dblist($rname,INP),"
    puts -nonewline "$dblist($rname,OUT),"
    puts "$archivedstat"
}

#
# Generate a report of what's been archived.
#
proc gen_crossref {} {
    global dblist
    set rnames [read_dblist]
    set arlist [read_arlist]
 
# Loop over all of the record names in the database  
    foreach rname $rnames { 
        set archivedstat "not archived"
# Check against the PVs listed in the archiver configuration files.
	foreach arentry $arlist {
	    set arname [lindex [split $arentry] 0]

# Is the record (default field=VAL) archived?
	    if {[string compare $rname $arname] == 0} {
		set archivedstat "VAL"
		dooutput dblist $rname $archivedstat
# Are any of the fields archived?
	    } elseif {[string first [format "%s." $rname] $arentry] == 0 } {
		set archivedstat [string range $arentry [expr [string length $rname] + 1] [expr [string length $rname] + 5]] 
		dooutput dblist $rname $archivedstat
	    } 
	}
	if {$archivedstat == "not archived"} {
	    dooutput dblist $rname $archivedstat
	}
    }
}

gen_crossref
exit

