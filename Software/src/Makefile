SOURCES = main.cpp \
	mock_arduino.cpp \
	TestSupport.cpp \
	WordBuffer.cpp WordBufferTest.cpp \
	TennisMachine.cpp TennisMachineTest.cpp 


OBJECTS := $(addsuffix .o, $(addprefix .build/, $(basename $(SOURCES))))
DEPFILES := $(subst .o,.dep, $(subst .build/,.deps/, $(OBJECTS)))
TESTCPPFLAGS = -D_TEST_ -Itest -Iarduino
CPPDEPFLAGS = -MMD -MP -MF .deps/$(subst .allsrc/,,$(basename $<)).dep
RUNTEST := $(if $(COMSPEC), runtests.exe, runtests)



FSOURCES = testFramework.cpp \
	mock_arduino.cpp \
	TestSupport.cpp TestSupportTest.cpp

FOBJECTS := $(addsuffix .o, $(addprefix .build/, $(basename $(FSOURCES))))
FDEPFILES := $(subst .o,.dep, $(subst .build/,.deps/, $(FOBJECTS)))
FTESTCPPFLAGS = -D_TEST_ -Itest -Iarduino
FCPPDEPFLAGS = -MMD -MP -MF .deps/$(subst .allsrc/,,$(basename $<)).dep
FRUNTEST := $(if $(COMSPEC), frameworktest.exe, frameworktest)



all: runframeworktest run

.build/%.o: .allsrc/%.cpp
	mkdir -p .deps/$(dir $<)
	mkdir -p .build/$(dir $<)
	$(COMPILE.cpp) $(TESTCPPFLAGS) $(CPPDEPFLAGS) -o $@ $<

compile: copy $(OBJECTS)

fcompile: copy $(FOBJECTS)

copy:
	mkdir -p .allsrc
	-cp test/* .allsrc 
	-cp morse/* .allsrc

runtests: compile
	$(CC) $(OBJECTS) -lstdc++ -o $@

run: runtests
	./runtests

runframeworktest: frameworktest
	./frameworktest
	
	
frameworktest: fcompile
	$(CC) $(FOBJECTS) -lstdc++ -o $@
	
clean:
	@rm -rf .deps/ .build/ .allsrc $(RUNTEST) $(FRUNTEST)

-include $(DEPFILES)


# works: g++    -c -D_TEST_ -Itest -Iarduino -MMD -MP -MF .deps/main.dep -o .build/main.o .allsrc/main.cpp
# nwrks: g++    -c -D_TEST_ -Itest -Iarduino -MMD -MP -MF .deps/.allsrc/main.dep -o .build/main.o .allsrc/main.cpp
#        g++    -c -D_TEST_ -Itest -Iarduino -MMD -MP -MF .deps/main.dep -o .build/main.o .allsrc/main.cpp
