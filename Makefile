#Variables
CXX = g++ 
CXXFLAGS = -std=c++17 -pthread -g

#Include and library paths for GTest
INCLUDES = $(GTEST_INCLUDE_DIR) 
LDFLAGS = -L$(GTEST_LIB_DIR) -lgtest -lgtest_main -pthread

SOURCES = channel_test.cpp 
OBJS = $(SOURCES:.cpp=.o)

#Executable
TARGET = channel_test

all : $(TARGET)

$(TARGET): $(OBJS)  
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o : %.cpp 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean: 
	rm -f $(OBJS) $(TARGET)

format:
	clang-format -i *.cpp *.hpp

#make GTEST_INCLUDE_DIR="-I/home/elerer/googletest/googletest/include -I/home/elerer/github_Stuff/channels/"  GTEST_LIB_DIR=~/googletest/build/lib

