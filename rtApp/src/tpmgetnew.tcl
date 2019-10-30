# built-in casi things; mostly tstamp utilities
source /p/epics/extensions/src/ChannelArchiver/casi/tcl/casiTools.tcl

###################################################
#//* <B>Syntax:</B> s2s stamp
#//
#// <B>Returns:</B> secs as double
#//
#// Convert stamp into seconds
#//

proc s2s { stamp } {
    stamp2values $stamp year month day hour minute second
    scan $second "%f" dsec
    set fullsec [ expr int($dsec) ]
    set fraction [ expr $dsec - $fullsec]
    set fullsec [ clock scan "$month/$day/$year $hour:$minute:$fullsec" ]
    expr $fullsec + $fraction
}

# print some basic things about a given channel on any given mjd.
# note that here, as in the rest of these procs, the nominal location of
# the TPM archive is hard-coded.  this should really get fixed; best would
# be a declared environment variable as part of "setup TPM"

proc channelInfo { mjd chName } {

    set archdir /mcptpm/${mjd}/freq_directory
# allocate an archive structure
    set arch [archive] 
# open the archive, stuffing the necessary info into an archive structure
    if {[$arch open $archdir] == 0} {
	error "can't open archive for $mjd"
    }
# allocate a channel structure
    set ch [channel]      
# open the requested channel for this archive, stuffing the channel struct
    if {[$arch findChannelByName $chName $ch] == 0} {
	error "can't find channel name $chName"
    }

    $arch findChannelByName $chName $ch
    puts "$archdir  $chName"
    puts "first data timestamp [$ch getFirstTime]"
    puts "last data timestamp [$ch getLastTime]"
}

# get the time of the first data entry for a specific channel on a specific mjd
proc getChannelStart { mjd chName } {

    set archdir /mcptpm/${mjd}/freq_directory
    set arch [archive] 
    if {[$arch open $archdir] == 0} {
	error "can't open archive for $mjd"
    }
    set ch [channel]      
    if {[$arch findChannelByName $chName $ch] == 0} {
	error "can't find channel name $chName"
    }

    $arch findChannelByName $chName $ch
    return [$ch getFirstTime]
}

# get the time of the last data entry for a specific channel on a specific mjd
proc getChannelEnd { mjd chName } {

    set archdir /mcptpm/${mjd}/freq_directory
    set arch [archive] 
    if {[$arch open $archdir] == 0} {
	error "can't open archive for $mjd"
    }
    set ch [channel]      
    if {[$arch findChannelByName $chName $ch] == 0} {
	error "can't find channel name $chName"
    }

    $arch findChannelByName $chName $ch
    return [$ch getLastTime]
}

#######
# Spreadsheet Context Procedures defined in casiTools.tcl:
#
# proc openSpreadsheetContext { archiveName start startStamp channelNameList
#								 contextVar valuesVar statiVar } 
# proc nextSpreadsheetContext { contextVar timeStampVar valuesVar statiVar } 
# proc closeSpreadsheetContext { contextVar } 
#######

#######
# Taken from spreadsheetExport.tcl:
# Helper routines to print value/status - modified
# to include Yanny parameter files format.
#
# values will be a list of values, elements might be {} if no value found.
# stati is a list of status strings, mostly useful for the case that value=={}
# because the channel was disconnected (IOC down)
# or archiving was disabled
#
# no Yanny parameter file support for this one!
proc printValuesAndStatus { fd time values stati { parfileStructname "" } } {
    if { $parfileStructName != "" } {
	    puts "printValuesAndStatus doesn't support Yanny parameter files."
	    return
	}
    puts -nonewline $fd "$time"
    foreach value $values status $stati {
        if { $value == {} } { set value "-" }
        if { $status != {} } { set status "($status)" }
        puts -nonewline $fd "\t$value\t$status"
    }
    puts $fd ""
}

proc printHeader { fd chNameList { parfileStructname "" } } {

    if {$parfileStructname != "" } {
# Write Yanny parameter file structure definition.
        puts $fd "typedef struct {"
        puts $fd "  char time\[22\];"
        foreach chname $chNameList {
	        regsub {\.} $chname _ elname
	        puts $fd "  double $elname;"
        }
        puts $fd "} ${parfileStructname};"
    } else {
# Write spreadsheet header.
        puts -nonewline $fd "; TAI              "
        foreach chname $chNameList {
            puts -nonewline $fd "\t$chname"
        }
        puts $fd ""
    }
}

proc printValues { fd time values stati { parfileStructname "" } } {
    if { $parfileStructname != "" } {
# For each sample, write the data
# write the parfile struct name, and the time
	    puts $fd ""
        puts -nonewline $fd "$parfileStructname [format \"%.22s\" $time]   "
    } else {
        puts -nonewline $fd [stampToTai $time]
    }

    foreach value $values status $stati {
        if { $value == {} } {
            puts -nonewline $fd "\t($status)"
        } else {
            puts -nonewline $fd "\t$value"
        }
    }
    puts $fd ""
}

