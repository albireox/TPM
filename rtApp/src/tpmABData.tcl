# Obtain Connie's interface routines for TPM archive
# data as well as the built-in casi things - mostly
# timestamp utilities.

source /p/tpmbase/bin/iris/tpmgetnew.tcl

#
# Convert month, day, year to MJD [from slaLib]
# Returns -1 on any error
#
proc cldj {iyt imt idt} {
    scan $iyt "%d" iy
    scan $imt "%d" im
    scan $idt "%d" id

    set mtab(0) 31 
    set mtab(1) 28 
    set mtab(2) 31 
    set mtab(3) 30 
    set mtab(4) 31 
    set mtab(5) 30 
    set mtab(6) 31 
    set mtab(7) 31 
    set mtab(8) 30 
    set mtab(9) 31 
    set mtab(10) 30 
    set mtab(11) 31

    if {$iy < -4699} { 
	puts "cldj: bad year: $iy < -4699"
	return -1 
    }
    if {$im < 1 || $im > 12} { 
	puts "cldj: bad month: $im < 1 || $im > 12"
        return -1 
    } 

    if {([expr $iy % 4] == 0) && 
	(([expr $iy % 100] != 0) || ([expr $iy % 400] == 0))} { 
	set mtab(1) 29 
    } 

    if {$id < 1 || $id > $mtab([expr $im - 1])} { 
        puts "cldj: bad day: $id < 1 || $id > $mtab([expr $im - 1])" 
	return -1 
    }

    set term0 [expr floor(($iy - (12 - $im)/10))]
    set term1 [expr floor((1461*($term0 + 4712))/4)]
    set term2 [expr floor((306* (($im + 9)%12) + 5)/10)]
    set term3 [expr floor(($term0 + 4900)/100)]
    set term4 [expr floor((3*$term3)/4)]

    set mjd [expr $term1 + $term2 - $term4 + $id - 2399904]

    return $mjd
}

    
# Make a list of all Allen-Bradley channels (174
# 16-bit words).

proc ABChannelList { } {
    for {set i 0} {$i < 32} {incr i} {lappend list32 [format "%2.2d" $i]}
    for {set i 0} {$i < 20} {incr i} {lappend list20 [format "%2.2d" $i]}
    for {set i 0} {$i < 8} {incr i} {lappend list8 [format "%2.2d" $i]}
    for {set i 0} {$i < 4} {incr i} {lappend list4 [format "%2.2d" $i]}
    for {set i 0} {$i < 2} {incr i} {lappend list2 [format "%2.2d" $i]}

    makeChanList ABList tpm_ABI1_ $list32
    makeChanList ABList tpm_ABO1_ $list32
    makeChanList ABList tpm_ABI2_ $list32
    makeChanList ABList tpm_ABO2_ $list32
    makeChanList ABList tpm_ABI3_ $list8
    makeChanList ABList tpm_ABI4_ $list8
    makeChanList ABList tpm_ABI5_ $list8
    makeChanList ABList tpm_ABI6_ $list2
    makeChanList ABList tpm_ABI7_ $list2
    makeChanList ABList tpm_ABI8_ $list2
    makeChanList ABList tpm_ABI9_ $list2
    makeChanList ABList tpm_ABI10_ $list2
    makeChanList ABList tpm_ABO11_ $list2
    makeChanList ABList tpm_ABO12_ $list2
    makeChanList ABList tpm_ABB3_ $list20
    makeChanList ABList tpm_ABB10_ $list4

    return $ABList
}

# Write the Allen-Bradley data out to a "Survey-standard
# Yanny parameter file" using the structure name "TPMABDATA".
#
#
#
proc ABDump {outfile cnt month day hms {year THISYEAR}} {
    set months [list Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec]

    if {[scan $hms %d:%d:%d"  h m s] != 3} {
	puts "Error: unable to extract HH:MM:SS from $hms."
	return
    }

    set M [expr [lsearch $months $month] + 1]
    if {$M == 0} {
        puts "Error: invalid month $month."
        puts "Choices are: $months."
        return
    } 

    if {[scan $day "%d" iday] != 1} {
	puts "Error: unable to extract numeric day from $day."
	return
    }
    if {$year != "THISYEAR"} {
        if {[catch {
        set tstart [values2stamp $year $M $iday $h $m $s]
	}]} {
	    puts "Error: conversion to timestamp failed."
            puts "Year = $year, Month = $month, Day = $iday."
	    puts "Hours = $h, Minutes = $m, Seconds = $s."
	    return
	}
    } else {
        set tsnow [stampNow]
        set secsnow [expr [s2s $tsnow] + 7*3600]
        stamp2values $tsnow NY NM ND Nh Nm Ns
	
        if {[catch {
        set tstart [values2stamp $NY $M $iday $h $m $s]
	}]} {
	    puts "Error: conversion to timestamp failed."
            puts "Year = $NY, Month = $month, Day = $iday."
	    puts "Hours = $h, Minutes = $m, Seconds = $s."
	    return
	}

        set secs [s2s $tstart]
	if {$secs > $secsnow} {
	    puts "Rolling date back to last year"
	    set NY [expr $NY - 1]
	    set tstart [values2stamp $NY $M $iday $h $m $s]
	}
	stamp2values $tstart year month iday h m s
     } 

    puts "Searching the archive at $tstart."	    
    set secs [s2s $tstart]

    set mdate [cldj $year $M $iday]
    if {$mdate < 0} {
	puts "Error: conversion to MJD failed."
        puts "Year = $year, Month = $month, Day = $iday."
	return
    }
    set mjd [format "%.0f" $mdate]
    if {$h > 17 || ($h == 17 && $m > 10)} { set mjd [expr $mjd + 1] }
    puts "We'll be using the archiver file for MJD $mjd."

# get the span of time coved by this MJD
    set tstampFirst [getChannelStart $mjd tpm_uptime]
    set secsFirst [s2s $tstampFirst]
    set tstampLast [getChannelEnd $mjd tpm_uptime]
    set secsLast [s2s $tstampLast]

# check for requested date being in this range
    if {$secs < $secsFirst || $secs > $secsLast} {
	puts "Error: requested date not in specified MJD"
	return
    } 

# adjust to make sure sample is as closest to specified h:m:s,
# knowing that the interlock data is scanned at 1Hz!

    set tstart [secs2stamp [expr [s2s $tstart] - 0.5]]
    set tend [secs2stamp [expr [s2s $tstart] + $cnt - 0.5]]

    flush stdout
    
    getSamplesSeconds $mjd [ABChannelList] 1 $outfile TPMABDATA $tstart $tend
}
    
