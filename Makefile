CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -pthread -Iinclude -Ivendor/ntgcalls/include -I/usr/include
LDFLAGS = -Lvendor/ntgcalls/lib -lntgcalls -lyaml-cpp -lcurl -lpthread -ldl -lz -lresolv

SRCDIR = src
INCDIR = include
BINDIR = bin
TESTDIR = tests

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BINDIR)/%.o)
TARGET = $(BINDIR)/tina-music-bot
BRIDGE_TARGET = $(BINDIR)/assistant-bridge

TEST_TARGET = $(BINDIR)/test_full

all: $(TARGET) $(BRIDGE_TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@ln -sf tina-music-bot $(BINDIR)/yukki-music-bot
	@echo "Build complete: $(TARGET)"

$(BRIDGE_TARGET):
	@mkdir -p $(BINDIR)
	@echo "Building native voice bridge..."
	@cd bridge && go build -o ../$(BRIDGE_TARGET) .
	@echo "Build complete: $(BRIDGE_TARGET)"

$(BINDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	@echo "Running full test suite..."
	@./$(TEST_TARGET)

$(TEST_TARGET): $(TESTDIR)/test_full.cpp $(BINDIR)/queue_manager.o $(BINDIR)/logger.o $(BINDIR)/database.o $(BINDIR)/locales.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lyaml-cpp -lpthread

clean:
	rm -rf $(BINDIR)

.PHONY: all test clean $(BRIDGE_TARGET)