#######
# get samples using the spreadsheet context and iterator.
#
# if parfileStructname == "" we use spreadsheet rather than
# Yanny parameter file output format.
#
######

proc getSamples { mjd chNameList outfile {parfileStructname ""} {tstamp0 0} {tstampN -1}} {

    if {$outfile != ""} {
	    set fd [open $outfile w]
    } else {
	    set fd stdout
    }

    set archiveName /mcptpm/${mjd}/freq_directory

    printHeader $fd $chNameList $parfileStructname

    set time $tstamp0
    set go [ openSpreadsheetContext $archiveName $time $chNameList context values stati ]

    if { $go } {
        while { $go } {
            printValues $fd $time $values $stati $parfileStructname
            if { $time > $tstampN } break
            set go [ nextSpreadsheetContext context time values stati ]
        }

        closeSpreadsheetContext context
    } else {
	puts "Error: Archive $archiveName - not present or $time not found"
    }

    if { $outfile != " " } { close $fd }
}

proc sstest { } {
   set tsnow [stampNow]
   set tsstart [secs2stamp [expr [s2s $tsnow] - 43200.0]]
   set tsend [secs2stamp [expr [s2s $tsstart] + 3600.0]]
   set clist [list tpm_ALENC1 tpm_AZENC1 tpm_ROCENC]

   getSamples 52626 $clist sstest.par SSTEST $tsstart $tsend
}

 
# write TPM data from channels chNameA and chNameB on the specified mjd to 
# either stdout or to outfile.  Start the file at the beginning unless tstamp0
# is specified, in which case start there, and go to the end unless tstampN
# is specified, in which cast stop there.  Get the data closest to 
# the times in a sequence, where the time sequence starts at either the 
# beginning of the file or tstamp0 and goes in dt increments.

proc getSamplesOld { mjd chNameA chNameB dt {outfile ""} {tstamp0 0} {tstampN -1}} {
# open outfile, if requested.  should check for success, properly speaking.   
    if {$outfile != ""} {
	set fd [open $outfile w]
    } else {
	set fd stdout
    }
# open achive
    set archdir /mcptpm/${mjd}/freq_directory
    set arch [archive] 
    if {[$arch open $archdir] == 0} {
	error "can't open archive for $mjd"
    }
# open channels
    set chA [channel]      
    if {[$arch findChannelByName $chNameA $chA] == 0} {
	error "can't find channel name $chNameA"
    }
    set chB [channel] 
    if {[$arch findChannelByName $chNameB $chB] == 0} {
	error "can't find channel name $chNameB"
    }
# allocate structures to read data values.
    set valA [value]
    set valB [value]
# get start and end tstamps, and convert to seconds.
    if {$tstamp0 == 0} {
	set tstamp0 [$chA getFirstTime]
    }
    set t0 [s2s $tstamp0]
    if {$tstampN == -1} {
	set tstampN [$chA getLastTime]
    }
    set tN [s2s $tstampN]

    puts $tstamp0
    puts $tstampN

    set t $t0
    while {$t<$tN} {
# get next time in our timesequence as a tstamp
	set stamp [secs2stamp $t]
# move in the channel structure to the data entry closest to our desired time
	$chA getValueNearTime $stamp $valA
	$chB getValueNearTime $stamp $valB
# read the values
	set A [$valA get]
	set B [$valB get]
	set diff [expr $A-$B]
# write out the data
	puts $fd "$stamp [format %12.2f $A] [format %12.2f $B] \
		[format %12.2f $diff]" 
# increment the time to the next desired reading
	set t [expr $t+$dt]
    }
    if {$outfile != ""} {	
	close $fd
    }
}

# various little utilities to make lists of tpm channel names.

# make listvar be a list of all the channel names that are 
# baserange0...baseraneN
# for example, if one wants all the M1 temperatures, say:
# makeChanList foo tpm_TM_M0B0Chans.VAL {A B C D E F G H}
# which will make foo hold the list of all module0, block0's  thermometer 
# names.  now do:
# makeChanList foo tpm_TM_M0B1Chans.VAL {A B C D E F G H}
# and foo will have module0, block1's thermometer names appended.
# do this for B2...B5 and you will have them all.
# or, look at the next proc.
proc makeChanList { _listvar base rangelist} {

    upvar $_listvar listvar
    foreach member $rangelist {
	set name ${base}$member
	lappend listvar $name
    }
    return $listvar
}

# append to listvar a list of all the module 0 thermometer TPM channel names.
proc allModule0 { _listvar } {

    upvar $_listvar listvar
    set base tpm_TM_M0B
    set baseend Chans.VAL
    set blocklist {0 1 2 3 4 5}
    set rangelist {A B C D E F G H}
    foreach block $blocklist {
	set blockbase ${base}${block}${baseend}
	makeChanList listvar $blockbase $rangelist
    }
    return $listvar
}

