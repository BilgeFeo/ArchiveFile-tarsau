#include "unpack.h"
#include "utils.h"

/* ─── Dahili Yardımcı Fonksiyonlar ─── */

/**
 * Arşiv dosyasının header bölümünü okuyup parse eder.
 * Başarılıysa 0, hata varsa -1 döndürür.
 */
static int parse_header(FILE *fp, Archive *archive, size_t *header_size)
{
    char size_buf[HEADER_SIZE_LEN + 1];
    char *header_data = NULL;
    char *ptr, *end;
    size_t records_len;

    archive->file_count = 0;

    /* İlk 10 byte: header boyutunu oku */
    if (fread(size_buf, 1, HEADER_SIZE_LEN, fp) != HEADER_SIZE_LEN) {
        print_error("Arsiv dosyasi uygunsuz veya bozuk!");
        return -1;
    }
    size_buf[HEADER_SIZE_LEN] = '\0';

    /* Header boyutunu sayıya çevir */
    *header_size = (size_t)strtoul(size_buf, &end, 10);
    if (*header_size <= HEADER_SIZE_LEN || *end != '\0') {
        /* strtoul boşlukları atlar, ancak sondaki geçersiz karakter kontrolü */
        /* Boşluk karakterlerini tolere et */
        while (*end == ' ') end++;
        if (*end != '\0' && end != size_buf + HEADER_SIZE_LEN) {
            print_error("Arsiv dosyasi uygunsuz veya bozuk!");
            return -1;
        }
        if (*header_size <= HEADER_SIZE_LEN) {
            print_error("Arsiv dosyasi uygunsuz veya bozuk!");
            return -1;
        }
    }

    /* Kayıt bölümünü oku */
    records_len = *header_size - HEADER_SIZE_LEN;
    header_data = (char *)malloc(records_len + 1);
    if (!header_data) {
        print_error("Bellek ayrilamadi.");
        return -1;
    }

    if (fread(header_data, 1, records_len, fp) != records_len) {
        print_error("Arsiv dosyasi uygunsuz veya bozuk!");
        free(header_data);
        return -1;
    }
    header_data[records_len] = '\0';

    /* Kayıtları parse et: |dosya_adı,izinler,boyut| */
    ptr = header_data;
    while (*ptr) {
        char name[256];
        unsigned int perm;
        unsigned long fsize;
        char *pipe_start, *pipe_end;
        char record[512];
        size_t rec_len;

        /* Sonraki '|' bul */
        pipe_start = strchr(ptr, '|');
        if (!pipe_start) break;

        /* Kapanış '|' bul */
        pipe_end = strchr(pipe_start + 1, '|');
        if (!pipe_end) {
            print_error("Arsiv dosyasi uygunsuz veya bozuk!");
            free(header_data);
            return -1;
        }

        /* Kayıt içeriğini çıkar */
        rec_len = (size_t)(pipe_end - pipe_start - 1);
        if (rec_len >= sizeof(record)) {
            print_error("Arsiv dosyasi uygunsuz veya bozuk!");
            free(header_data);
            return -1;
        }
        memcpy(record, pipe_start + 1, rec_len);
        record[rec_len] = '\0';

        /* Virgülle ayrılmış alanları parse et */
        if (sscanf(record, "%255[^,],%o,%lu", name, &perm, &fsize) != 3) {
            print_error("Arsiv dosyasi uygunsuz veya bozuk!");
            free(header_data);
            return -1;
        }

        if (archive->file_count >= MAX_FILES) {
            print_error("Arsiv dosyasinda cok fazla dosya kaydi var!");
            free(header_data);
            return -1;
        }

        strncpy(archive->files[archive->file_count].name, name,
                sizeof(archive->files[0].name) - 1);
        archive->files[archive->file_count].name[sizeof(archive->files[0].name) - 1] = '\0';
        archive->files[archive->file_count].permissions = (mode_t)perm;
        archive->files[archive->file_count].size = (size_t)fsize;
        archive->file_count++;

        /* Sonraki kayda ilerle */
        ptr = pipe_end + 1;
    }

    free(header_data);

    if (archive->file_count == 0) {
        print_error("Arsiv dosyasi uygunsuz veya bozuk!");
        return -1;
    }

    return 0;
}

