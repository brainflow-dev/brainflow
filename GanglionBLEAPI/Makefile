CC = g++

TARGET = GanglionLibrary.so

MACHINE = $(shell $(CC) -dumpmachine)
PLATFORM = POSIX
LIBS = -lm
LDFLAGS = -shared
CFLAGS = -O1 -Wall -c -fmessage-length=0 -fPIC -std=c++11
ifeq ($(DEBUG), y)  
	CFLAGS += -g -DDEBUG
endif

SRCS := $(wildcard ./Bglib/*.cpp ../src/utils/timestamp.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(SRCS:.cpp=.d)

all: $(TARGET)

%.o: %.cpp
	$(CC) -I./Interface/ -I../src/utils/ $(CFLAGS) -DPLATFORM_$(PLATFORM) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"

$(TARGET): $(OBJS)
	@echo 'Building target: $@'
	$(CC) -o"$(TARGET)" $(OBJS) $(LIBS) $(LDFLAGS)
	@echo 'Finished building target: $@'

clean:
	-$(RM) $(OBJS) $(DEPS) $(TARGET)

.PHONY: all clean

