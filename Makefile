PROJ 	 = hattools
# The variable below is the prefix path of gcc that can be found with gcc -v 
# If this isn't set then Make won't know which modules have been built and a lack of gcc imperative for setting the
# cache is a serious blunder IMO as modules are supposed to ease compilation...
GCCP	 = /home/richard/gcc
CACHE 	 = gcm.cache/$(GCCP)/include/c++/15.0.0/
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
	algorithm \

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
		
TEMP1		:= $(addprefix $(CACHE),$(MODS))
CACHETARGETS	:= $(TEMP1:=.gcm)
SORTSOBJS	= $(SORTSSRC:%.ixx=%.o)
UTILSOBJS	= $(UTILSSRC:%.ixx=%.o)
OBJS		= $(SRC:%.cpp=%.o)
EXEOBJS1	= $(EXESRC1:%.cpp=%.o)
EXE1		= hatsorttests 
LIB		= lib$(PROJ).so
print-%  : ; @echo $* = $($*) # Courtesy https://www.cmcrossroads.com/article/printing-value-makefile-variable

.PHONY : all 
all : $(CACHETARGETS) $(SORTOBJS) $(UTILSOBJS) $(OBJS) $(LIB) $(EXE1)

exe : $(EXE1)

.PHONY : clean
clean : 
	$(RM) $(SORTSOBJS) $(UTILSOBJS) $(OBJS) $(EXEOBJS1) $(LIB) $(EXE1)
	$(RM) -r gcm.cache

.PHONY : cacheclean
cacheclean :
	$(RM) -r gcm.cache/usr/*

$(EXE1) : $(CACHETARGETS) $(LIB) $(EXEOBJS1)
	$(CXX) -L$(PWD) -Wl,-rpath=$(PWD) $(EXEOBJS1) -l$(PROJ) -o $@

$(LIB) : $(SORTSOBJS) $(UTILSOBJS) $(OBJS)
	$(CXX) -shared $^ -o $@ 

%.gcm : 
	$(CXX) $(CXXFLAGS) -x c++-system-header $(subst .gcm,,$(subst $(CACHE),,$@))

%.o : %.ixx
	$(CXX) $(CXXFLAGS) -c -o $@ -x c++ $<

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
