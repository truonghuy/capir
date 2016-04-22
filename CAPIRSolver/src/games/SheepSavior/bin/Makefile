
# name of dependency file
DEPFILE = Makefile.dep

# solver, WorldModels and problem directories
UTILS = ../../../utils/
WORLDMODELS = ../../../WorldModels/
GAMESRC = ../src/

# include directories
INCDIR = -I$(UTILS) -I$(GAMESRC) -I$(WORLDMODELS) 
ZLIB = -lz

# Change this line if you want a different compiler
#CXX = g++ -ggdb -Wall -W $(INCDIR) 
#CXX = g++ -ggdb $(INCDIR)   
CXX = g++ -O2 $(INCDIR) 

# files
TARGETS = CAPIRSolver	

UTILSOBJ = $(UTILSSRCS:$(UTILS)%.cc=%.o)
WORLDMODELSOBJ = $(WORLDMODELSSRCS:$(WORLDMODELS)%.cc=%.o)
GAMESRCOBJ = $(GAMESRCSRCS:$(GAMESRC)%.cc=%.o)

HDRS = $(UTILSHDR) $(WORLDMODELSHDR) $(GAMESRCHDR) 
SRCS = $(UTILSSRCS) $(WORLDMODELSSRCS) $(GAMESRCSRCS)  

UTILSHDR = 	$(UTILS)Distribution.h \
    $(UTILS)Compression.h \
    $(UTILS)Utilities.h \
	$(UTILS)Model.h \
	$(UTILS)RandSource.h \
	$(UTILS)Simulator.h \
	$(UTILS)ValueIteration.h \
	$(UTILS)PathFinder.h  \
    $(UTILS)GameRunner.h 

UTILSSRCS =	$(UTILS)Distribution.cc \
    $(UTILS)Compression.cc \
    $(UTILS)Utilities.cc \
    $(UTILS)Simulator.cc \
	$(UTILS)ValueIteration.cc \
	$(UTILS)PathFinder.cc  \
    $(UTILS)GameRunner.cc

GAMESRCHDR =	$(GAMESRC)GB_Sheep.h \
    $(GAMESRC)GB_Ghost.h \
    $(GAMESRC)GB_GhostMaze.h \
    $(GAMESRC)GB_SheepMaze.h \
    $(GAMESRC)GB_Fiery.h \
    $(GAMESRC)GB_FieryMaze.h \
    $(GAMESRC)GB_Human.h \
    $(GAMESRC)GB_AiAssistant.h \
    $(GAMESRC)GhostBustersLevel.h

GAMESRCSRCS =	$(GAMESRC)GB_Ghost.cc \
    $(GAMESRC)GB_GhostMaze.cc \
    $(GAMESRC)GB_Sheep.cc \
    $(GAMESRC)GB_SheepMaze.cc \
    $(GAMESRC)GB_Fiery.cc \
    $(GAMESRC)GB_FieryMaze.cc \
    $(GAMESRC)GB_Human.cc \
    $(GAMESRC)GB_AiAssistant.cc \
    $(GAMESRC)GhostBustersLevel.cc

WORLDMODELSHDR =	$(WORLDMODELS)pugixml.hpp \
	$(WORLDMODELS)ObjectWithProperties.h \
    $(WORLDMODELS)SpecialLocation.h \
    $(WORLDMODELS)Agent.h \
    $(WORLDMODELS)Player.h \
    $(WORLDMODELS)Monster.h \
    $(WORLDMODELS)MazeWorldDescription.h \
    $(WORLDMODELS)Maze.h \
    $(WORLDMODELS)MazeWorld.h

WORLDMODELSSRCS =	$(WORLDMODELS)pugixml.cpp \
	$(WORLDMODELS)ObjectWithProperties.cc \
	$(WORLDMODELS)SpecialLocation.cc \
	$(WORLDMODELS)Agent.cc \
    $(WORLDMODELS)Player.cc \
    $(WORLDMODELS)Monster.cc \
    $(WORLDMODELS)Maze.cc \
    $(WORLDMODELS)MazeWorld.cc
    
# targets
all: $(TARGETS)
	 
clean:
	rm -f *~ *.o *.obj $(TARGETS) 

CAPIRSolver: $(GAMESRC)CAPIRSolver.cc $(UTILSOBJ) $(WORLDMODELSOBJ) $(GAMESRCOBJ) 
	$(CXX) -o $@ $< $(UTILSOBJ) $(WORLDMODELSOBJ) $(GAMESRCOBJ) $(ZLIB)

depend:	
	g++ -MM $(INCDIR) $(SRCS) > $(DEPFILE)

include $(DEPFILE)

%.o:
	$(CXX) -o $@ -c $<


# DO NOT DELETE
