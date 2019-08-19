# build param
CC = g++
CFLAGS = -std=c++11 -pthread -Wall -g

# directory
BINDIR = .
OBJDIR = $(BINDIR)/obj
TARGET = $(BINDIR)/HTTPServer

INCLUDE = include
SRCDIR = src

# build directory
ALL_INCLUDE += $(patsubst %, -I%, $(INCLUDE))
ALL_SRC += $(wildcard $(SRCDIR)/*.cpp)
ALL_OBJ += $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(ALL_SRC))

# make obj directory
#$(shell mkdir -p $(OBJDIR))

# build
$(TARGET) : $(ALL_OBJ)
	$(CC) $(CFLAGS) $(ALL_INCLUDE) $^ -o $@

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(ALL_INCLUDE) -c $< -o $@

# clean
.PHONY:clean
clean:
	-rm $(BINDIR)/* $(BINDIR) -rf
