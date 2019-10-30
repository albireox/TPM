set machine "sdsshost.apo.nmsu.edu"
set port 4812

##############################
package require http

set url "http://$machine:$port"

if [catch {::http::geturl $url -timeout 2000} engine] {
    puts "There was an error getting the URL."
    puts "This probably means that the ArchiveEngine is not running."
    puts "The error reported is: "
    puts $engine
    puts " "
    puts "Trying to restart"
    exec /home/observer/bin/startEngine
	
} else {
    puts "Got the URL, Engine must be OK".	
}
exit
