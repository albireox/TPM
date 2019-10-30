#include <vxWorks.h>
#include <taskLib.h>
#include <stdio.h>
#include <cadef.h>

/*
 * smp_ca is the underlying routine by which EPICS process variables
 * are accessed. The short timeout of 0.1 sec on the pend_io
 * for the read assumes that a local PV is called for and than CA is
 * only the calling interface into DB access.
*/
float smp_ca(char * pvname, chid * chan) 
{
    int status;   
    float fval;	
    
    if (!*chan) {
	status = ca_search(pvname, chan);	
	status = ca_pend_io(1.0);
	if (status != ECA_NORMAL) {
	    printf("%s: not found!\n", pvname);
	    return 0;
	} 
    }
    
    status = ca_rget(*chan, &fval);	
    if (status != ECA_NORMAL) {
	printf("%s:%x: error in ca_rget call\n", pvname, *chan);
	return 0;
    }
    status = ca_pend_io(0.1); 
    if (status != ECA_NORMAL) {
	printf("%s:%x error in ca_pend_io call\n", pvname, *chan);
	return 0;
    }
    return fval;
}
