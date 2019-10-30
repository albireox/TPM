[schematic2]
uniq 234
[tools]
[detail]
w 152 2258 100 0 n#34 hwin.hwin#33.in 176 2256 176 2256 eais.ai.INP
w 1764 697 100 0 n#228 hwout.hwout#227.outp 1794 686 1794 686 estringouts.soa.OUT
w 1761 1179 100 0 n#224 hwout.hwout#222.outp 1792 1168 1791 1168 1791 1167 1791 1167 elongouts.debug.OUT
w 2600 372 100 0 n#94 elongouts.lo2.OUT 2624 368 2624 368 hwout.hwout#99.outp
w 993 2027 100 0 n#217 hwin.hwin#215.in 1022 2016 1024 2016 ewaves.waveform.INP
w 180 -181 100 0 n#210 hwin.hwin#209.in 176 -177 176 -176 estringins.si3.INP
w 152 50 100 0 n#197 estringins.si2.INP 176 48 176 48 hwin.hwin#199.in
w 1762 2290 100 0 n#206 elongouts.timeout.OUT 1792 2288 1792 2288 hwout.hwout#168.outp
w 4034 1090 100 0 n#204 hwout.hwout#203.outp 4064 1088 4064 1088 elongouts.mbboErr2.OUT
w 152 1394 100 0 n#194 eais.ai4.INP 176 1392 176 1392 hwin.hwin#196.in
w 1765 -177 100 0 n#188 hwout.hwout#189.outp 1795 -179 1795 -179 estringouts.so3.OUT
w 4040 1410 100 0 n#185 embbos.mbboErr.OUT 4064 1408 4064 1408 hwout.hwout#187.outp
w 3336 2148 100 0 n#182 elongouts.lo4.OUT 3360 2144 3360 2144 hwout.hwout#184.outp
w 2600 -428 100 0 n#179 elongouts.lo3.OUT 2624 -432 2624 -432 hwout.hwout#181.outp
w 1768 1490 100 0 n#172 hwout.hwout#173.outp 1792 1488 1792 1488 estringouts.readCmt.OUT
w 1768 1954 100 0 n#169 eaos.slope.OUT 1792 1952 1792 1952 hwout.hwout#171.outp
w 2594 658 100 0 n#165 hwout.hwout#164.outp 2624 656 2624 656 ebos.bo2.OUT
w 2594 178 100 0 n#161 embbos.mbbo2.OUT 2624 176 2624 176 hwout.hwout#163.outp
w 2626 1362 100 0 n#160 hwout.hwout#158.outp 2656 1360 2656 1360 mbbodirects.mbboD.OUT
w 1768 1714 100 0 n#153 hwout.hwout#151.outp 1792 1712 1792 1712 estringouts.writeCmt.OUT
w 152 1618 100 0 n#145 eais.ai3.INP 176 1616 176 1616 hwin.hwin#147.in
w 152 1810 100 0 n#142 eais.ai2.INP 176 1808 176 1808 hwin.hwin#144.in
w 152 2034 100 0 n#139 eais.ai1.INP 176 2032 176 2032 hwin.hwin#141.in
w 152 722 100 0 n#136 elongins.li1.INP 176 720 176 720 hwin.hwin#138.in
w 2594 -110 100 0 n#135 hwout.hwout#133.outp 2624 -112 2624 -112 embbos.mbbo3.OUT
w 1768 879 100 0 n#42 hwout.hwout#124.outp 1792 877 1792 877 estringouts.so.OUT
w 1771 399 100 0 n#112 hwout.hwout#123.outp 1795 397 1795 397 estringouts.so1.OUT
w 1771 143 100 0 n#115 hwout.hwout#122.outp 1795 141 1795 141 estringouts.so2.OUT
w 152 274 100 0 n#47 hwin.hwin#118.in 176 272 176 272 estringins.si1.INP
w 4040 1634 100 0 n#92 elongouts.loErr.OUT 4064 1632 4064 1632 hwout.hwout#98.outp
w 2600 1938 100 0 n#88 elongouts.lo.OUT 2624 1936 2624 1936 hwout.hwout#96.outp
w 152 946 100 0 n#82 elongins.li.INP 176 944 176 944 hwin.hwin#91.in
w 4040 1890 100 0 n#77 ebos.boErr.OUT 4064 1888 4064 1888 hwout.hwout#87.outp
w 2600 2386 100 0 n#65 hwout.hwout#66.outp 2624 2384 2624 2384 eaos.ao.OUT
w 152 530 100 0 n#64 hwin.hwin#71.in 176 528 176 528 embbis.mbbi.INP
w 152 1138 100 0 n#36 ebis.bi.INP 176 1136 176 1136 hwin.hwin#72.in
w 2600 2162 100 0 n#32 ebos.bo.OUT 2624 2160 2624 2160 hwout.hwout#83.outp
w 2600 1746 100 0 n#30 embbos.mbbo.OUT 2624 1744 2624 1744 hwout.hwout#84.outp
w 2600 946 100 0 n#53 hwout.hwout#51.outp 2624 944 2624 944 eaos.ao2.OUT
w 4040 2146 100 0 n#17 hwout.hwout#12.outp 4064 2144 4064 2144 eaos.aoErr.OUT
s 1598 477 100 0 also ignores responses
s 2448 2576 100 0 OUTPUT RECORDS
s 2416 2544 100 0 which have a response
s 2416 1111 100 0 which have not response
s 2416 1072 100 0 this will fail in a loopback mode
s 2432 512 100 0 which converts to a bit stream
s 2432 791 100 0 which ignores the response
s 2432 -9 100 0 which convert int->float
s 1600 2480 100 0 RECORDS WHICH CONTROL
s 1600 2448 100 0 LINK ASPECTS
s 1600 2407 100 0 timeout control max time to wait for a response
s 1600 2119 100 0 slope controls ao and ai conversions
s 1600 2087 100 0 rval->%f, %f->rval
s 1600 2055 100 0 refer to header of devAscii.c
s 1600 1799 100 0 writeCmt = write command terminator
s 1600 1575 100 0 readCmt = read command terminator
s 3872 2391 100 0 OUTPUT RECORDS
s 3872 2359 100 0 which are faulty
s 3872 2304 100 0 data type spec invalid in response format
s 3872 2272 100 0 only assignement suppression is valid
s 3872 1767 100 0 serial link name not specified
s 2432 -288 100 0 which converts to ascii chars
s 3168 2288 100 0 defaults to integer output
s 3872 1495 100 0 %*k or &nk is only valid as first chars
s 1600 1044 100 0 STRING OUTPUT RECORDS
s 1600 964 100 0 has a prompt and response
s 1601 506 100 0 no prompt other than the writeCTM 
s 1603 260 100 0 prompt but no response
s 1603 237 100 0 this will fail in a loopback mode
s 1603 4 100 0 no prompt and no response
s 1603 -35 100 0 writing the null string to this will
s 1603 -67 100 0 cause the write command terminator to be output
s 1603 -99 100 0 this will fail in a loopback mode
s 240 2510 100 0 INPUT RECORDS
s 240 2448 100 0 as well as records with default response types 
s 240 2407 100 0 are not shown as they cannot be demonstrated 
s 240 2311 100 0 basic with prompt and float input
s 240 2071 100 0 input is integer so it is converted to float
s 240 1847 100 0 input is binary converted to float
s 240 1447 100 0 input defaults to float
s 240 1424 100 0 the %4k is necessary in loopback mode
s 240 1207 100 0 more input records with various formats
s 240 327 100 0 entire echo is the string in val
s 240 103 100 0 only part of echo is string in val
s 3872 1207 100 0 %*b is not valid!
s 2512 1575 100 0 beware that a bug exists in mbbo direct
s 2512 1543 100 0 record init. You must set OMSL to supervisory
s 2592 1520 100 0 prior to the record's use even if the
s 2592 1488 100 0 default is supervisory
s 240 -121 100 0 val is used for output and input
s 241 -145 100 0 this provides for modifiable output strings
s 898 2230 100 0 the waveform record is used as a long
s 898 2205 100 0 stringIn record. this is tricky to test
s 898 2176 100 0 what I do is connect the cpu port to
s 898 2150 100 0 a sun serial port and tip to that port;
s 899 2126 100 0 trigger the record; then dump a reply
s 901 2101 100 0 in from the tip port - this must be
s 901 2076 100 0 fast so paste from a text editor or whatever
s 1601 1286 100 0 debug enables printf of all i/o in driver
s 240 2480 100 0 records for strings spanning multiple lines
s 240 2368 100 0 in a serial line loopback mode (need an actual device or process)
[cell use]
use keck 3359 -377 100 0 keck#233
xform 0 3430 -224
p 4176 -32 100 1024 0 set10:TEL
p 3804 -208 -150 1024 -1 set2:
p 3596 -318 150 1024 -1 set3:
p 3686 -319 150 1024 -1 set4:
p 4272 -304 150 1024 -1 set5:drvAscii sample database (can be used with sio loopback)
p 3872 -512 100 1024 -1 set6:5/12/97
p 4176 -512 100 1024 -1 set7:Al Honey
p 4256 -464 150 1024 -1 set8:1
p 4352 -464 150 1024 -1 set9:1
use bd200tr -368 -648 -100 0 frame
xform 0 2272 1056
use hwin 854 1976 100 0 hwin#215
xform 0 926 2016
p 766 1966 100 0 -1 val(in):@/tyCo/1 <waveform><%s>
use hwin 8 1576 100 0 hwin#147
xform 0 80 1616
p -144 1566 100 0 -1 val(in):@/tyCo/1 <ai3 1><ai3 %b>
use hwin 8 1768 100 0 hwin#144
xform 0 80 1808
p -144 1758 100 0 -1 val(in):@/tyCo/1 <ai2 10111101><ain %8b>
use hwin 8 1992 100 0 hwin#141
xform 0 80 2032
p -144 1982 100 0 -1 val(in):@/tyCo/1 <ai1 1><ai1 %d>
use hwin 8 2216 100 0 hwin#33
xform 0 80 2256
p -144 2206 100 0 -1 val(in):@/tyCo/1 <ai 1><ai %f>
use hwin 8 488 100 0 hwin#71
xform 0 80 528
p -160 478 100 0 -1 val(in):@/tyCo/1 <mbbi 15><mbbi %d>
use hwin 8 1096 100 0 hwin#72
xform 0 80 1136
p -144 1086 100 0 -1 val(in):@/tyCo/1 <bin 0><bin %d>
use hwin 8 904 100 0 hwin#91
xform 0 80 944
p -144 894 100 0 -1 val(in):@/tyCo/1 <lin 0><lin %d>
use hwin 8 232 100 0 hwin#118
xform 0 80 272
p -80 222 100 0 -1 val(in):@/tyCo/1 <strin1 >
use hwin 8 680 100 0 hwin#138
xform 0 80 720
p -144 670 100 0 -1 val(in):@/tyCo/1 <lin 0><%*3s %d>
use hwin 8 1352 100 0 hwin#196
xform 0 80 1392
p -144 1342 100 0 -1 val(in):@/tyCo/1 <ai4 1><%4k%b>
use hwin 8 8 100 0 hwin#199
xform 0 80 48
p -80 -2 100 0 -1 val(in):@/tyCo/1 <stringin2><st%s>
use hwin 8 -217 100 0 hwin#209
xform 0 80 -177
p -80 -227 100 0 -1 val(in):@/tyCo/1 <prompt %s><%s>
use eais 200 1512 200 0 ai3
xform 0 304 1584
p 62 1704 100 0 0 DTYP:Ascii SIO
p -80 1454 100 0 0 LINR:NO CONVERSION
p 256 1550 100 0 -1 SCAN:Passive
p -80 1084 100 0 0 typ(INP):path
use eais 200 1704 200 0 ai2
xform 0 304 1776
p 62 1896 100 0 0 DTYP:Ascii SIO
p -80 1646 100 0 0 LINR:NO CONVERSION
p 256 1742 100 0 -1 SCAN:Passive
p -80 1276 100 0 0 typ(INP):path
use eais 200 1928 200 0 ai1
xform 0 304 2000
p 62 2120 100 0 0 DTYP:Ascii SIO
p -80 1870 100 0 0 LINR:NO CONVERSION
p 256 1966 100 0 -1 SCAN:Passive
p -80 1500 100 0 0 typ(INP):path
use eais 200 2152 200 0 ai
xform 0 304 2224
p 62 2344 100 0 0 DTYP:Ascii SIO
p -80 2094 100 0 0 LINR:NO CONVERSION
p 256 2190 100 0 -1 SCAN:Passive
p -80 1724 100 0 0 typ(INP):path
use eais 200 1288 200 0 ai4
xform 0 304 1360
p 62 1480 100 0 0 DTYP:Ascii SIO
p -80 1230 100 0 0 LINR:NO CONVERSION
p 256 1326 100 0 -1 SCAN:Passive
p -80 860 100 0 0 typ(INP):path
use hwout 1818 646 100 0 hwout#227
xform 0 1890 686
p 1890 636 100 0 -1 val(outp):@/tyCo/1 <so><%*s>
use hwout 2648 2344 100 0 hwout#66
xform 0 2720 2384
p 2720 2334 100 0 -1 val(outp):@/tyCo/1 <ao %f><ao %*f>
use hwout 2648 904 100 0 hwout#51
xform 0 2720 944
p 2720 894 100 0 -1 val(outp):@/tyCo/1 <ao2 %f>
use hwout 4088 2104 100 0 hwout#12
xform 0 4160 2144
p 4160 2094 100 0 -1 val(outp):@/tyCo/1 <aoErr %f><aoErr %f>
use hwout 2648 2120 100 0 hwout#83
xform 0 2720 2160
p 2720 2110 100 0 -1 val(outp):@/tyCo/1 <bo %d ab><bo %*d ab>
use hwout 2648 1704 100 0 hwout#84
xform 0 2720 1744
p 2704 1694 100 0 -1 val(outp):@/tyCo/1 <mbbo %d><mbbo %*d>
use hwout 4088 1848 100 0 hwout#87
xform 0 4160 1888
p 4160 1838 100 0 -1 val(outp):@/tyCo/1 <bout %d><%5k%d>
use hwout 2648 1896 100 0 hwout#96
xform 0 2720 1936
p 2720 1886 100 0 -1 val(outp):@/tyCo/1 <lo %d><lo %*d>
use hwout 4088 1592 100 0 hwout#98
xform 0 4160 1632
p 4160 1582 100 0 -1 val(outp):<loErr %d><%*k>
use hwout 2648 328 100 0 hwout#99
xform 0 2720 368
p 2734 296 100 0 0 typ(outp):val
p 2720 318 100 0 -1 val(outp):@/tyCo/1 <lo2 %16b><lo2 %*s>
use hwout 1819 101 100 0 hwout#122
xform 0 1891 141
p 1891 91 100 0 -1 val(outp):@/tyCo/1 <strout ><strout %*s>
use hwout 1819 357 100 0 hwout#123
xform 0 1891 397
p 1891 347 100 0 -1 val(outp):@/tyCo/1 <><%*k> 
use hwout 1816 837 100 0 hwout#124
xform 0 1888 877
p 1888 827 100 0 -1 val(outp):@/tyCo/1 <so><so>
use hwout 2648 -152 100 0 hwout#133
xform 0 2720 -112
p 2704 -162 100 0 -1 val(outp):@/tyCo/1 <mbbo %f><mbbo %*f>
use hwout 1816 1672 100 0 hwout#151
xform 0 1888 1712
p 1888 1662 100 0 -1 val(outp):@/tyCo/1 <writeCmt %s>
use hwout 2680 1320 100 0 hwout#158
xform 0 2752 1360
p 2736 1310 100 0 -1 val(outp):@/tyCo/1 <mbboD %d><mbboD %*d>
use hwout 2648 136 100 0 hwout#163
xform 0 2720 176
p 2704 126 100 0 -1 val(outp):@/tyCo/1 <mbbo %4b><mbbo %*s>
use hwout 2648 616 100 0 hwout#164
xform 0 2720 656
p 2734 584 100 0 0 typ(outp):val
p 2720 606 100 0 -1 val(outp):@/tyCo/1 <bo2 %d><%*k>
use hwout 1816 2248 100 0 hwout#168
xform 0 1888 2288
p 1888 2238 100 0 -1 val(outp):@/tyCo/1 <timeout>
use hwout 1816 1912 100 0 hwout#171
xform 0 1888 1952
p 1888 1902 100 0 -1 val(outp):@/tyCo/1 <slope>
use hwout 1816 1448 100 0 hwout#173
xform 0 1888 1488
p 1888 1438 100 0 -1 val(outp):@/tyCo/1 <readCmt %s>
use hwout 2648 -472 100 0 hwout#181
xform 0 2720 -432
p 2734 -504 100 0 0 typ(outp):val
p 2720 -482 100 0 -1 val(outp):@/tyCo/1 <lo3 %4c><lo3 %*S>
use hwout 3384 2104 100 0 hwout#184
xform 0 3456 2144
p 3470 2072 100 0 0 typ(outp):val
p 3456 2094 100 0 -1 val(outp):@/tyCo/1 <lo4 ><lo4 %*s>
use hwout 4088 1368 100 0 hwout#187
xform 0 4160 1408
p 4144 1358 100 0 -1 val(outp):@/tyCo/1 <mbboErr %d><mbboErr %*k>
use hwout 1819 -219 100 0 hwout#189
xform 0 1891 -179
p 1891 -229 100 0 -1 val(outp):@/tyCo/1 
use hwout 4088 1048 100 0 hwout#203
xform 0 4160 1088
p 4174 1016 100 0 0 typ(outp):val
p 4160 1038 100 0 -1 val(outp):@/tyCo/1 <mbboErr2 %16b><mbboErr2 %*b>
use hwout 1816 1128 100 0 hwout#222
xform 0 1888 1168
p 1902 1096 100 0 0 typ(outp):val
p 1888 1118 100 0 -1 val(outp):@/tyCo/1 <debug>
use estringouts 1554 606 200 0 soa
xform 0 1666 702
p 1890 716 100 0 -1 DTYP:Ascii SIO
p 1474 508 100 0 0 OMSL:closed_loop
p 1474 540 100 0 0 PINI:NO
p 1618 732 100 0 -1 SCAN:Passive
p 1506 137 100 0 0 typ(OUT):path
use estringouts 1555 61 200 0 so2
xform 0 1667 157
p 1891 171 100 0 -1 DTYP:Ascii SIO
p 1475 -37 100 0 0 OMSL:closed_loop
p 1475 -5 100 0 0 PINI:NO
p 1619 187 100 0 -1 SCAN:Passive
p 1507 -408 100 0 0 typ(OUT):path
use estringouts 1555 317 200 0 so1
xform 0 1667 413
p 1891 427 100 0 -1 DTYP:Ascii SIO
p 1475 219 100 0 0 OMSL:closed_loop
p 1475 251 100 0 0 PINI:NO
p 1619 443 100 0 -1 SCAN:Passive
p 1507 -152 100 0 0 typ(OUT):path
use estringouts 1552 797 200 0 so
xform 0 1664 893
p 1888 907 100 0 -1 DTYP:Ascii SIO
p 1472 699 100 0 0 OMSL:closed_loop
p 1472 731 100 0 0 PINI:NO
p 1616 923 100 0 -1 SCAN:Passive
p 1504 328 100 0 0 typ(OUT):path
use estringouts 1552 1632 200 0 writeCmt
xform 0 1664 1728
p 1888 1742 100 0 -1 DTYP:Ascii SIO
p 1472 1534 100 0 0 OMSL:closed_loop
p 1472 1566 100 0 0 PINI:NO
p 1616 1758 100 0 -1 SCAN:Passive
p 1504 1163 100 0 0 typ(OUT):path
use estringouts 1552 1408 200 0 readCmt
xform 0 1664 1504
p 1888 1518 100 0 -1 DTYP:Ascii SIO
p 1472 1310 100 0 0 OMSL:closed_loop
p 1472 1342 100 0 0 PINI:NO
p 1616 1534 100 0 -1 SCAN:Passive
p 1504 939 100 0 0 typ(OUT):path
use estringouts 1555 -259 200 0 so3
xform 0 1667 -163
p 1891 -149 100 0 -1 DTYP:Ascii SIO
p 1475 -357 100 0 0 OMSL:closed_loop
p 1475 -325 100 0 0 PINI:NO
p 1619 -133 100 0 -1 SCAN:Passive
p 1507 -728 100 0 0 typ(OUT):path
use elongouts 2392 1880 200 0 lo
xform 0 2496 1968
p 2720 1966 100 0 -1 DTYP:Ascii SIO
p 2448 1934 100 0 -1 SCAN:Passive
p 2208 1740 100 0 0 typ(DOL):path
use elongouts 3832 1576 200 0 loErr
xform 0 3936 1664
p 4160 1662 100 0 -1 DTYP:Ascii SIO
p 3888 1630 100 0 -1 SCAN:Passive
p 3648 1436 100 0 0 typ(DOL):path
use elongouts 2392 312 200 0 lo2
xform 0 2496 400
p 2720 398 100 0 -1 DTYP:Ascii SIO
p 2448 366 100 0 -1 SCAN:Passive
p 2208 172 100 0 0 typ(DOL):path
use elongouts 2392 -488 200 0 lo3
xform 0 2496 -400
p 2720 -402 100 0 -1 DTYP:Ascii SIO
p 2448 -434 100 0 -1 SCAN:Passive
p 2208 -628 100 0 0 typ(DOL):path
use elongouts 3128 2088 200 0 lo4
xform 0 3232 2176
p 3456 2174 100 0 -1 DTYP:Ascii SIO
p 3184 2142 100 0 -1 SCAN:Passive
p 2944 1948 100 0 0 typ(DOL):path
use elongouts 3832 1032 200 0 mbboErr2
xform 0 3936 1120
p 4160 1118 100 0 -1 DTYP:Ascii SIO
p 3888 1086 100 0 -1 SCAN:Passive
p 3648 892 100 0 0 typ(DOL):path
use elongouts 1560 2232 200 0 timeout
xform 0 1664 2320
p 1888 2318 100 0 -1 DTYP:Ascii SIO
p 1616 2286 100 0 -1 SCAN:Passive
p 1376 2092 100 0 0 typ(DOL):path
use elongouts 1559 1111 200 0 debug
xform 0 1663 1199
p 1887 1197 100 0 -1 DTYP:Ascii SIO
p 1615 1165 100 0 -1 SCAN:Passive
p 1375 971 100 0 0 typ(DOL):path
use ewaves 1031 1873 200 0 waveform
xform 0 1152 1984
p 836 2008 100 0 -1 DTYP:Ascii SIO
p 1152 1902 100 0 0 NELM:100
p 1099 2018 100 0 -1 SCAN:Passive
p 928 1274 100 0 0 typ(INP):path
use estringins 200 168 200 0 si1
xform 0 304 240
p -16 302 100 0 -1 DTYP:Ascii SIO
p 256 270 100 0 -1 SCAN:Passive
use estringins 200 -56 200 0 si2
xform 0 304 16
p -16 78 100 0 -1 DTYP:Ascii SIO
p 256 46 100 0 -1 SCAN:Passive
use estringins 200 -280 200 0 si3
xform 0 304 -208
p -16 -146 100 0 -1 DTYP:Ascii SIO
p 256 -178 100 0 -1 SCAN:Passive
use embbos 2392 1656 200 0 mbbo
xform 0 2496 1744
p 2720 1774 100 0 -1 DTYP:Ascii SIO
p 2448 1710 100 0 -1 SCAN:Passive
p 2112 1775 100 0 0 typ(DOL):path
p 2177 1327 100 0 0 typ(OUT):path
use embbos 2392 -200 200 0 mbbo3
xform 0 2496 -112
p 2720 -82 100 0 -1 DTYP:Ascii SIO
p 2448 -34 100 0 -1 SCAN:Passive
p 2112 -81 100 0 0 typ(DOL):path
p 2177 -529 100 0 0 typ(OUT):path
use embbos 2392 88 200 0 mbbo2
xform 0 2496 176
p 2720 206 100 0 -1 DTYP:Ascii SIO
p 2448 142 100 0 -1 SCAN:Passive
p 2112 207 100 0 0 typ(DOL):path
p 2177 -241 100 0 0 typ(OUT):path
use embbos 3832 1320 200 0 mbboErr
xform 0 3936 1408
p 4160 1438 100 0 -1 DTYP:Ascii SIO
p 3888 1374 100 0 -1 SCAN:Passive
p 3552 1439 100 0 0 typ(DOL):path
p 3617 991 100 0 0 typ(OUT):path
use eaos 2384 2304 200 0 ao
xform 0 2496 2416
p 2720 2414 100 0 -1 DTYP:Ascii SIO
p 2112 2398 100 0 0 OMSL:supervisory
p 2448 2382 100 0 -1 SCAN:Passive
p 2112 2028 100 0 0 typ(OUT):path
use eaos 2384 864 200 0 ao2
xform 0 2496 976
p 2720 974 100 0 -1 DTYP:Ascii SIO
p 2112 958 100 0 0 OMSL:supervisory
p 2448 942 100 0 -1 SCAN:Passive
p 2112 588 100 0 0 typ(OUT):path
use eaos 3824 2064 200 0 aoErr
xform 0 3936 2176
p 4160 2174 100 0 -1 DTYP:Ascii SIO
p 3552 2158 100 0 0 OMSL:supervisory
p 3888 2142 100 0 -1 SCAN:Passive
p 3552 1788 100 0 0 typ(OUT):path
use eaos 1552 1872 200 0 slope
xform 0 1664 1984
p 1888 1982 100 0 -1 DTYP:Ascii SIO
p 1280 1966 100 0 0 OMSL:supervisory
p 1616 1950 100 0 -1 SCAN:Passive
p 1280 1596 100 0 0 typ(OUT):path
use mbbodirects 2392 1208 200 0 mbboD
xform 0 2512 1408
p 2736 1390 100 0 -1 DTYP:Ascii SIO
p 2448 1358 100 0 -1 SCAN:Passive
p 2720 782 100 0 0 typ(OUT):path
use elongins 200 840 200 0 li
xform 0 304 912
p -16 974 100 0 -1 DTYP:Ascii SIO
p 256 878 100 0 -1 SCAN:Passive
p -80 540 100 0 0 typ(INP):path
use elongins 200 616 200 0 li1
xform 0 304 688
p -16 750 100 0 -1 DTYP:Ascii SIO
p 256 654 100 0 -1 SCAN:Passive
p -80 316 100 0 0 typ(INP):path
use ebos 2392 600 200 0 bo2
xform 0 2496 688
p 2720 686 100 0 -1 DTYP:Ascii SIO
p 2448 654 100 0 -1 SCAN:Passive
p 2048 268 100 0 0 typ(OUT):path
use ebos 2392 2104 200 0 bo
xform 0 2496 2192
p 2704 2190 100 0 -1 DTYP:Ascii SIO
p 2448 2158 100 0 -1 SCAN:Passive
p 2048 1772 100 0 0 typ(OUT):path
use ebos 3832 1832 200 0 boErr
xform 0 3936 1920
p 4160 1934 100 0 -1 DTYP:Ascii SIO
p 3872 1998 100 0 -1 SCAN:Passive
p 3488 1500 100 0 0 typ(OUT):path
use embbis 200 424 200 0 mbbi
xform 0 304 496
p -16 558 100 0 -1 DTYP:Ascii SIO
p 256 462 100 0 -1 SCAN:Passive
p -112 479 100 0 0 typ(INP):path
use ebis 200 1032 200 0 bi
xform 0 304 1104
p -16 1166 100 0 -1 DTYP:Ascii SIO
p 256 1070 100 0 -1 SCAN:Passive
p -48 604 100 0 0 typ(INP):path
[comments]