/**
 * Parse edilmiş kayıtlara göre dosyaları arşivden çıkarır.
 * Başarılıysa 0, hata varsa -1 döndürür.
 */
static int extract_files(FILE *fp, Archive *archive, const char *dest_dir)
{
    int i;
    char filepath[512];
    char errmsg[1024];

    for (i = 0; i < archive->file_count; i++) {
        FILE *out;
        char *buf;
        size_t fsize = archive->files[i].size;

        /* Hedef dosya yolunu oluştur */
        if (dest_dir[0] != '\0') {
            snprintf(filepath, sizeof(filepath), "%s/%s", dest_dir, archive->files[i].name);
        } else {
            snprintf(filepath, sizeof(filepath), "%s", archive->files[i].name);
        }

        /* İçeriği oku */
        buf = (char *)malloc(fsize);
        if (!buf) {
            print_error("Bellek ayrilamadi.");
            return -1;
        }

        if (fread(buf, 1, fsize, fp) != fsize) {
            snprintf(errmsg, sizeof(errmsg),
                     "'%s' icerigi okunurken hata olustu!", archive->files[i].name);
            print_error(errmsg);
            free(buf);
            return -1;
        }

        /* Dosyayı yaz */
        out = fopen(filepath, "w");
        if (!out) {
            snprintf(errmsg, sizeof(errmsg),
                     "'%s' dosyasi olusturulamadi!", filepath);
            print_error(errmsg);
            free(buf);
            return -1;
        }

        if (fwrite(buf, 1, fsize, out) != fsize) {
            snprintf(errmsg, sizeof(errmsg),
                     "'%s' yazilirken hata olustu!", filepath);
            print_error(errmsg);
            fclose(out);
            free(buf);
            return -1;
        }

        fclose(out);
        free(buf);

        /* İzinleri ayarla */
        if (chmod(filepath, archive->files[i].permissions) != 0) {
            snprintf(errmsg, sizeof(errmsg),
                     "'%s' izinleri ayarlanirken hata olustu!", filepath);
            print_error(errmsg);
            /* İzin hatası kritik değil, devam et */
        }
    }

    return 0;
}

/* ─── Ana Unpack Fonksiyonu ─── */

int unpack_archive(Args *args)
{
    FILE *fp;
    Archive archive;
    size_t header_size;
    const char *dest_dir;
    int i;

    /* Arşiv dosyasını aç */
    fp = fopen(args->archive_file, "r");
    if (!fp) {
        print_error("Arsiv dosyasi uygunsuz veya bozuk!");
        return -1;
    }

    /* Header'ı parse et */
    if (parse_header(fp, &archive, &header_size) != 0) {
        fclose(fp);
        return -1;
    }

    /* Hedef dizin belirle */
    dest_dir = args->dest_dir;

    /* Dizin varsa oluştur */
    if (dest_dir[0] != '\0') {
        if (ensure_directory(dest_dir) != 0) {
            fclose(fp);
            return -1;
        }
    }

    /* Dosyaları çıkar */
    if (extract_files(fp, &archive, dest_dir) != 0) {
        fclose(fp);
        return -1;
    }

    fclose(fp);

    /* Başarı mesajı */
    if (dest_dir[0] != '\0') {
        printf("%s dizininde ", dest_dir);
    }

    for (i = 0; i < archive.file_count; i++) {
        if (i > 0) printf(", ");
        printf("%s", archive.files[i].name);
    }
    printf(" dosya(lari) acildi.\n");

    return 0;
}
