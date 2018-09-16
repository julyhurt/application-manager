
TARGET = appsvc
CXX = g++
CXXFLAGS = -std=c++11 -ggdb3 -Wall
OEXT = o

# https://blog.csdn.net/humadivinity/article/details/78890754
INCLUDES =
DEP_LIBS = -lpthread -lcrypto -lssl -lACE -lglog -lcpprest -Wl,-Bstatic -ljsoncpp -lboost_thread -lboost_system -Wl,-Bdynamic


all : format $(TARGET) 

## source and object files 
SRCS = \
	main.cpp \
	Application.cpp \
    ApplicationShortRun.cpp \
    ApplicationPeriodRun.cpp \
	Configuration.cpp \
	RestHandler.cpp \
	Utility.cpp \
	Timer.cpp \
	TimerActionKill.cpp \
	DailyLimitation.cpp \
	TimerAction.cpp
		

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
	#dos2unix *.cpp *.h *.json

.PHONY: clean
clean:
	rm -f *.$(OEXT) $(TARGET) 
