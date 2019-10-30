#xyplotting routine

global xopd xmin xmax xint
global y1opd ymin ymax yint y2opd y3opd
global .graph
global xd y1d y2d y3d xy1d xy2d xy3d
global begun stopped
global y2ok y3ok

#intialize values

set xmin $XY_XMIN 
set xmax $XY_XMAX
set xint $XY_XINT 
set xopd $XY_XOPD 
set ymin $XY_YMIN 
set ymax $XY_YMAX
set yint $XY_YINT 
set y1opd $XY_Y1PD
set y2opd $XY_Y2PD
set y3opd $XY_Y3PD

set maxny 3
set begun 0
set stopped 1
set y2ok 0
set y3ok 0

# create frame

wm title . $XY_TITLE 
frame .top -borderwidth 10
pack .top -side top

# create graph

blt_graph .graph -plotbackground white
.graph xaxis configure -min $xmin -max $xmax \
	-stepsize [expr ($xmax-$xmin)/$xint]
.graph yaxis configure -min $ymin -max $ymax \
	-stepsize [expr ($ymax-$ymin)/$yint]

# create entry fields

frame .lbls
label .lbls.opdes -text OPDES -padx 60
label .lbls.min -text Min -padx 30
label .lbls.max -text Max -padx 20
label .lbls.nint -text Nint -padx 20
pack .lbls.opdes -side left
pack .lbls.min -side left 
pack .lbls.max -side left
pack .lbls.nint -side left

# x: opdes, xmin, xmax, xint

frame .xinp
label .xinp.x -text "  X:" -padx 0
entry .xinp.xopd -width 20 -relief sunken \
	-textvariable xopd
entry .xinp.xmin -width 10 -relief sunken \
	-textvariable xmin
entry .xinp.xmax -width 10 -relief sunken \
	-textvariable xmax
entry .xinp.xint -width 5 -relief sunken \
	-textvariable xint

pack .xinp.x -side left
pack .xinp.xopd -side left
pack .xinp.xmin -side left
pack .xinp.xmax -side left
pack .xinp.xint -side left

bind .xinp.xopd <Return> UpdateXopd
bind .xinp.xmin <Return> UpdateXaxis
bind .xinp.xmax <Return> UpdateXaxis
bind .xinp.xint <Return> UpdateXaxis
#bind .xinp.xopd <Button-2> {set xopd [selection get]}

# y: opdes, xmin, xmax, xint

frame .yinp
label .yinp.y1 -text Y1: -padx 0
entry .yinp.y1opd -width 20 -relief sunken \
	-textvariable y1opd
entry .yinp.ymin -width 10 -relief sunken \
	-textvariable ymin
entry .yinp.ymax -width 10 -relief sunken \
	-textvariable ymax
entry .yinp.yint -width 5 -relief sunken \
	-textvariable yint

pack .yinp.y1 -side left
pack .yinp.y1opd -side left
pack .yinp.ymin -side left
pack .yinp.ymax -side left
pack .yinp.yint -side left

bind .yinp.y1opd <Return> UpdateYopd
bind .yinp.ymin <Return> UpdateYaxis
bind .yinp.ymax <Return> UpdateYaxis
bind .yinp.yint <Return> UpdateYaxis
#bind .yinp.y1opd <Button-2> {set y1opd [selection get]}

frame .y2inp
label .y2inp.y2 -text Y2: -padx 0
entry .y2inp.y2opd -width 20 -relief sunken \
	-textvariable y2opd
pack .y2inp.y2 -side left
pack .y2inp.y2opd -side left
bind .y2inp.y2opd <Return> UpdateYopd
#bind .y2inp.y2opd <Button-2> {set y2opd [selection get]}

frame .y3inp
label .y3inp.y3 -text Y3: -padx 0
entry .y3inp.y3opd -width 20 -relief sunken \
	-textvariable y3opd
pack .y3inp.y3 -side left
pack .y3inp.y3opd -side left
bind .y3inp.y3opd <Return> UpdateYopd
#bind .y3inp.y3opd <Button-2> {set y3opd [selection get]}

# create buttons

frame .binp 
button .binp.start -text Start -command Start -padx 10
button .binp.stop -text Stop -command Stop -padx 10
button .binp.clear -text Clear -command Clear -padx 10
button .binp.quit -text Quit -command exit -padx 10
pack .binp.start -side left -padx 10
pack .binp.stop -side left -padx 10
pack .binp.clear -side left -padx 10
pack .binp.quit -side right -padx 10

#pack items

pack .graph .lbls 
pack .xinp -fill x
pack .yinp  -fill x
pack .y2inp -fill x 
pack .y3inp -fill x
pack .binp -pady 10 -expand true

proc UpdateXopd {} {
	global xopd xmin xmax xint
}

proc UpdateXaxis {} {
	global xopd xmin xmax xint
	set xstp [expr int(($xmax-$xmin)/$xint)]
	.graph xaxis configure -min $xmin -max $xmax -stepsize $xstp
}

proc UpdateYopd {} {
	global y1opd ymin ymax yint y2opd y3opd

}

proc UpdateYaxis {} {
	global y1opd ymin ymax yint y2opd y3opd
	set ystp [expr int(($ymax-$ymin)/$yint)]
	.graph yaxis configure -min $ymin -max $ymax -stepsize $ystp
}


proc Start {} {
	global xopd xmin xmax xint
	global y1opd ymin ymax yint y2opd y3opd
	global .graph
	global xd y1d y2d y3d xy1d xy2d xy3d
	global begun stopped
	global y2ok y3ok
	set stopped 0
	if {$begun != 1} {
		set stat [pv linkw xd $xopd]
		set stat [pv linkw y1d $y1opd]
		pv getw xd
		pv getw y1d
		.graph element create xy1d -xdata $xd -ydata $y1d \
		-symbol plus -linewidth 0 -foreground red \
		-scale .75
		if {[string length $y2opd] != 0} {
			set stat [pv linkw y2d $y2opd]
			pv getw y2d
		.graph element create xy2d -xdata $xd -ydata $y2d \
		-symbol circle -linewidth 0 -foreground green \
		-scale .75
			set y2ok 1
		}
		if {[string length $y3opd] != 0} {
			set stat [pv linkw y3d $y3opd]
			pv getw y3d
		.graph element create xy3d -xdata $xd -ydata $y3d \
		-symbol diamond -linewidth 0 -foreground blue \
		-scale .75
			set y3ok 1
		}
		update
		set begun 1
	}
	while {$stopped == 0} {
		Get_Data
		after 500
	}
}

proc Get_Data {} {
	global .graph
	global xd y1d y2d y3d xy1d xy2d xy3d
	global y2ok y3ok
	pv getw xd
	pv getw y1d
	.graph element append xy1d  {$xd  $y1d}
		if {$y2ok == 1} {
			pv getw y2d
		.graph element append xy2d {$xd  $y2d}
		}
		if {$y3ok ==1} {
			pv getw y3d
		.graph element append xy3d {$xd  $y3d}
		}
        UpdateXaxis
	UpdateYaxis
	update 
}

proc Clear {} {
	global .graph
	global xd y1d y2d y3d xy1d xy2d xy3d
	global begun stopped
	global y2ok y3ok
	set x {}
	set y1 {}
	set begun 0
	set stopped 1
	.graph element delete xy1d
	if {$y2ok == 1} {
		.graph element delete xy2d
		set y2ok 0
		set y2 {}
	}
	if {$y3ok == 1} {
		.graph element delete xy3d
		set y3ok 0
		set y3 {}
	}
}

proc Stop {} {
	global begun stopped
	set stopped 1
}

