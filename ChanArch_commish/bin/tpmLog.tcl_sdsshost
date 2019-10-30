# Uses $EPICS_EXTENSIONS/bin/iris/et_tclsh

set machine "sdsshost.apo.nmsu.edu"
set port 4812

package require http

# command line parse
set arg [lindex $argv 0]
if {$arg != "start" && $arg != "restart" && $arg != "stop"} {
    puts "Invalid command: $arg"
    puts "Usage: tpmLog start|stop|restart"
    exit
}

if {$arg == "stop" || $arg =="restart"} {
    set url "http://$machine:$port/stop"

    if [catch {::http::geturl $url -timeout 5000} engine] {
        puts "There was an error getting the URL."
        puts "This probably means that the ArchiveEngine is not running."
        puts "The error reported is: "
        puts $engine
    } else {
        set status [ ::http::status $engine ]
        if { ! [ string match $status "ok" ] } {
	    puts "Cannot connect to ArchiveEngine at $url, status: $status"
            puts "The error reported is: "
            puts $engine
        } else {
	    puts "Shutdown of ArchiveEngine is ok"
        }	
    }
}

if {$arg == "start" || $arg == "restart"} {
    if {[pv linkw mjd tpm_MJD] != 0} {
        puts "ERROR: Can't find PV tpm_MJD. TPM down?"
        exit
    }

    set url "http://$machine:$port"

    if [catch {::http::geturl $url -timeout 5000} engine] {
        puts "There was an error getting the URL."
        puts "This probably means that the ArchiveEngine is not running."
        puts "The error reported is: "
        puts $engine
    } else {
        set status [ ::http::status $engine ]
        if { ! [ string match $status "ok" ] } {
	    puts "Cannot connect to ArchiveEngine at $url, status: $status"
            puts "The error reported is: "
            puts $engine
        } else {
	    exit
	}
    }
	
    pv get mjd
    puts "Current MJD is $mjd"
    set newdir [format "/mcptpm/%.0f" $mjd]
    if {![file exists $newdir]} {
	file mkdir $newdir
	file mkdir $newdir/cfg
    } else {
        puts "$newdir already exists"
    }
    exec chmod 777 $newdir 

    cd /p/tpmbase/ChanArch/config 
    foreach file [glob *.cfg] {
	file copy -force $file $newdir/$file
    }

    set lck [format "%s/archive_active.lck" $newdir]
    for {set i 0} {$i < 5} {incr i} {
        if {[file exists $lck]} {
	   puts "Try #$i: Waiting 30 seconds for lock file to clear: $lck"
	   after 30000
	}
    }
    puts "Starting ArchiveEngine"
    exec /p/tpmbase/ChanArch/bin/startEngine $newdir &
}

exit
