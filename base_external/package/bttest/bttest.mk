
##############################################################
#
# BTTEST
#
##############################################################

#TODO: Fill up the contents below in order to reference final project git contents
BTTEST_VERSION = 'a6da18c13bc152b6d70f002cbf06dbabc7abe134'
# Note: Be sure to reference the *ssh* repository URL here (not https) to work properly
# with ssh keys and the automated build/test system.
# Your site should start with git@github.com:
BTTEST_SITE = 'git@github.com:cu-ecen-aeld/final-project-VenkatTata.git'
BTTEST_SITE_METHOD = git
BTTEST_GIT_SUBMODULES = YES

define BTTEST_BUILD_CMDS
	 $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)/bluez5_utils_5.55 all
endef

# TODO add your writer, finder and finder-test utilities/scripts to the installation steps below
define BTTEST_INSTALL_TARGET_CMDS
	
$(INSTALL) -m 0755 $(@D)/bluez5_utils_5.55/tools/btgatt-server $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
