ifeq ($(OS), Windows_NT)
	TARGET=SubSync.exe
	RM=cmd /C del /Q
else
	TARGET=SubSync
	RM=rm -f
endif

all: $(TARGET)

$(TARGET):	main.c
	gcc main.c -o $(TARGET) -Wall -Wextra -O2

run:	$(TARGET)
	$(TARGET) "Metropolis.1927.720p.BluRay.x264-YTS-it.srt" 127.584

clean:
	$(RM) $(TARGET)