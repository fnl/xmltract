PROG = xmltract
OBJECTS = 

#CC = cc
CFLAGS += -g -Wall -std=c99 -include standard.h -DG_LOG_DOMAIN=\"xmltract\" `pkg-config --cflags libxml-2.0`
LDFLAGS +=
LDLIBS += `pkg-config --libs libxml-2.0`

$(PROG): $(OBJECTS)

clean:
	rm -f *.o xmltract
	rm -rf xmltract.dSYM
