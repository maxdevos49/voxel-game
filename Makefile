VERSION		= 0.0.1
BINARY		= voxel


CC			= clang
CFLAGS		= -std=c11 -Wall -Wextra -pedantic -Wno-gnu-zero-variadic-macro-arguments -Idep/glfw/3.4/include -Idep/glad/include -Idep/cglm/0.9.6/include -Idep/stb/include -Idep/m4xdev/include
CPPFLAGS	= -DVERSION=\"$(VERSION)\"
LDLIBS		= -Ldep/glfw/3.4/lib -lglfw -framework Cocoa -framework IOKit -framework CoreFoundation -lm

#
# Configure above
################################################################################

SOURCES		:= $(shell find src -name '*.c')
OBJECTS		=  $(patsubst src/%.c,obj/%.o,$(SOURCES))

################################################################################

.PHONY: release
release: CFLAGS	+= -O3 -Werror -march=native -flto
release: CPPFLAGS += -DLOG_LEVEL=0 -DNDEBUG
release: bin/$(BINARY)

.PHONY: run-release
run-release: clean release
	./bin/$(BINARY)

.PHONY: debug
debug: CFLAGS += -g -march=native -fsanitize=address,undefined,signed-integer-overflow
debug: CPPFLAGS += -DLOG_LEVEL=0 -DDEBUG
debug: disable_crash_reporter bin/$(BINARY)

.PHONY: run-debug
run-debug: clean debug
	./bin/$(BINARY)

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


