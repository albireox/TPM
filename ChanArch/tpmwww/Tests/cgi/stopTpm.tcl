set machine "sdsshost.apo.nmsu.edu"
set port 4812

##############################
package require http

set url "http://$machine:$port/stop"

if [catch {::http::geturl $url -timeout 2000} engine] {
    puts "<h3>There was an error getting the URL.</h3>"
    puts "This probably means that the ArchiveEngine is not running."
    puts "The error reported is: "
    puts $engine
} else {
    set status [ ::http::status $engine ]
    if { ! [ string match $status "ok" ] } {
	puts "Cannot connect to ArchiveEngine at $url, status: $status"
    } else {
	puts "Shutdown of ArchiveEngine is ok"
    }	
}
exit
