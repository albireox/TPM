#Makefile at top of application tree
SHELL = /bin/sh
TOP = .
include $(TOP)/config/CONFIG_APP
DIRS += commonApp logApp rtApp slaApp
include $(TOP)/config/RULES_TOP

all :: inc buildInstall

tags :
	@if [ X"$(EMACS_DIR)" = X"" ]; then \
		echo "Not creating tag file (emacs isn't set up)" >&2 ; \
		exit 1; \
	fi

	@rm -f TAGS
	@for d in $(DIRS); do \
		echo $$d; \
		etags --append -a -t -o TAGS $$d/src/*.[ch]; \
	done

install ::
	@echo ""
	@echo "Make sure the current TPM directories under"
	@echo ""
	@echo "     `pwd`"
	@echo ""
	@echo "have the latest versions of the files.  These will be copied to"
	@echo "$(TPM_DIR) during the TPM's installation."
	@echo ""
	@if [ "$(TPM_DIR)" = "" ]; then \
		echo "The destination directory has not been specified." >&2; \
		echo "Set the environment variable TPM_DIR" >&2; \
		echo ""; \
		exit 1; \
	fi
	@if [ ! -d $(TPM_DIR) ]; then \
		echo $(TPM_DIR) "doesn't exist; making it"; \
		mkdir $(TPM_DIR); \
	fi
	@:
	@: Check the inode number for . and $(TPM_DIR) to find out if two
	@: directories are the same\; they may have different names due to
	@: symbolic links and automounters
	@:
	@if [ `ls -id $(TPM_DIR) | awk '{print $$1}'` = \
				`ls -id . | awk '{print $$1}'` ]; then \
		echo "The destination directory is the same" \
			"as the current directory; aborting." >&2; \
		echo ""; \
		exit 1; \
	fi
	@echo "I'll give you 5 seconds to think about it (control-C to abort) ..."
	@for pos in          5 4 3 2 1; do \
	   echo "                              " | sed -e 's/ /'$$pos'/'$$pos; \
	   sleep 1; \
	done
	@echo "... and we're off... deleting"
	-@/bin/rm -rf $(TPM_DIR)
	@mkdir $(TPM_DIR)
	@echo "installing ups..."
	@if [ X`echo $(HOSTNAME) | grep sdsshost` = "Xsdsshost" ]; then \
	     echo "	on sdsshost"; \
	     cp -R ups_host $(TPM_DIR)/ups; \
	else \
	     echo "	on sdsscommish"; \
	     cp -R ups_commish $(TPM_DIR)/ups; \
	fi
	@if [ -d bin ]; then \
	    echo "installing binaries..."; \
	    cp -R bin $(TPM_DIR); \
        fi
	@if [ -d dbd ]; then \
	    echo "installing data definition files..."; \
	    cp -R  dbd $(TPM_DIR); \
	fi
	@if [ -d db ]; then \
	    echo "installing real-time database..."; \
	    cp -R db $(TPM_DIR); \
        fi
	@if [ -d rtApp/opi ]; then \
	    echo "installing operator displays..."; \
	    mkdir $(TPM_DIR)/opi; \
	    cp -R rtApp/opi/* $(TPM_DIR)/opi; \
        fi
	@if [ -d iocBoot ]; then \
	    echo "installing IOC boot files..."; \
	    cp -R iocBoot $(TPM_DIR); \
	fi
	@echo "installing ChanArch config files..."; 
	@if [ X`echo $(HOSTNAME) | grep sdsshost` = "Xsdsshost" ]; then \
	    echo "	on sdsshost"; \
	    cp -R ChanArch_host $(TPM_DIR)/ChanArch; \
	else \
	    echo "	on sdsscommish"; \
	    cp -R ChanArch_commish $(TPM_DIR)/ChanArch; \
	fi
	@cp `find $(EPICS)/extensions/bin/ -name CGIExport` $(TPM_DIR)/ChanArch/tpmwww/Tests/cgi/CGIExport.cgi
	@chmod -R 777 $(TPM_DIR)
	@ln -s /mcptpm/ChanArchTmp $(TPM_DIR)/ChanArch/tpmwww/Tests/cgi/tmp
	@echo "done with TPM install."
