#
# TPM IOC2 Startup Script
# Version: 28 October 2004 
# Peregrine M. McGehee/LANL
#
#
#       Set up our network environment.
#
sysClkRateSet(60)

cd "/p/tpmbase/iocBoot/ioctpm2"
#
hostAdd ("sdsshost.apo.nmsu.edu", "192.41.211.171")
hostAdd("sampepys", "192.41.211.1")
routeAdd("0", "sampepys")
#
# Mount /p from sdsshost so we can dump debug traces,
# and /home as it is needed to load
# the mcpbase code
#
nfsMount("sdsshost.apo.nmsu.edu", "/p", "/p")
nfsMount("sdsshost.apo.nmsu.edu", "/home", "/home")
#
# Add user vxboot (pid 5036, gid 5530) to group products
#
au_p = malloc(4); *au_p = 4525		/* group products */
nfsAuthUnixSet "sdsshost.apo.nmsu.edu", 5036, 5530, 1, au_p

dbgInit()

#
# Load Ron's tracing tools from vx_tools (RHL)
#
#ld < /p/mcpbase/vx_tools/objects/trace.mv167.o
#ld < /p/mcpbase/vx_tools/lib/vxt.mv167.o

#
# Load murmur
#
#ld < /p/mcpbase/vx_tools/objects/dvx_var_dvx.mv167.o
#ld < /p/mcpbase/murmur_client_vx/lib/muruser.m68040.o

#
# Load taskSwitchHook utils for the use of the tracer
#
#ld < /p/mcpbase/util/dscTrace.o
#trace_limPerTimPeriod = 30	/* control throttling of murmur messages */

#
#       Setup the environment.
#
shellPromptSet("[tpm2:~]$ ")
tyBackspaceSet(0x08)

#
#
# Load slaLib
#
ld < ../../bin/mv162/slaLib

#
# Load EPICS IOC core and application
#
ld < ../../bin/mv162/iocCore
ld < ../../bin/mv162/seq
ld(0, 1, "../../bin/mv162/tpmcommonLib")

IpDevCreate
drvSerialInit
# Default is 9600 baud
drvSerialSetBaudRate("/tyCo/3", 38400)
drvSerialSetBaudRate("/tyCo/4", 38400)
drvSerialSetBaudRate("/tyCo/5", 2400)
drvSerialSetBaudRate("/tyCo/6", 2400)
drvSerialSetBaudRate("/tyCo/9", 2400)

ld(0, 1, "../../bin/mv162/tpmrtLib")
#
putenv("EPICS_TS_MIN_WEST=0")
# Get NTP time from utc-time.apo.nmsu.edu (galileo)
putenv("EPICS_TS_NTP_INET=192.41.211.40")

# Create tnet device.
hostAdd("monts1", "192.41.211.121")           
# Interface scanned once every 10 seconds.
#tnetDevCreateWithTimeout("/pty/humid.", "monts1", 2500, 15.0)      
tnetDevCreate("/pty/humid.", "monts1", 2500)      

#TSdriverDebug=5
TSconfigure(0,0,0,0,0,0,0)
#
# Load the database
cd "../../db" 
dbLoadDatabase("../dbd/tpmrt.dbd")
#
dbLoadRecords("util2.db")
# Serial I/O
dbLoadRecords("mig.db")
dbLoadRecords("dmig_dt.db")
dbLoadRecords("galil.db")
dbLoadRecords("galil_m1.db")
dbLoadRecords("galil_m2.db")
dbLoadRecords("scale.db")
dbLoadRecords("tmicro.db")
dbLoadRecords("tmicro_block.db")
dbLoadRecords("temp_means.db")
dbLoadRecords("temp_alarms.db")
dbLoadRecords("imager_ups.db")
# Terminal Server I/O
dbLoadRecords("humid.db")
# Data Analysis
dbLoadRecords("encl_humid.db")
#
#	Start EPICS IOC software
#
iocInit
#
#	And our state machines
#seq &performance_check
seq &mode_check
seq &galil_parse
#
TSreport
