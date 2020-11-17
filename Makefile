SRC_DIR = .
BIN_DIR = ./bin
INCLUDE_DIR = ./include/
LIB_DIR = ./lib/
OGDF_INCLUDE_DIR = $(INCLUDE_DIR)/home/ubuntu/ogdf/include
OGDF_LIB_DIR = $(LIB_DIR)/home/ubuntu/ogdf/ogdf
GUROBI_LIB_DIR = /home/ubuntu/gurobi910/linux64/lib #Typical installation directory for Gurobi
CC = g++ -g -std=c++0x
LIBS = ../include/home/ubuntu/ogdf -lOGDF -lCOIN
INCLUDE = -I$(INCLUDE_DIR) -I$(OGDF_INCLUDE_DIR)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,bin/%.o,$(wildcard $(SRC_DIR)/*.cpp))
includes = $(wildcard $(INCLUDE_DIR)*.h)
all : mcp

$(BIN_DIR)/%.o : $(SRC_DIR)/%.cpp ${includes}
	$(CC) $(INCLUDE) -c -o $@ $<

mcp : $(OBJECTS) 
	$(CC) $(OBJECTS) -g -o $@ -L$(OGDF_LIB_DIR) -lOGDF -lCOIN -L$(GUROBI_LIB_DIR) -lgurobi_c++ -lgurobi81

clean :
	rm -rf $(BIN_DIR)/*.o  mcp *.svg *.pdf *.png *.dot GeneratedGraphs.tex 

latex :
	./drawLatex.sh

.PHONY: clean

