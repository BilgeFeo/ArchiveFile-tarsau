#include "utils.h"
#include <ctype.h>

/**
 * Dosyanın ASCII metin dosyası olup olmadığını kontrol eder.
 * Tüm byte'lar yazdırılabilir ASCII veya beyaz boşluk (\n, \r, \t) olmalıdır.
 * Metin dosyasıysa 1, değilse 0 döndürür.
 */
int is_text_file(const char *path)
{
    FILE *fp;
    int ch;

    fp = fopen(path, "r");
    if (!fp) {
        return 0;
    }

    while ((ch = fgetc(fp)) != EOF) {
        /* ASCII aralığı: 0x00 - 0x7F */
        if (ch > 127) {
            fclose(fp);
            return 0;
        }
        /* Yazdırılabilir karakter veya beyaz boşluk (tab, newline, carriage return) */
        if (!isprint(ch) && ch != '\n' && ch != '\r' && ch != '\t') {
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return 1;
}

/**
 * Standart hata çıktısına hata mesajı yazdırır.
 */
void print_error(const char *msg)
{
    fprintf(stderr, "tarsau: %s\n", msg);
}

/**
 * Verilen dizin yolunun var olmasını garantiler.
 * Yoksa oluşturur (gerekirse iç içe dizinler dahil, mkdir -p benzeri).
 * Başarılıysa 0, hata varsa -1 döndürür.
 */
int ensure_directory(const char *path)
{
    struct stat st;
    char tmp[512];
    char *p = NULL;
    size_t len;

    /* Dizin zaten var mı? */
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return 0;
        }
        print_error("Belirtilen yol bir dizin degil.");
        return -1;
    }

    /* Yolu kopyala ve iç içe dizinleri oluştur */
    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);

    /* Sondaki '/' varsa kaldır */
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = '\0';
    }

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                perror("mkdir");
                return -1;
            }
            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        perror("mkdir");
        return -1;
    }

    return 0;
}

/**
 * Dosyanın izin bitlerini oktal string olarak döndürür (örn: "644").
 * Statik tampona yazar; döndürülen pointer sonraki çağrıda geçersiz olur.
 */
const char *get_file_permissions_str(const char *path)
{
    static char perm_str[8];
    struct stat st;

    if (stat(path, &st) != 0) {
        return "000";
    }

    /* Sadece user/group/other izinlerini al (son 9 bit) */
    snprintf(perm_str, sizeof(perm_str), "%o", st.st_mode & 0777);
    return perm_str;
}
