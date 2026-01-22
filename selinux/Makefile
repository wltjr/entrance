# SELinux Policy Makefile for Entrance

POLICY_MODULE = entrance
VERSION = 1.0.0

# Compiler settings
CHECKPOLICY = checkpolicy
CHECKMODULE = checkmodule
SEMODULE_PACKAGE = semodule_package
SEMODULE = semodule
M4 = m4

# Determine SELinux type (strict/targeted/mcs/mls)
SELINUX_TYPE ?= $(shell selinuxenabled && sestatus | grep -oP 'Loaded policy name:\s+\K\w+' 2>/dev/null || echo strict)
SELINUX_MAKEFILE = /usr/share/selinux/$(SELINUX_TYPE)/include/Makefile

# Check if we have refpolicy development environment
ifeq ($(wildcard $(SELINUX_MAKEFILE)),)
    # Fallback to basic build
    USE_REFPOLICY = no
    SELINUX_DEVEL = /usr/share/selinux/devel
else
    USE_REFPOLICY = yes
endif

# Output files
MOD = $(POLICY_MODULE).mod
PP = $(POLICY_MODULE).pp
FC = $(POLICY_MODULE).fc
TE = $(POLICY_MODULE).te

.PHONY: all load unload relabel clean help

all: $(PP)

# Build using refpolicy if available, otherwise basic build
ifeq ($(USE_REFPOLICY),yes)
$(PP): $(TE) $(FC)
	@echo "Building with refpolicy macros ($(SELINUX_TYPE))..."
	$(MAKE) -f $(SELINUX_MAKEFILE) $@
else
# Fallback: basic build without refpolicy
$(MOD): $(TE)
	@echo "Compiling policy module (basic mode)..."
	$(CHECKMODULE) -M -m -o $@ $<

$(PP): $(MOD) $(FC)
	@echo "Packaging policy module..."
	$(SEMODULE_PACKAGE) -o $@ -m $(MOD) -f $(FC)
endif

# Load policy module
load: $(PP)
	@echo "Loading entrance SELinux policy..."
	$(SEMODULE) -i $(PP)
	@echo "Policy loaded successfully"

# Unload policy module
unload:
	@echo "Removing entrance SELinux policy..."
	$(SEMODULE) -r $(POLICY_MODULE) 2>/dev/null || true
	@echo "Policy removed"

# Relabel entrance files
relabel:
	@echo "Relabeling entrance files..."
	restorecon -Rv /usr/sbin/entrance
	restorecon -Rv /usr/lib64/entrance/ 2>/dev/null || true
	restorecon -Rv /etc/entrance/
	restorecon -Rv /var/cache/entrance/ 2>/dev/null || true
	restorecon -v /var/log/entrance.log 2>/dev/null || true
	restorecon -Rv /var/run/entrance* 2>/dev/null || true
	restorecon -Rv /run/entrance* 2>/dev/null || true
	@echo "Relabeling complete"

# Full installation
install: load relabel
	@echo "Entrance SELinux policy installed and files relabeled"

# Clean build artifacts
clean:
	rm -f $(MOD) $(PP) tmp/
	@echo "Build artifacts cleaned"

help:
	@echo "Entrance SELinux Policy Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build policy module (default)"
	@echo "  load     - Build and load policy into kernel"
	@echo "  unload   - Remove policy from kernel"
	@echo "  relabel  - Relabel entrance files with correct contexts"
	@echo "  install  - Load policy and relabel files (full setup)"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  sudo make install    # Install policy and relabel files"
	@echo "  sudo make relabel    # Just relabel files"
	@echo "  sudo make unload     # Remove policy"
