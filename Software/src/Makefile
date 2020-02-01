SOURCES = main.cpp \
	mock_arduino.cpp \
	WordBuffer.cpp

OBJECTS := $(addsuffix .o, $(addprefix .build/, $(basename $(SOURCES))))
DEPFILES := $(subst .o,.dep, $(subst .build/,.deps/, $(OBJECTS)))
TESTCPPFLAGS = -D_TEST_ -Itest -Iarduino
CPPDEPFLAGS = -MMD -MP -MF .deps/$(subst .allsrc/,,$(basename $<)).dep
RUNTEST := $(if $(COMSPEC), runtest.exe, runtest)

all: runtests

.build/%.o: .allsrc/%.cpp
	mkdir -p .deps/$(dir $<)
	mkdir -p .build/$(dir $<)
	$(COMPILE.cpp) $(TESTCPPFLAGS) $(CPPDEPFLAGS) -o $@ $<

compile: copy $(OBJECTS)

copy:
	@echo "o" $(OBJECTS)
	@echo "d" $(DEPFILES)
	mkdir -p .allsrc
	-cp test/* .allsrc 
	-cp morse/* .allsrc

runtests: compile
	$(CC) $(OBJECTS) -lstdc++ -o $@

clean:
	@rm -rf .deps/ .build/ .allsrc $(RUNTEST)

-include $(DEPFILES)


# works: g++    -c -D_TEST_ -Itest -Iarduino -MMD -MP -MF .deps/main.dep -o .build/main.o .allsrc/main.cpp
# nwrks: g++    -c -D_TEST_ -Itest -Iarduino -MMD -MP -MF .deps/.allsrc/main.dep -o .build/main.o .allsrc/main.cpp
#        g++    -c -D_TEST_ -Itest -Iarduino -MMD -MP -MF .deps/main.dep -o .build/main.o .allsrc/main.cpp
