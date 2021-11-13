
##############################################################
#
# AESDI2CTEMP
#
##############################################################

#TODO: Fill up the contents below in order to reference final project git contents
AESDI2CTEMP_VERSION = 'a6217e94a7f2686d12f121fb9f76f8cff67ee454'
# Note: Be sure to reference the *ssh* repository URL here (not https) to work properly
# with ssh keys and the automated build/test system.
# Your site should start with git@github.com:
AESDI2CTEMP_SITE = 'git@github.com:cu-ecen-aeld/final-project-VenkatTata.git'
AESDI2CTEMP_SITE_METHOD = git
AESDI2CTEMP_GIT_SUBMODULES = YES

define AESDI2CTEMP_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)/tempsensor all
endef

# TODO add your writer, finder and finder-test utilities/scripts to the installation steps below
define AESDI2CTEMP_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/tempsensor/* $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
