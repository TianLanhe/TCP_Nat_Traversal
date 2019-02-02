TARGET := client server natcheckerclient testserver master slave testslave testclient

# 设置 shell，否则 echo -e 会出问题
SHELL=/bin/bash

SOURCE_PATH := .
# dep 目录不能加"./"，因为当 "./abc" 成为目标时， $@ 会变成 "abc"，在 patsubst 时会出问题
DEP_PATH := dep
OBJ_PATH := obj
BIN_PATH := bin
# main 函数所在目录，会自动在前面加 "./" ，find 命令要求的
MAIN_PATH := main

DBG_ENABLE := 0
LIBS := pthread
LIBRARY_PATH := /lib
INCLUDE_PATH := . include
OTHER_FLAGS := -std=c++11

SOURCE_FILES := $(shell find . -path ./$(MAIN_PATH) -prune -o -name *.cpp -print)
DEP_FILES := $(patsubst $(SOURCE_PATH)/%.cpp,$(DEP_PATH)/%.d,$(SOURCE_FILES))
OBJ_FILES := $(patsubst $(SOURCE_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(SOURCE_FILES))
TARGET_FILES := $(addprefix $(BIN_PATH)/, $(TARGET))

ifeq (1, $(DBG_ENABLE))
	CXXFLAGS += -O0 -g
else
	CXXFLAGS += -O3
endif

CXXFLAGS  += $(foreach dir, $(INCLUDE_PATH), -I$(dir))
CXXFLAGS += $(OTHER_FLAGS)
LDFLAGS += $(foreach lib, $(LIBRARY_PATH), -L$(lib))
LDFLAGS += $(foreach lib, $(LIBS), -l$(lib))

ifneq ($(findstring _$(MAKECMDGOALS)_, _all_ __), )
DEP_FILES += $(foreach target, $(TARGET), $(DEP_PATH)/$(MAIN_PATH)/$(target).d)
OBJ_FILES += $(foreach target, $(TARGET), $(OBJ_PATH)/$(MAIN_PATH)/$(target).o)
else ifneq ($(findstring $(MAKECMDGOALS),$(TARGET)), )
DEP_FILES += $(foreach target, $(findstring $(MAKECMDGOALS),$(TARGET)), $(DEP_PATH)/$(MAIN_PATH)/$(target).d)
OBJ_FILES += $(foreach target, $(findstring $(MAKECMDGOALS),$(TARGET)), $(OBJ_PATH)/$(MAIN_PATH)/$(target).o)
endif

.PHONY : all $(TARGET)

all : $(TARGET)

$(TARGET) : % : $(BIN_PATH)/%

$(TARGET_FILES) : $(BIN_PATH)/% : $(filter-out $(OBJ_PATH)/$(MAIN_PATH)/%.o, $(OBJ_FILES)) $(OBJ_PATH)/$(MAIN_PATH)/%.o
	@mkdir -p $(BIN_PATH)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(DEP_PATH)/%.d : $(SOURCE_PATH)/%.cpp
	@echo Generating $@... ;	\
	mkdir -p $(dir $@);		\
	set -e;	\
	rm -f $@; \
	$(CXX) $(CXXFLAGS) -E -MM  $< > $@.temp; \
	sed 's,.*:,$@ :,g' < $@.temp > $@;	\
	sed 's,.*:,$(patsubst $(DEP_PATH)/%.d,$(OBJ_PATH)/%.o,$@) :,g' < $@.temp >> $@;	\
	echo -e "\t@mkdir -p $(patsubst $(DEP_PATH)/%,$(OBJ_PATH)/%,$(dir $@))" >> $@ ;	\
	echo -e "\t$(CXX) $(CXXFLAGS) -c -o $(patsubst $(DEP_PATH)/%.d,$(OBJ_PATH)/%.o,$@) $< " >> $@ ; \
	rm -f $@.temp

ifneq ($(findstring _$(MAKECMDGOALS)_, _all_ __), )
sinclude $(DEP_FILES)
else ifneq ($(findstring $(MAKECMDGOALS),$(TARGET)), )
sinclude $(DEP_FILES)
endif

.PHONY : mkdir clean cleanall

mkdir: exist_dir = $(wildcard $(dir $(DEP_FILES)) $(dir $(OBJ_FILES)) $(BIN_PATH))
	not_exist_dir = $(filter-out $(exist_dir),$(dir $(DEP_FILES)) $(dir $(OBJ_FILES)) $(BIN_PATH))
mkdir:
	@echo "Not Exist Directories: " $(not_exist_dir);	\
	if [ -n "$(not_exist_dir)" ]; then \
		mkdir -p $(not_exist_dir) ;	\
	fi

cleanall: clean
	-rm -rf $(BIN_PATH)

clean:
	-rm -rf $(DEP_PATH) $(OBJ_PATH)
