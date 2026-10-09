CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -pthread -Iinclude -I/usr/include
LDFLAGS = -lcurl -lpthread

SRCDIR = src
INCDIR = include
BINDIR = bin
TESTDIR = tests

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BINDIR)/%.o)
TARGET = $(BINDIR)/yukki-music-bot

TEST_TARGET = $(BINDIR)/test_queue

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

$(BINDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	@echo "Running tests..."
	@./$(TEST_TARGET)

$(TEST_TARGET): $(TESTDIR)/test_queue.cpp $(BINDIR)/queue_manager.o $(BINDIR)/logger.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lpthread

clean:
	rm -rf $(BINDIR)

.PHONY: all test clean
