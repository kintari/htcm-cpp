
CXX=clang++
CXXFLAGS=-std=c++20 -g -O0 -MMD -U_FORTIFY_SOURCE -Werror=return-type

BUILD_CONFIG=debug

CPPFILES=$(wildcard src/*.cpp)
OFILES=$(patsubst src/%.cpp,$(OBJDIR)/%.o,$(CPPFILES))
DFILES=${OFILES:.o=.d}

OBJDIR=build/obj/$(BUILD_CONFIG)
BINDIR=build/bin/$(BUILD_CONFIG)

TARGET=$(BINDIR)/htcm-cpp


.PHONY: compile clean

compile: $(TARGET)

$(TARGET): $(OFILES)
	$(CXX) -o $(TARGET) $(OFILES)

$(OFILES:)
$(OBJDIR)/%.o: src/%.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) $<

info:
	echo $(CPPFILES)
	echo $(OFILES)
	echo $(DFILES)

clean:
	-rm -f build/bin/$(BUILD_CONFIG)/*
	-rm -f build/obj/$(BUILD_CONFIG)/*

-include ${DFILES}
