#
# TPM IOC #1 Startup Script
# Version: 14 August 2003 
# Peregrine M. McGehee/LANL
#
#
#       Set up our network environment.
#
sysClkRateSet(60)

cd "/p/tpmbase/iocBoot/ioctpm"
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
shellPromptSet("[tpm:~]$ ")
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
ld(0, 1, "../../bin/mv162/tpmcommonLib");

# 3 = -10..10V
ipacAddCarrier(&ipmv162, "C:l=0,0")
PMSS = IP16ADC_init(2,3,1)
_PMSS = PMSS

IP470_A = ip470DevCreate(0xfff58000)
IP470_B = ip470DevCreate(0xfff58100)
_IP470_B = IP470_B

ld < ../../bin/mv162/tpmrtLib
#
putenv("EPICS_TS_MIN_WEST=0")
# Get NTP time from utc-time.apo.nmsu.edu (galileo)
putenv("EPICS_TS_NTP_INET=192.41.211.40")
#TSdriverDebug=5
TSconfigure(0,0,0,0,0,0,0)
#
# Load the database
cd "../../db" 
dbLoadDatabase("../dbd/tpmrt.dbd")
#
dbLoadRecords("util.db")
dbLoadRecords("ilock.db")
dbLoadRecords("ab.db")
dbLoadRecords("absub.db")
dbLoadRecords("axis.db")
dbLoadRecords("scaled.db")
dbLoadRecords("tests_axis.db")
dbLoadRecords("ip470.db")
dbLoadRecords("ampstat.db")
dbLoadRecords("slip.db")
dbLoadRecords("pmss.db")
dbLoadRecords("waveAnl.db")
dbLoadRecords("plc_analog.db")
#
#	Start EPICS IOC software
#
# Fudge SHMEM CRCCHECK against MCP until RHL updates. 11 Aug 2003.
##CRCCHECK_FORCE = 1

iocInit
#
#	And our state machines
#seq &performance_check
#seq &galil_parse
#
TSreport
