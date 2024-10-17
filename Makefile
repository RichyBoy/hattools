PROJ 	 = hattools

# gcc doesn't have any options for controlling the gcm.cache location and it is dependant on both the 
# compiler location (prefix) and the compiler version. Or, for GNU Make to check for compiled code in 
# the gcm.cache we need to construct the path of where it lives..
#
# A plea to the gcc gang: please provide something like gcc -dump-gcm.cache 

GCCPATH	:= $(shell which gcc)
GCCVERS := $(shell gcc -dumpversion)
CACHE 	 = gcm.cache/$(subst /bin/gcc,,$(GCCPATH))/include/c++/$(GCCVERS)/
CXXFLAGS = -std=c++2b -fmodules-ts -fconcepts -Wall -O3 -g -fPIC

# C++23 modules for compilation
MODS := chrono \
	vector \
	concepts \
	execution \
	format \
	fstream \
	iostream \
	limits \
	memory \
	random \
	span \
	thread \
	ranges \
	ratio \
	algorithm 

# Project's own module sources, paritions first
SORTSSRC	= sorts/hattools-pidgeon.ixx \
		  sorts/hattools-pidgeon23.ixx \
		  sorts/hattools-pidgeonflock.ixx \
		  sorts/hattools.sorts.ixx		
		  #sorts/hattools-radical.ixx 
UTILSSRC	= utils/hattools-generator.ixx \
		  utils/hattools-timer.ixx \
		  utils/hattools.utils.ixx

# Project's regular source code
SRC	= utils/hattools-timer-impl.cpp 

# Executable source code(s)
EXESRC1	= tests/hatsorttests.cpp
		
DIRS = lib64 bin
TEMP1		:= $(addprefix $(CACHE),$(MODS))
CACHETARGETS	:= $(TEMP1:=.gcm)
SORTSOBJS	= $(SORTSSRC:%.ixx=%.o)
UTILSOBJS	= $(UTILSSRC:%.ixx=%.o)
OBJS		= $(SRC:%.cpp=%.o)
EXEOBJS1	= $(EXESRC1:%.cpp=%.o)
EXE1		= bin/hatsorttests 
LIB		= lib64/lib$(PROJ).so
print-%  : ; @echo $* = $($*) # Courtesy https://www.cmcrossroads.com/article/printing-value-makefile-variable

.PHONY : all 
all : $(shell mkdir -p $(DIRS)) $(CACHETARGETS) $(SORTOBJS) $(UTILSOBJS) $(OBJS) $(LIB) $(EXE1)

exe : $(EXE1)

.PHONY : clean
clean : 
	$(RM) $(SORTSOBJS) $(UTILSOBJS) $(OBJS) $(EXEOBJS1) $(LIB) $(EXE1)
	$(RM) -r gcm.cache $(DIRS)

.PHONY : cacheclean
cacheclean :
	$(RM) -r gcm.cache/usr/*

$(EXE1) : $(CACHETARGETS) $(LIB) $(EXEOBJS1)
	$(CXX) -pie -L$(PWD)/lib64 -Wl,-rpath=$(PWD)/lib64 $(EXEOBJS1) -l$(PROJ) -o $@

$(LIB) : $(SORTSOBJS) $(UTILSOBJS) $(OBJS)
	$(CXX) -shared $^ -o $@ 

%.gcm : 
	$(CXX) $(CXXFLAGS) -x c++-system-header $(subst .gcm,,$(subst $(CACHE),,$@))

%.o : %.ixx
	$(CXX) $(CXXFLAGS) -c -o $@ -x c++ $<

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
