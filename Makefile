﻿AR       := ar
CC       := gcc
CXX      := g++
RANLIB   := ranlib
ARFLAGS  := rc
CFLAGS   := -g -O0 -ffunction-sections -fdata-sections -fno-strict-aliasing
CPPFLAGS := -Wall -Werror -ansi -MMD -I$(CURDIR)/include
LDFLAGS  := -Wl,--as-needed -Wl,-gc-section -L$(CURDIR)
LIBS     :=

#JSON_C_PATH := D:/Software/GNU/json-c

ifeq ($(OS),Windows_NT)
CPPFLAGS += -I$(JSON_C_PATH)/include
LDFLAGS  += -L$(JSON_C_PATH)/lib
endif

ifeq ($(OS),Windows_NT)
OUT      := run.exe
else
OUT      := run
endif

USER_SRC := demo.cc
USER_OBJ := $(patsubst %.cc,%.o,$(USER_SRC))
USER_DEP := $(patsubst %.cc,%.d,$(USER_SRC))

define compile_c
@echo CC	$1
@$(CC) $(CPPFLAGS) -std=c99 $(CFLAGS) -c -o $1 $2
endef

define compile_cc
@echo CXX	$1
@$(CC) $(CPPFLAGS) -std=c++11 $(CFLAGS) -c -o $1 $2
endef

define ar_lib
@echo AR	$1
@$(AR) $(ARFLAGS) $1 $2
@$(RANLIB) $1
endef

define link_objects
@echo LD	$1
@$(CXX) $(CFLAGS) $(LDFLAGS) -o $1 $2 $(LIBS)
endef

.PHONY: all
all: $(OUT)
$(OUT): $(USER_OBJ)
	$(call link_objects,$@,$^)
$(USER_OBJ): %.o: %.cc
	$(call compile_cc,$@,$<)

.PHONY: clean
clean:
ifeq ($(OS), Windows_NT)
	@for %%I in ($(subst /,\,$(USER_DEP) $(USER_OBJ)) $(OUT)) do if exist %%I del /f /q %%I
else
	@$(RM) $(USER_DEP)
	@$(RM) $(USER_OBJ)
	@$(RM) $(OUT)
endif

.PHONY: debug
debug: $(OUT)
	@gdb $(OUT)