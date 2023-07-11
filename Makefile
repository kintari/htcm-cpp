
CXX=clang++
CXXFLAGS=-std=c++20 -g -O0 -MMD -U_FORTIFY_SOURCE $(IFLAGS) -Werror=return-type
IFLAGS=-Iinclude -I$(ZLIB)

BUILD_CONFIG=debug

CPPFILES=$(wildcard src/*.cpp)
OFILES=$(patsubst src/%.cpp,$(OBJDIR)/%.o,$(CPPFILES))
DFILES=${OFILES:.o=.d}

ZLIB=lib/zlib-1.2.13
LIBS=$(ZLIB)/libz.a

OBJDIR=build/obj/$(BUILD_CONFIG)
BINDIR=build/bin/$(BUILD_CONFIG)

TARGET=$(BINDIR)/htcm-cpp

.PHONY: compile clean

compile: $(BINDIR) $(OBJDIR) $(TARGET)

$(BINDIR):
	mkdir -p $@

$(OBJDIR):
	mkdir -p $@

$(TARGET): $(LIBS) $(OFILES)
	$(CXX) -o $(TARGET) $(OFILES) $(LIBS)

$(OBJDIR)/%.o: src/%.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) $<

lib/zlib-1.2.13/libz.a:
	cd `dirname $@` && ./configure --64 --static && make

info:
	echo $(CPPFILES)
	echo $(OFILES)
	echo $(DFILES)

clean:
	-rm -f build/*

-include ${DFILES}
