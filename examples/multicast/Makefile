CONTIKI_PROJECT = root intermediate sink
all: $(CONTIKI_PROJECT)

MAKE_MAC = MAKE_MAC_TSCH

AT86RF2XX_BOARD = ISMTV_v1.1
TARGET=vesna

VESNA= ../../arch/platform/vesna/vesna-drivers/

CONTIKI = ../..

include $(CONTIKI)/Makefile.identify-target
MODULES_REL += $(TARGET)

include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_NET_DIR)/ipv6/multicast

MAKE_ROUTING = MAKE_ROUTING_RPL_CLASSIC
include $(CONTIKI)/Makefile.include
