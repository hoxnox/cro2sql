#
# compile release: ./make
# compile debug: ./make --debug
#

first: all

#### Defige release/debug dependent vars
ifdef debug
OBJECTS_DIR   := Debug
CXXFLAGS      = -Wall -ggdb -g3 -DDEBUG
else
OBJECTS_DIR   := Release
CXXFLAGS      = -Wall -O2
endif

#### SOURCES
SRC_DIR     =  ./src
OBJ_DIR     := $(OBJECTS_DIR)
SRC         :=  dataparser.cpp \
		dbstruct.cpp \
		freader.cpp \
		getopt.cpp \
		misc.cpp
HDR         =  dataparser.h \
	       dbstruct.h \
	       freader.h \
	       misc.h \
	       getopt.hpp \
	       messager.hpp
### Define compile variables
MAKEFILE        = Makefile
CC              = gcc
CXX             = g++
INCPATH         = -I"." \
		  -I"./src"
LINK            = g++
LFLAGS          = 
LIBS            = 
DEL_FILE        = rm -f
DEL_DIR         = rm -rf
MKDIR           = mkdir -p
CXXOUTPUT       = -o

### SOURCES TRANSLATION
HDR  = $(addprefix $(SRC_DIR)/,$(HDR))
OBJ := $(addprefix $(OBJ_DIR)/,$(addsuffix .obj,$(basename $(SRC))))
SRC  = $(addprefix $(SRC_DIR)/,$(SRC))

### Build directory creation/deletion
mk_dir:
	-@$(MKDIR) $(OBJECTS_DIR)
clean:
	-@$(DEL_DIR) $(OBJECTS_DIR)

#### Implicit rules
$(OBJ_DIR)/%.obj: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCPATH) $(CXXOUTPUT)$@ -c $<

all: mk_dir cro2sql.exe test
without_test: mk_dir cro2sql.exe

$(OBJECTS_DIR)/main.obj : $(SRC_DIR)/main.cpp
	$(CXX) $(CXXFLAGS) $(INCPATH) $(CXXOUTPUT)$@ -c $<
cro2sql.exe : $(OBJ) $(OBJECTS_DIR)/main.obj
	$(LINK) $(LFLAGS) -o $(OBJECTS_DIR)/cro2sql.exe $(OBJ) $(OBJECTS_DIR)/main.obj $(LIBS)

test: mk_dir test.exe
# test
$(OBJECTS_DIR)/test.obj : test/test.cpp
	$(CXX) $(CXXFLAGS) $(INCPATH) $(CXXOUTPUT)$@ -c $<
test.exe: $(OBJ) $(OBJECTS_DIR)/test.obj
	$(LINK) $(LFLAGS) -o $(OBJECTS_DIR)/test.exe $(OBJ) $(OBJECTS_DIR)/test.obj $(LIBS)

.PHONY: all first test without_test

