umask 000
#
cat tmicro_head.html > m1.html
echo "<b>Last updated on: " >> m1.html
date >> m1.html
echo "</b><p>" >> m1.html
caGet1 tmicro_m1 | ./mktable.awk >> m1.html
cat tmicro_tail.html >> m1.html
#
cat tmicro_head.html > m2.html
echo "<b>Last updated on: " >> m2.html
date >> m2.html
echo "</b><p>" >> m2.html
caGet1 tmicro_m2 | ./mktable.awk >> m2.html
cat tmicro_tail.html >> m2.html
#
cat tmicro_head.html > truss.html
echo "<b>Last updated on: " >> truss.html
date >> truss.html
echo "</b><p>" >> truss.html
caGet1 tmicro_truss | ./mktable.awk >> truss.html
cat tmicro_tail.html >> truss.html
#
