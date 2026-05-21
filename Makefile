# ──────────────────────────────────────────────
# tarsau - Basit Arsivleme Araci Makefile
# ──────────────────────────────────────────────

CC       = gcc
CFLAGS   = -Wall -Wextra -I include
SRCDIR   = src
LIBDIR   = lib
BINDIR   = bin
INCDIR   = include

# Kaynak ve nesne dosyalari
SRCS     = $(SRCDIR)/main.c $(SRCDIR)/parser.c $(SRCDIR)/pack.c $(SRCDIR)/unpack.c $(SRCDIR)/utils.c
OBJS     = $(LIBDIR)/main.o $(LIBDIR)/parser.o $(LIBDIR)/pack.o $(LIBDIR)/unpack.o $(LIBDIR)/utils.o

# Hedef
TARGET   = $(BINDIR)/tarsau

# Varsayilan hedef
all: dirs $(TARGET)

# Dizinleri olustur
dirs:
	@mkdir -p $(BINDIR) $(LIBDIR)

# Baglama (linking)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Derleme kurallari
$(LIBDIR)/main.o: $(SRCDIR)/main.c $(INCDIR)/tarsau.h $(INCDIR)/parser.h $(INCDIR)/pack.h $(INCDIR)/unpack.h $(INCDIR)/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBDIR)/parser.o: $(SRCDIR)/parser.c $(INCDIR)/parser.h $(INCDIR)/tarsau.h $(INCDIR)/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBDIR)/pack.o: $(SRCDIR)/pack.c $(INCDIR)/pack.h $(INCDIR)/tarsau.h $(INCDIR)/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBDIR)/unpack.o: $(SRCDIR)/unpack.c $(INCDIR)/unpack.h $(INCDIR)/tarsau.h $(INCDIR)/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBDIR)/utils.o: $(SRCDIR)/utils.c $(INCDIR)/utils.h $(INCDIR)/tarsau.h
	$(CC) $(CFLAGS) -c $< -o $@

# Temizlik
clean:
	rm -f $(LIBDIR)/*.o $(TARGET)

# Yeniden derleme
rebuild: clean all

.PHONY: all dirs clean rebuild
