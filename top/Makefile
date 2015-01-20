CFLAGS =	-pedantic -Wall -Wextra -std=c99 -rdynamic -D_XOPEN_SOURCE_EXTENDED

OBJS =		Affinity.o AffinityPanel.o AvailableColumnsPanel.o AvailableMetersPanel.o BatteryMeter.o\
 CategoriesPanel.o CheckItem.o ClockMeter.o ColorsPanel.o ColumnsPanel.o CPUMeter.o CRT.o\
 DisplayOptionsPanel.o FunctionBar.o Hashtable.o HostnameMeter.o htop.o IOPriority.o Header.o\
 IOPriorityPanel.o ListItem.o LoadAverageMeter.o MemoryMeter.o Meter.o MetersPanel.o Object.o\
 OpenFilesScreen.o Panel.o Process.o ProcessList.o RichString.o ScreenManager.o Settings.o\
 SignalsPanel.o String.o SwapMeter.o TasksMeter.o TraceScreen.o UptimeMeter.o UsersTable.o Vector.o\

LIBS = -lm -lncurses

TARGET =	top

$(TARGET):	$(OBJS) config.h
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
