awk 'BEGIN {
print "<table border=1>";
print "<tr><th>DESCRIPTION</th>";
print "<th>degC</th>";
print "<th>PROCESS VARIABLE</th>";
print "<th>STATUS</th></tr>";
}
{
  if (NF > 1) {
	print "<tr><td>";
	for (i=2;i<NF;i++) printf("%s ",$i);
	print "</td><td align=right>"
	print $NF;
	print "</td><td>"
	print $1;
	print "</td><td>"
	if ($2 != "DUMMY" && $2 != "caGet") {
	if ($NF <= -200) { print "<b>BAD!</b>"; } 
	else if ($NF <= -50.0) { print " Patched "; }
	}
	print "</td></tr>";
  }
}
END {
print "</table>";
}'
