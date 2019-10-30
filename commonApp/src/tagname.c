#include <vxWorks.h>
#include <stdio.h>
#include <assert.h>

/*
 * This function is provided so that the dollar-Name:-dollar only
 * needs to be expanded in one file
 */

const char *
getCvsTagname(void)
{
   return "$Name:  $";
}

char *
version_cmd(char *cmd)			/* NOTUSED */
{
   return "tpmVersion=\"$Name:  $|"  __DATE__ "|" __TIME__ "\"\n";
}



/*****************************************************************************/
/*
 * return the TPM version
 */
char *
tpmVersion(char *version,		/* string to fill out, or NULL */
	   int len)			/* dimen of string */
{
   static char buff[100 + 1];		/* buffer if version == NULL */
   int i;
   int print = (version == NULL) ? 1 : 0; /* should I print the version? */
   const char *ptr;			/* scratch pointer */
   const char *tag = version_cmd("");	/* CVS tagname + compilation time */

   if(version == NULL) {
      version = buff; len = 101;
   }

   version[len - 1] = '\a';		/* check for string overrun */

   ptr = strchr(tag, ':');
   if(ptr == NULL) {
      strncpy(version, tag, len - 1);
   } else {
      ptr++;
      while(isspace(*ptr)) ptr++;
      
      if(*ptr != '$') {			/* a CVS tag */
	 for(i = 0; ptr[i] != '\0' && !isspace(ptr[i]) && i < 100; i++) {
	    version[i] = ptr[i];
	 }
	 version[i] = '\0';
      } else {
	 if(*ptr == '$') ptr++;
	 if(*ptr == '|') ptr++;
	 
	 sprintf(version, "NOCVS:%s", ptr);
	 version[strlen(version) - 2] = '\0'; /* trim "double quote\n" */
      }
   }

   assert(version[len - 1] == '\a');	/* no overrun */
   if(print) {
      printf("tpmVersion: %s\n", version);
   }

   return(version);
}

