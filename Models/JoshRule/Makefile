
FLEET_ROOT=../../../

include $(FLEET_ROOT)/Fleet.mk


all:
	g++ Main.cpp -o main -lm -O3 $(FLEET_FLAGS) $(FLEET_INCLUDE) $(FLEET_LIBS) 
clang:
	clang++ Main.cpp -o main -O2 $(CLANG_FLAGS) $(FLEET_INCLUDE) $(FLEET_LIBS) 
debug:
	g++ Main.cpp -o main -lm -g $(FLEET_FLAGS) $(FLEET_INCLUDE) $(FLEET_LIBS) 
cdebug:
	clang++ Main.cpp -g -o main -O2 $(CLANG_FLAGS) $(FLEET_INCLUDE) $(FLEET_LIBS) 
