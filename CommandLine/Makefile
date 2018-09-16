
TARGET = appc
CXX = g++
CXXFLAGS = -std=c++11 -ggdb3 -Wall -Wl,-Bdynamic
OEXT = o

INCLUDES =
DEP_LIBS = -ldl -Wl,-Bstatic -lcpprest -lboost_system -lboost_program_options -lglog -lpthread -lssl -lcrypto -Wl,-Bdynamic

all : format $(TARGET) 

## source and object files 
SRCS = main.cpp \
	../ApplicationManager/Utility.cpp

OBJS = $(SRCS:.cpp=.$(OEXT))

# =============
# File suffixes
# =============
.SUFFIXES: .cpp .$(OEXT)

# ====================
#compile all cpp files
# ===================
%.${OEXT}: %.cpp
	${CXX} ${CXXFLAGS} ${INCLUDES} -c $<;

# Executable binary
$(TARGET): $(OBJS) 
	$(CXX) ${CXXFLAGS} -o $@ $^ $(DEP_LIBS)

format:
	#dos2unix *.cpp

.PHONY: clean
clean:
	rm -f *.$(OEXT) $(TARGET) 
