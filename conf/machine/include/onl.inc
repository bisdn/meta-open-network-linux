# onie vars
ONIE_VENDOR ?= "unknown"
# TODO setup a mapping?
ONIE_VENDOR_SHORT ?= "${ONIE_VENDOR}"
ONIE_MACHINE_TYPE ?= "unknown"
ONIE_ARCH ?= "${TARGET_ARCH}"
ONIE_MACHINE ?= "${ONIE_VENDOR_SHORT}_${ONIE_MACHINE_TYPE}"
ONIE_MACHINE_REV ?= "0"
ONIE_PLATFORM ?= "${ONIE_ARCH}-${ONIE_MACHINE}-r${ONIE_MACHINE_REV}"

# onl vars
ONL_PLATFORM="${@'${ONIE_ARCH}-${ONIE_MACHINE}'.replace('_', '-')}"

# list of platforms in ONIE format for which to include support
ONL_PLATFORM_SUPPORT ?= "${ONIE_PLATFORM}"

# some platforms require common vendor modules
ONL_MODULE_VENDORS ?= ""
