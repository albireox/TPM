
static HLOGFILE lf = 0;
static SEM_ID semNotify = 0;
static unsigned short sampleRate = DEF_SAMPLE_RATE;
static unsigned long stats[statTotal] = { 0 };

/*
 *	Constant local data...
 */

static char const sep[] = " \t";

/* This variable holds the default path to where the log files will be
   held. There is code that expects this string to contain the
   trailing '/' */

static char const thePath[] = "/mcptpm/";

/* This array holds all the information pertaining to register
   handles. We make it 'const' for debugging purposes. */

/* This pointer gets us to the shared data structure. */

#define data ((struct SDSS_FRAME volatile*) 0x02800002)

static struct RegDef regList[MAX_REGISTERS];
static int N_REG = MAX_REGISTERS;
