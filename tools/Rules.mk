TARGET_$(dir) := $(dir)/mdlconv
TARGETS       := $(TARGETS) $(TARGET_$(dir))
SRC_$(dir)    := $(wildcard $(dir)/*.c)

$(TARGET_$(dir)): $(SRC_$(dir))
	$(CC) $(CFLAGS) $(INCS) -lm -lassimp $(SRC_$(dir)) -o $(TARGET_$(dir))
