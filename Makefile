VERSION		= "0.0.2"
BINARY		= voxel


CC			= clang
CFLAGS		= -std=c11 -march=native -Wall -Wextra -pedantic -Wno-gnu-zero-variadic-macro-arguments
CPPFLAGS	= -Idep/glfw/3.4/include -Idep/glad/include -Idep/cglm/0.9.6/include -Idep/stb/include -Idep/m4xdev/include -DVERSION=\"$(VERSION)\"
LDLIBS		= -Ldep/glfw/3.4/lib -lglfw -framework Cocoa -framework IOKit -framework CoreFoundation -lm

GPERF_PREFIX := $(shell brew --prefix gperftools)

#
# Configure above
################################################################################

SOURCES		:= $(shell find src -name '*.c')
OBJECTS		=  $(patsubst src/%.c,obj/%.o,$(SOURCES))

################################################################################

.PHONY: release
release: CFLAGS	+= -Werror -flto -O3
release: CPPFLAGS += -DLOG_LEVEL=0 -DNDEBUG
release: bin/$(BINARY)

.PHONY: profile
profile: CFLAGS	+= -Werror -O3 -gline-tables-only -fno-omit-frame-pointer -fno-optimize-sibling-calls -fdebug-info-for-profiling
profile: CPPFLAGS += -I$(GPERF_PREFIX)/include -DLOG_LEVEL=0 -DNDEBUG
profile: LDLIBS += -L$(GPERF_PREFIX)/lib -lprofiler
profile: disable_crash_reporter bin/$(BINARY)
profile:
	CPUPROFILE=voxel.prof CPUPROFILE_FREQUENCY=1000 ./bin/$(BINARY)
	pprof --http=:3000 ./bin/voxel voxel.prof

.PHONY: debug
debug: CFLAGS += -O2 -fsanitize=address,undefined,signed-integer-overflow -g
debug: CPPFLAGS += -DLOG_LEVEL=0 -DDEBUG
debug: disable_crash_reporter bin/$(BINARY)



################################################################################

bin/$(BINARY): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDLIBS)  -o $@

obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ -MMD -MP

DEPS=$(OBJECTS:.o=.d)
-include $(DEPS)

.PHONY: clean
clean:
	rm -rf obj bin

#
# Rules to disable and enable the MacOS crash reporter.
################################################################################

.PHONY: disable_crash_reporter
disable_crash_reporter:
	defaults write com.apple.CrashReporter DialogType none

.PHONY: enable_crash_reporter
enable_crash_reporter:
	defaults write com.apple.CrashReporter DialogType prompt


