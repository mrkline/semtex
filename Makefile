# Taken largely from http://scottmcpeak.com/autodepend/autodepend.html
# I mean to mess with another build systems (maybe scons) at some point,
# but will do just fine until then

CXXFLAGS= -std=c++11 -Wall
OBJS := main.o FileParser.o FileQueue.o ProcessorThread.o UnitReplacer.o IntegralReplacer.o SummationReplacer.o DerivReplacer.o

all: CXXFLAGS += -g
all: semtex
release: CXXFLAGS+= -O2 -DNDEBUG
release: semtex

# link
semtex: $(OBJS)
	$(CXX) $(CXXFLAGS) -pthread -lboost_regex -lboost_system -lboost_filesystem $(OBJS) -o semtex

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

# compile and generate dependency info;
# more complicated dependency computation, so all prereqs listed
# will also become command-less, prereq-less targets
#   sed:    strip the target (everything before colon)
#   sed:    remove any continuation backslashes
#   fmt -1: list words one per line
#   sed:    strip leading spaces
#   sed:    add trailing colons
%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $*.cpp -o $*.o
	$(CXX) -MM $(CXXFLAGS) $*.cpp > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

# remove compilation products
clean:
	rm -f semtex *.o *.d

.PHONY: clean
