LIBNAME:=rpiplc
LIBRARY:=lib$(LIBNAME).so

override CXXFLAGS+=-Wall -Werror -std=c++14
override LDFLAGS+=-shared

CC=g++
CXX=g++
PREFIX=/usr/local

HEADERS:=$(wildcard src/*.h)
SRCS:=$(wildcard src/*.cpp)
OBJS:=$(patsubst %.cpp,%.o,$(SRCS))

TEST_SRCS=$(wildcard test/*.cpp)
TEST_BINS=$(patsubst %.cpp,%,$(TEST_SRCS))

define test-targets
.PHONY: $(1)
$(1): $(1).cpp $(LIBRARY)
	$(CXX) $(CXXFLAGS) -Isrc -L. -o $(1) $(1).cpp -lrpiplc
endef

.PHONY: first all world clean tests install
first all world: $(LIBRARY)

$(LIBRARY): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJS): CXXFLAGS+=-fpic
$(OBJS): $(SRCS) $(HEADERS)

$(foreach TEST_BIN,$(TEST_BINS),$(eval $(call test-targets,$(TEST_BIN))))

tests: $(TEST_BINS)

install:
ifneq ($(DESTDIR),)
	mkdir -p $(DESTDIR)
endif
	mkdir -p $(DESTDIR)$(PREFIX)/lib/
	cp $(LIBRARY) $(DESTDIR)$(PREFIX)/lib/
ifeq ($(DESTDIR),)
	ldconfig
endif
	mkdir -p $(DESTDIR)$(PREFIX)/include/$(LIBNAME)
	cp $(HEADERS) $(DESTDIR)$(PREFIX)/include/$(LIBNAME)/

clean:
	rm -f $(OBJS) $(LIBRARY) $(TEST_BINS)
