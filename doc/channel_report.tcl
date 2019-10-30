proc read_dbcsv {} {
    set fileID [open tpm_crossref.csv r]
    set rlist [split [read $fileID] \n]
    close $fileID

    return $rlist
}

proc beginsubsection { ss } {
    puts " "
    puts "\\subsection{${ss}}"
    puts " "
    puts "This subsection is derived from the EPICS database"
    puts "entries and the ChannelArchiver configuration files" 
    puts "via gentpmdoc.sh."
}

proc begindeluxetable { ss tstr tablesection tid } {
    puts " "
    puts "\\begin{deluxetable}{llll}"
    puts "\\tablecaption{${ss}: ${tstr} Channels - Section ${tablesection}\\label{tbl-${ss}${tid}${tablesection}}}"
    puts "\\tablehead{"
    puts "\\colhead{Channel} &"
    puts "\\colhead{Scan Period} &"
    puts "\\colhead{Device Type} &"
    puts "\\colhead{Description}"
    puts "}"
    puts "\\startdata"
}

proc enddeluxetable { }  {
    puts "\\enddata"
    puts "\\end{deluxetable}"
}

proc channel_report {} {
    set rlist [read_dbcsv]
    set subsys "ADMIN AMP DEWAR DIO GALIL MIG MOTOR PLC PMSS SERVO SHMEM SLIP THERMAL WEATHER"
    
    foreach ss $subsys {
	beginsubsection $ss

	foreach table "archived not" {
	    set cnt 0
	    set tablesection 1
	    set tid [string range $table 0 0]
	    if {$table == "archived"} {
		set tstr "Archived"
	    } else {
		set tstr "Non-archived"
	    }
            foreach rec $rlist {
	        set pv [lindex [split $rec ,] 0]
		regsub -all "_" $pv "\\_" pv
	        set fulldesc [lindex [split $rec ,] 1]
		set subsys [lindex [split $fulldesc :] 0]
	        set desc [string range $fulldesc [expr [string length $subsys] + 1] end]
	        set scan [lindex [split $rec ,] 2]
	        set dtyp [lindex [split $rec ,] 3]
	        set inp [lindex [split $rec ,] 4] 
	        set out [lindex [split $rec ,] 5]
	        set archived [lindex [split [lindex [split $rec ,] 6]] 0]
	      
                if {$subsys == $ss && 
		     (($table == "archived" && $archived != "not") ||
		      ($table == "not" && $archived == "not"))} {

		    if {$cnt == 0} {
			begindeluxetable $ss $tstr $tablesection $tid
		    }

		    if {$table == "archived"} {
		        set field ".${archived}"
		    } else {
		        set field ""
		    }	

		    puts "${pv}${field} & ${scan} & ${dtyp} & ${desc} \\\\"
		    incr cnt

		    if {$cnt == 35} {
			enddeluxetable
		        incr tablesection
		        set cnt 0
		    }
		}
	    }
	    if {$cnt > 0} {
	        enddeluxetable
	    }
	}
    }
}

channel_report
exit
