
##############################################################
#
# BTTEST
#
##############################################################

#TODO: Fill up the contents below in order to reference final project git contents
BTTEST_VERSION = '987f4ddc26821cc3da089b3d1cc4d713149f3ab1'
# Note: Be sure to reference the *ssh* repository URL here (not https) to work properly
# with ssh keys and the automated build/test system.
# Your site should start with git@github.com:
BTTEST_SITE = 'git@github.com:cu-ecen-aeld/final-project-VenkatTata.git'
BTTEST_SITE_METHOD = git
BTTEST_GIT_SUBMODULES = YES

define BTTEST_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)/bttest all
endef

# TODO add your writer, finder and finder-test utilities/scripts to the installation steps below
define BTTEST_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/bttest/* $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
