Target      := ps4_figure_emu.prx
ODIR        := build
SDIR        := src
CFILES      := $(wildcard $(SDIR)/*.c)
OBJS        := $(patsubst $(SDIR)/%.c, $(ODIR)/%.o, $(CFILES))

CC          := clang
CFLAGS      := -target x86_64-scei-ps4-elf -O2 -fPIC -I$(OO_TOOLCHAIN)/include
LDFLAGS     := -target x86_64-scei-ps4-elf -shared -L$(OO_TOOLCHAIN)/lib -lScePad -lSceUsbd -lSceVideoOut

$(Target): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(ODIR)/%.o: $(SDIR)/%.c
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(ODIR) $(Target)
