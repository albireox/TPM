/*
 * Local copy of shared memory - updated by 20Hz scan task.
*/
struct SDSS_FRAME TPM_SDSS_FRAME;

/*
 * List of vxWorks variables that are mapped to EPICS records
 * using symbolic device support.
*/

long
    tpm_CRCLOOPS,
    tpm_SHMLOOPS,
    tpm_MCPCOUNT;
    tpm_DALENC1;

char
    tpm_VERSION[MAX_STRING_SIZE];

/*
 * Raw Ilock status data
*/
unsigned short
    tpm_ILOCK[190];
/*
 * Fast channels
*/
long
    tpm_AZENC1F,
    tpm_ALENC1F,
    tpm_ROCENCF,
    tpm_AZMTRC1F,
    tpm_ALMTRC1F,
    tpm_ROMTRCF,
    tpm_AZERRF,
    tpm_ALERRF,
    tpm_ROERRF;
	
/*
 * Readbacks from MCP in various encoder and ADC counts.
*/
long
    tpm_ALCPOS,
    tpm_ALCVOLT,
    tpm_ALENC1, 
    tpm_ALENC2,
    tpm_ALMTRC1,
    tpm_ALMTRC2,
    tpm_ALMTRV1,
    tpm_ALMTRV2,
    tpm_ALWSPOS,
    tpm_ALERR,
    tpm_ALSTATE,
    tpm_AZCPOS,
    tpm_AZCVOLT,
    tpm_AZENC1,
    tpm_AZENC2,
    tpm_AZMTRC1,
    tpm_AZMTRC2,
    tpm_AZMTRV1,
    tpm_AZMTRV2,
    tpm_AZWSPOS,
    tpm_AZERR,
    tpm_AZSTATE,
    tpm_ROCENC,
    tpm_ROCPOS,
    tpm_ROCVOLT,
    tpm_ROMTRC,
    tpm_ROMTRV,
    tpm_ROERR,
    tpm_ROSTATE,
    tpm_AZCMD,
    tpm_ALCMD,
    tpm_IRCMD;

/*
 * Interlock bits - again from MCP.
*/
unsigned short
    tpm_ALBRKENCMD,
    tpm_ALBRKENSTAT,
    tpm_ALBRKENMCP,
    tpm_ALBRKDISCMD,
    tpm_ALBRKDISSTAT,
    tpm_ALBRKDISMCP,
    tpm_AZBRKENCMD,
    tpm_AZBRKENSTAT,
    tpm_AZBRKENMCP,
    tpm_AZBRKDISCMD,
    tpm_AZBRKDISSTAT,
    tpm_AZBRKDISMCP,
    tpm_M2FORCE;

/*
 * PVTs
*/
long
/* MCP versions */
    tpm_AZMCPPOS,
    tpm_AZMCPVEL,
    tpm_AZMCPTIM,
    tpm_ALMCPPOS,
    tpm_ALMCPVEL,
    tpm_ALMCPTIM,
    tpm_ROMCPPOS,
    tpm_ROMCPVEL,
    tpm_ROMCPTIM,
/* TCC versions */
    tpm_AZTCCPOS,
    tpm_AZTCCVEL,
    tpm_AZTCCTIM,
    tpm_ALTCCPOS,
    tpm_ALTCCVEL,
    tpm_ALTCCTIM,
    tpm_ROTCCPOS,
    tpm_ROTCCVEL,
    tpm_ROTCCTIM;
