#include "pack.h"
#include "utils.h"
#include <libgen.h>

/* ─── Dahili Yardımcı Fonksiyonlar ─── */

/**
 * Giriş dosyalarını doğrular: erişilebilirlik, ASCII kontrolü, boyut limiti.
 * Doğrulama başarılıysa archive yapısını doldurur.
 * Başarılıysa 0, hata varsa -1 döndürür.
 */
static int validate_and_collect(Args *args, Archive *archive)
{
    struct stat st;
    size_t total_size = 0;
    int i;
    char errmsg[512];

    archive->file_count = 0;

    for (i = 0; i < args->input_count; i++) {
        const char *fname = args->input_files[i];

        /* Dosya mevcut mu? */
        if (stat(fname, &st) != 0) {
            snprintf(errmsg, sizeof(errmsg), "'%s' dosyasi bulunamadi!", fname);
            print_error(errmsg);
            return -1;
        }

        /* Normal dosya mı? */
        if (!S_ISREG(st.st_mode)) {
            snprintf(errmsg, sizeof(errmsg), "'%s' normal bir dosya degil!", fname);
            print_error(errmsg);
            return -1;
        }

        /* ASCII metin dosyası mı? */
        if (!is_text_file(fname)) {
            snprintf(errmsg, sizeof(errmsg),
                     "%s giris dosyasinin formati uyumsuzdur!", fname);
            print_error(errmsg);
            return -1;
        }

        /* Orijinal tam yolu sakla (dosya okuma için) */
        strncpy(archive->files[i].fullpath, fname, sizeof(archive->files[i].fullpath) - 1);
        archive->files[i].fullpath[sizeof(archive->files[i].fullpath) - 1] = '\0';

        /* Sadece basename'i kaydet (header'a yazılacak) */
        {
            char tmp[512];
            strncpy(tmp, fname, sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';
            strncpy(archive->files[i].name, basename(tmp), sizeof(archive->files[i].name) - 1);
            archive->files[i].name[sizeof(archive->files[i].name) - 1] = '\0';
        }
        archive->files[i].permissions = st.st_mode & 0777;
        archive->files[i].size = (size_t)st.st_size;

        total_size += archive->files[i].size;
        archive->file_count++;
    }

    /* Toplam boyut kontrolü */
    if (total_size > MAX_TOTAL_SIZE) {
        print_error("Giris dosyalarinin toplam boyutu 200 MB'i asamaz!");
        return -1;
    }

    return 0;
}

/**
 * Header (organizasyon) bölümünü oluşturur.
 * Sonuç: header_buf tamponuna yazılır, header_len toplam uzunluk.
 * header_buf çağıran tarafından free() edilmelidir.
 * Başarılıysa 0, hata varsa -1 döndürür.
 */
static int build_header(Archive *archive, char **header_buf, size_t *header_len)
{
    char records[65536]; /* kayıt dizesi */
    int  offset = 0;
    int  i;
    size_t total_header_size;

    records[0] = '\0';

    /* Her dosya için kayıt oluştur: |dosya_adı,izinler,boyut| */
    for (i = 0; i < archive->file_count; i++) {
        int written = snprintf(records + offset, sizeof(records) - (size_t)offset,
                               "|%s,%o,%zu|",
                               archive->files[i].name,
                               archive->files[i].permissions,
                               archive->files[i].size);
        if (written < 0 || (size_t)written >= sizeof(records) - (size_t)offset) {
            print_error("Header olusturulurken hata olustu (tampon yetersiz).");
            return -1;
        }
        offset += written;
    }

    /* Toplam header boyutu = 10 (sabit alan) + kayıt uzunluğu */
    total_header_size = HEADER_SIZE_LEN + (size_t)offset;

    /* Tampon ayır */
    *header_buf = (char *)malloc(total_header_size + 1);
    if (!*header_buf) {
        print_error("Bellek ayrilamadi.");
        return -1;
    }

    /* İlk 10 byte: header boyutunu sağa dayalı yaz */
    snprintf(*header_buf, HEADER_SIZE_LEN + 1, "%*zu", HEADER_SIZE_LEN, total_header_size);

    /* Kayıtları kopyala */
    memcpy(*header_buf + HEADER_SIZE_LEN, records, (size_t)offset);
    (*header_buf)[total_header_size] = '\0';

    *header_len = total_header_size;

    return 0;
}

/**
 * Arşiv dosyasını yazar: header + tüm dosya içerikleri.
 * Başarılıysa 0, hata varsa -1 döndürür.
 */
static int write_archive(const char *output_path, const char *header,
                          size_t header_len, Archive *archive)
{
    FILE *out;
    FILE *in;
    int i;
    char buf[8192];
    size_t n;

    out = fopen(output_path, "w");
    if (!out) {
        print_error("Cikti dosyasi olusturulamadi.");
        return -1;
    }

    /* Header yaz */
    if (fwrite(header, 1, header_len, out) != header_len) {
        print_error("Header yazilirken hata olustu.");
        fclose(out);
        return -1;
    }

    /* Her dosyanın içeriğini sırayla yaz */
    for (i = 0; i < archive->file_count; i++) {
        in = fopen(archive->files[i].fullpath, "r");
        if (!in) {
            char errmsg[512];
            snprintf(errmsg, sizeof(errmsg),
                     "'%s' dosyasi okunamadi!", archive->files[i].fullpath);
            print_error(errmsg);
            fclose(out);
            return -1;
        }

        while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
            if (fwrite(buf, 1, n, out) != n) {
                print_error("Dosya icerigi yazilirken hata olustu.");
                fclose(in);
                fclose(out);
                return -1;
            }
        }

        fclose(in);
    }

    fclose(out);
    return 0;
}

/* ─── Ana Pack Fonksiyonu ─── */

int pack_files(Args *args)
{
    Archive archive;
    char *header = NULL;
    size_t header_len = 0;
    int ret;

    /* 1. Doğrulama ve bilgi toplama */
    if (validate_and_collect(args, &archive) != 0) {
        return -1;
    }

    /* 2. Header oluştur */
    if (build_header(&archive, &header, &header_len) != 0) {
        return -1;
    }

    /* 3. Arşiv dosyasını yaz */
    ret = write_archive(args->output_file, header, header_len, &archive);
    free(header);

    if (ret != 0) {
        return -1;
    }

    printf("Dosyalar birlestirildi.\n");
    return 0;
}
