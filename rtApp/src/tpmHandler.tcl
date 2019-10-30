#--------
# tpmHandler.tcl - IOP style handler for TPM information.
#--------

#--------
# Globals defined and used in this file.
#--------

#
# verbosity flag for debug/information.
#
global tpmVerbose
set tpmVerbose 0

#
# tpmData($pv) - 	only a placeholder since the TPM 
#			computes its own alarms. Not sure if
#			iop/packeteer will complain if this
#			didn't exist.
global tpmData

#
# tpmError($pv) -	The error strings made available 
#			to IOP. Error strings match EPICS
#			alarm severities, i.e.
#			NO_ALARM, MINOR, MAJOR, INVALID.
global tpmError

#
# Make sure the arrays exist and are properly initialized.
#
if {![info exists tpmData]} {
    set tpmData(arrayName) tpmData
    set tpmData(serialNum) 0 
    set tpmData(timeStamp) [getclock]
}
if {![info exists tpmError]} {
    set tpmError(arrayName) tpmError
    set tpmError(serialNum) 0 
    set tpmError(timeStamp) [getclock]
}

#--------
# Handler for TPM data
#--------
proc tpmHandler {} {
    global tpmError
    global tpmData
    global tpmVerbose

#
# check the status return and remove tpmHandler
# if faulted on first attempt.
#
    set status [updateTPMError]

    if {$status == 1 && $tpmError(serialNum) == 1} {
	murmur "PMM tpmHandler: faulted on first try, giving up"
	if {$tpmVerbose == 1} {
	    puts stderr "PMM tpmHandler: faulted on first try, giving up"
	}
	schedule tpmHandler "" 0
    }

    return 0 
}

#-------- 
#
# Send a query to the TPM using the caGet1 UNIX command line
# tool. This accepts an input file containing the channel
# names we want. Using caGet1 rather than caget cuts down on
# the number of processes we have to create.
#
# In the ideal world IOP would be linked to the et_wish library.
#
# The channels requested are all *.SEVR fields for indication
# of EPICS Alarm status. 
#
#--------
proc updateTPMError {} {
    global tpmData
    global tpmError
    global tpmVerbose

    set SEVR { NO_ALARM MINOR MAJOR INVALID }

    resetErrorArray tpmError
    resetErrorArray tpmData

    set DBFILE /p/tpmbase/db/tpmAlarms.db

    if [catch {set cagetvals [exec caGet1 $DBFILE ]}] {
	murmur "PMM tpmHandler: caGet1 failed"
	if {$tpmVerbose == 1} {
	    puts stderr "PMM tpmHandler: caGet1 failed"
	}
	set tpmError(CA) INVALID 
	set tpmData(CA) "Channel Access Failure: TPM Down?"

# Failed!
	set status 1
    } else {

# OK so far - will change if a channel doesn't connect.
	set tpmError(CA) NO_ALARM	
	set tpmData(CA) NO_ALARM

#
# last five elements are: *** caGet completed normally ***    
#
        set nvals [expr [llength $cagetvals] - 5]

        for {set i 0} {$i < $nvals} {incr i 3} {
	    set pv [lindex $cagetvals $i]
	    set classdata [lindex $cagetvals [expr $i + 1]]
	    set class [lindex [split $classdata :] 0]
	    set mydata [lindex [split $classdata :] 1]

	    set sevr [lindex $cagetvals [expr $i + 2]]

	    if {$tpmVerbose == 1} {
		puts "$pv: $class => $sevr, $mydata"
	    }

#
# Spurious pv names of "NOT" appear in caGet1 return if
# connection problems occur. The full error string is
#	*** NOT FOUND / NOT CONNECTED ***
# parsed as 
#	sevr pv class  sevr pv class sevr
#
	    if {$pv != "NOT"} {

#
# Set the severity to INVALID if there was a connection
# problem. Also flag the CA error value.
#
		if {$sevr == "***"} { 
		    set sevr INVALID 
		    set tpmError(CA) INVALID
		    set tpmData(CA) "Channel Access Failure: TPM Down?"
		}

#
# Update the tpmError entry - taking care to maximize
# the alarm severity if multiple native TPM alarms exist
# for the same error class.
#
		if {![info exists tpmError($class)]} {
		    set tpmError($class) $sevr
		    if { $sevr != "NO_ALARM" } { 
		        set tpmData($class) $mydata
		    } else {
			set tpmData($class) ""
		    }
		} else {
		    if {[lsearch $SEVR $sevr] > 
			[lsearch $SEVR $tpmError($class)]} {
		        set tpmError($class) $sevr
		    }
		    if { $sevr != "NO_ALARM" } {
		        set tpmData($class) "$mydata:$tpmData($class)"
		    }
		} 
	    } 
	} 

# OK
	set status 0
    } end else 

    set tsnow [getclock]
    set tpmData(timeStamp) $tsnow
    set tpmError(timeStamp) $tsnow

    incr tpmData(serialNum)
    incr tpmError(serialNum)

    return $status
}

#--------
# Dump the present errors to stdout.
#--------
proc dumpTPMError {} {
    global tpmData tpmError

    puts "Contents of TPM Data and Error Arrays"
    puts ""

    puts [format "%10s %10s %s" INDEX ERROR DATA]
    foreach d [array names tpmData] {
	if {$tpmError($d) != "NO_ALARM" } {
	    puts [format "%10s %10s %s" $d $tpmError($d) $tpmData($d)]
	}
    }

    return 0
}
