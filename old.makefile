CXX      := g++
CXXFLAGS := -Wall -I . -g
# CXXGLAGS += -std=c++11 -stdlib=libstdc++
TARGET   := flang
LIBS     := jack

ifeq ($(CONFIG), yaml)
	LIBS += yaml-cpp
endif
ifeq ($(CONFIG), xml)
	LIBS += tinyxml2
endif

SRCDIR    := src
OBJDIR    := objs
TARGETDIR := build

SRCEXT := cpp
DEPEXT := d
OBJEXT := o

SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))

SOURCES := $(OBJECTS:%_config.$(SRCEXT)=)
ifeq ($(CONFIG), yaml)
	A := $(shell echo $(SOURCES))
	SOURCES += yaml_config.$(SRCEXT)
endif

OBJECTS := $(SOURCES:$(SRCDIR)/%.$(SRCEXT)=$(OBJDIR)/%.$(OBJEXT))

.PHONY: all
.PHONY: dirs
.PHONY: clean
.PHONY: cleaner
.PHONY: help
.PHONY: complete

all: $(TARGET)
	@echo 'Done!'
	@echo
	@echo 'Built sources: '$(SOURCES)
	@echo 'Built objects: '$(OBJECTS)

help:
	@echo 'run make with no target to make all...'
	@echo
	@echo 'Make $(TARGET):'
	@echo '    dirs:	generate directory tree'
	@echo '    clean:	rm ./$(OBJDIR)/'
	@echo '    cleaner:	rm ./$(OBJDIR)/ and ./$(TARGETDIR)/'
	@echo '    $(TARGET):	built $(TARGET) (generates dir tree automatically)'
	@echo '    complete:	generate .clang_complete'

complete:
	echo '$(CXXFLAGS) $(LIBS:%=-l%)' > .clang_complete

dirs:
	mkdir -p $(TARGETDIR)
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

cleaner: clean
	rm -rf $(TARGETDIR)

-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

# Link
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGETDIR)/$(TARGET) $^ $(LIBS:%=-l%)

# Compile
$(OBJDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT) dirs
	@echo 'Compiling obj "$@"'
	$(CXX) $(CXXFLAGS) -c -o $@ $<

	@echo 'Generating dep file: "$(OBJDIR)/$*.$(DEPEXT)"'
	$(CXX) $(CXXFLAGS) -MM $< | \
		tr -d '\\\n' |          \
		sed 's|.*:|$@:|' |      \
		awk -F: '{              \
			print $$0;          \
			split($$2,a," ");   \
			for (i in a) print(a[i]":"); \
		}' > $(OBJDIR)/$*.$(DEPEXT)


