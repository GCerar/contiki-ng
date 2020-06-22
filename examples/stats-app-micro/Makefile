CONTIKI_PROJECT = stats-app
all: $(CONTIKI_PROJECT)

MAKE_MAC = MAKE_MAC_TSCH

AT86RF2XX_BOARD = ISMTV_v1.1
TARGET=vesna

#PROJECT_SOURCES= vsndriversconf.c stats-app.c  # TODO: Here for testbed devices...
#PROJECT_HEADERS= vsndriversconf.h				# I think we don't need them anymore

VESNA= ../../arch/platform/vesna/vesna-drivers/

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
