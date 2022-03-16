###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_cel_questone_2a_INCLUDES := -I $(THIS_DIR)inc
x86_64_cel_questone_2a_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_cel_questone_2a_DEPENDMODULE_ENTRIES := init:x86_64_cel_questone_2a ucli:x86_64_cel_questone_2a

