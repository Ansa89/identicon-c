BASE_NAME = libidenticon-c
VERSION = 0.0.0
PREFIX ?= /usr/local
LIBDIR ?= $(PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include

STATIC_LIB = $(BASE_NAME).a
PC_FILE = $(BASE_NAME).pc
HEADER = identicon-c.h
TARGET_ONLY = NO

SOURCES = identicon-c.c
OBJS = $(SOURCES:.c=.o)

CFLAGS = -Wall -Wextra -fPIC -Ilibs
LDFLAGS = -shared -lm

# Check what crypto library we will use
ifeq ($(USE_SODIUM), 1)
    DEPS += libsodium
    CFLAGS += -DUSE_SODIUM
else ifeq ($(USE_OPENSSL), 1)
    DEPS += openssl
    CFLAGS += -DUSE_OPENSSL
else
    SOURCES += libs/md5.c libs/sha1.c libs/sha256.c libs/sha512.c
endif

# Check what png library we will use
ifeq ($(MAKECMDGOALS), example)
ifeq ($(USE_CAIRO), 1)
    DEPS += cairo
    CFLAGS += -DUSE_CAIRO
else ifeq ($(USE_LIBPNG), 1)
    DEPS += libpng
    CFLAGS += -DUSE_LIBPNG
else ifeq ($(USE_STB), 1)
    CFLAGS += -DUSE_STB
else
    SOURCES += libs/lodepng.c
    CFLAGS += -DLODEPNG_NO_COMPILE_CPP
endif
endif

# Check if we can build identicon
ifdef DEPS
CHECK_DEPS = $(shell pkg-config --exists $(DEPS) || echo -n "error")
ifneq ($(CHECK_DEPS), error)
    CFLAGS += $(shell pkg-config --cflags $(DEPS))
    LDFLAGS += $(shell pkg-config --libs $(DEPS))
else ifneq ($(MAKECMDGOALS), clean)
    MISSING_LIBS = $(shell for lib in $(DEPS) ; do if ! pkg-config --exists $$lib ; then echo $$lib ; fi ; done)
    $(warning ERROR -- Cannot compile identicon-c)
    $(warning ERROR -- You need these libraries)
    $(warning ERROR -- $(MISSING_LIBS))
    $(error ERROR)
endif
endif

# Check on what system we are running
UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S), Linux)
    SHARED_EXT = so
    TARGET = $(BASE_NAME).$(SHARED_EXT).$(VERSION)
    SHARED_LIB = $(BASE_NAME).$(SHARED_EXT).$(shell echo $(VERSION) | rev | cut -d "." -f 1 | rev)
    LDFLAGS += -Wl,-soname=$(SHARED_LIB)
else ifeq ($(UNAME_S), FreeBSD)
    SHARED_EXT = so
    TARGET = $(BASE_NAME).$(SHARED_EXT).$(VERSION)
    SHARED_LIB = $(BASE_NAME).$(SHARED_EXT).$(shell echo $(VERSION) | rev | cut -d "." -f 1 | rev)
    LDFLAGS += -Wl,-soname=$(SHARED_LIB)
else ifeq ($(UNAME_S), Darwin)
    SHARED_EXT = dylib
    TARGET = $(BASE_NAME).$(VERSION).$(SHARED_EXT)
    SHARED_LIB = $(BASE_NAME).$(shell echo $(VERSION) | rev | cut -d "." -f 1 | rev).$(SHARED_EXT)
    LDFLAGS += -Wl,-install_name,$(SHARED_LIB)
else ifneq (, $(shell echo $(UNAME_S) | grep -E 'MSYS|MINGW|CYGWIN'))
    SHARED_EXT = dll
    TARGET = $(BASE_NAME).$(SHARED_EXT)
    TARGET_ONLY = YES
    NO_STATIC = 1
    LDFLAGS += -Wl,--out-implib,$(TARGET).a
endif


# Targets
all: $(TARGET)

%.o: %.c
	@echo "  CC    $@"
	@$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	@echo "  LD    $@"
	@$(CC) $(LDFLAGS) -shared $^ -o $@
	@if [ "$(NO_STATIC)" != "1" ]; then \
		echo "  AR    $(STATIC_LIB)" ;\
		ar rcs $(STATIC_LIB) $^ ;\
	fi

example: $(OBJS) example.o
	@echo "  LD    $@"
	@$(CC) -lm $(shell pkg-config --libs $(DEPS) 2>/dev/null) $^ -o $@

install: $(TARGET) $(HEADER) $(PC_FILE)
	@echo "Installing $(TARGET)"
	@install -D -m 0755 $(TARGET) $(abspath $(DESTDIR)/$(LIBDIR)/$(TARGET))
	@echo "Installing $(HEADER)"
	@install -D -m 0644 $(HEADER) $(abspath $(DESTDIR)/$(INCLUDEDIR)/$(HEADER))
	@echo "Installing $(PC_FILE)"
	@install -D -m 0644 $(PC_FILE) $(abspath $(DESTDIR)/$(PREFIX)/share/pkgconfig/$(PC_FILE))
	@if [ "$(NO_STATIC)" != "1" -a -e "$(STATIC_LIB)" ]; then \
		echo "Installing $(STATIC_LIB)" ;\
		install -m 0644 $(STATIC_LIB) $(abspath $(DESTDIR)/$(LIBDIR)/$(STATIC_LIB)) ;\
	fi
	@if [ "$(TARGET_ONLY)" != "YES" ]; then \
		cd $(abspath $(DESTDIR)/$(LIBDIR)) ;\
		ln -sf $(TARGET) $(SHARED_LIB) ;\
		ln -sf $(SHARED_LIB) $(BASE_NAME).$(SHARED_EXT) ;\
	fi
	@pc_file=$(abspath $(DESTDIR)/$(PREFIX)/share/pkgconfig/$(PC_FILE)) ;\
	sed -e 's:__PREFIX__:$(abspath $(PREFIX)):g' $$pc_file > temp_file && mv temp_file $$pc_file ;\
	sed -e 's:__LIBDIR__:$(abspath $(LIBDIR)):g' $$pc_file > temp_file && mv temp_file $$pc_file ;\
	sed -e 's:__INCLUDEDIR__:$(abspath $(INCLUDEDIR)):g' $$pc_file > temp_file && mv temp_file $$pc_file ;\
	sed -e 's:__VERSION__:$(VERSION):g' $$pc_file > temp_file && mv temp_file $$pc_file ;\
	sed -e 's:__LIBS__:$(DEPS):g' $$pc_file > temp_file && mv temp_file $$pc_file

clean:
	rm -f *.o libs/*.o example $(TARGET) $(STATIC_LIB)

.PHONY: all clean install