# append to listvar a list of all the TPM channel names that 
# have M2 thermometers
proc M2Temps { _listvar } {

    upvar $_listvar listvar
    set base tpm_TM_M1B
    set baseend Chans.VAL
    set blocklist {0 1 2}
    set rangelist {A B C D E F G H}
    foreach block $blocklist {
	set blockbase ${base}${block}${baseend}
	makeChanList listvar $blockbase $rangelist
    }
    return $listvar
}

# append to listvar a list of all the fork air thermomter channel names.
proc forkAirTemps { _listvar } {

    set base tpm_TM_M2B2Chans.VAL
    set rangelist {A C E G}
    upvar $_listvar listvar
    makeChanList listvar $base $rangelist
    return $listvar
}


# get TPM data for the channels in chNameList on the specified MJD, and write
# to outfile.  Start at tstamp0 or at the beginning of the archive file, and 
# go the end or to tstampN.  Write the data as a Survey-standard Yanny
# parameter file, and use the structure name parfileStructName.  Write the
# data out in a time series every dt seconds.  

proc getSamplesSeconds { mjd chNameList dt outfile {parfileStructname ""} {tstamp0 0} {tstampN -1}} {
    
    if {$outfile != ""} {
	set fd [open $outfile w]
    } else {
	set outfile stdout
    }

# add paramfile stuff
    puts $fd "typedef struct {"
#    puts $fd "  double time;"
    puts $fd "  char time\[22\];"
    foreach chname $chNameList {
	regsub {\.} $chname _ elname
	puts $fd "  double $elname;"
    }
    puts $fd "} ${parfileStructname};"
    puts $fd ""

# open archive and channels
    set archdir /mcptpm/${mjd}/freq_directory
    set arch [archive] 
    if {[$arch open $archdir] == 0} {
	error "can't open archive for $mjd"
    }
    set nch [llength $chNameList]
    foreach channel $chNameList {
	set ch [channel]      
	if {[$arch findChannelByName $channel $ch] == 0} {
	    error "can't find channel name $channel"
	}
	lappend chList $ch
    }
# get starttime and endtime
    set ch0 [lindex $chList 0]
    if {$tstamp0 == 0} {
	set tstamp0 [$ch0 getFirstTime]
    }
    set t0 [s2s $tstamp0]
    if {$tstampN == -1} {
	set tstampN [$ch0 getLastTime]
    }
    set tN [s2s $tstampN]
#    puts $tstamp0
#    puts $tstampN

# we want this sequentially in time, so find midnight.    
    set foo [split $tstamp0 " "]
    set date [lindex $foo 0]
    set midnight "$date 00:00:00"
    set midnight [s2s $midnight]

# allocate value structures to hold the data for each channel.  
# this is sloppy about keeping members of value and channel associated,
# and relies on you to keep the channel and value lists in the same order.
    for {set i 0} {$i < $nch} {incr i} {
	lappend valList [value]
    }

    set t $t0
    while {$t<=$tN} {
	set stamp [secs2stamp $t]
	set hours [expr ($t-$midnight)/3600.]
# write the parfile struct name, and the time
#	puts -nonewline $fd "$parfileStructname [format %2.4f $hours]   "
	puts -nonewline $fd "$parfileStructname [format \"%s\" $stamp]   "
# write the data for each channel
	for {set i 0} {$i < $nch} {incr i} {
	    [lindex $chList $i] getValueNearTime $stamp [lindex $valList $i]
	    set data [[lindex $valList $i] get]
	    puts -nonewline $fd "[format %3.2f $data]   "
	} 
	puts $fd " "
	set t [expr $t+$dt]
    }

# clean up allocated structures
    foreach ch $chList {
	rename $ch {}
    }

    foreach val $valList {
	rename $val {}
    }
    rename $arch {}

    if {$outfile != ""} {	
	close $fd
    }
}

# get many MJDs worth of tpm data from 4pm to 7am and write to parameter files
# specifiy filenameroot and the mjd will be appended to each outfile for you.  
proc getNightData {mjdlist filenameroot chanlist dt {parfileStructname ""}} {

    set testch [lindex $chanlist 0]
    
    foreach mjd $mjdlist {
	set filename ${mjd}_${filenameroot}.par
	set tstampStart [getChannelStart $mjd $testch]
	stamp2values $tstampStart year month day hour minute second
	set hour 00; set minute 00; set second 00;
	set midnight [s2s [values2stamp $year $month $day $hour $minute $second]]
	set fourpm [secs2stamp [expr $midnight + 16.0*3600.0]]
	set sevenam [secs2stamp [expr $midnight + 31.0*3600.0]]
	puts -nonewline "$mjd $fourpm $sevenam\r"
	flush stdout
	getSamplesSeconds $mjd $chanlist $dt $filename $parfileStructname $fourpm $sevenam
    }
}
