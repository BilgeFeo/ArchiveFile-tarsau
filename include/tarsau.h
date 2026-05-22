#ifndef TARSAU_H
#define TARSAU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

/* ─── Sabitler ─── */
#define MAX_FILES        32
#define MAX_TOTAL_SIZE   (200UL * 1024UL * 1024UL)   /* 200 MB */
#define HEADER_SIZE_LEN  10
#define DEFAULT_OUTPUT   "a.sau"
#define SAU_EXTENSION    ".sau"

/* ─── Çalışma Modları ─── */
typedef enum {
    MODE_NONE,
    MODE_PACK,    /* -b */
    MODE_UNPACK   /* -a */
} Mode;

/* ─── Tek Dosya Kayıt Yapısı ─── */
typedef struct {
    char  name[256];     /* dosya adı (sadece basename)  */
    char  fullpath[512]; /* orijinal tam yol (okuma için)*/
    mode_t permissions;  /* orijinal dosya izinleri      */
    size_t size;         /* dosya boyutu (byte)          */
} FileEntry;

/* ─── Arşiv Yapısı ─── */
typedef struct {
    FileEntry files[MAX_FILES];
    int       file_count;
} Archive;

/* ─── Komut Satırı Argüman Sonucu ─── */
typedef struct {
    Mode  mode;
    char  input_files[MAX_FILES][256];
    int   input_count;
    char  output_file[256];
    char  archive_file[256];
    char  dest_dir[256];
} Args;

#endif /* TARSAU_H */
