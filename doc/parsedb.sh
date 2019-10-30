awk -F, 'BEGIN {
rname = "foo"; desc=""; dtyp="Soft Channel";
inp=""; out=""; scan="Passive";
}
{
    if (match($1,"record") > 0) {
	if (rname != "foo") {
	    printf "%s,%s,%s,%s,%s,%s\n", rname, desc, scan, dtyp, inp, out;
	}
	rname = $2;
	desc="none"; dtyp="Soft Channel"; inp="";out="";scan="Passive";
    }
    if (match($1,"DESC") > 0) desc=$2;
    if (match($1,"DTYP") > 0) dtyp=$2;
    if (match($1,"SCAN") > 0) scan=$2;
    if (match($1,"INP") > 0) inp=$2;
    if (match($1,"OUT") > 0) out=$2;
}' | 
tr -s ' ' | tr -d '{' | tr -d '"' | tr -d ')' | sed 's/ ,/,/g' | sed 's/, /,/g'
