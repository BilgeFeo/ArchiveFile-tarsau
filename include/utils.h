#ifndef UTILS_H
#define UTILS_H

#include "tarsau.h"

/**
 * Dosyanın ASCII metin dosyası olup olmadığını kontrol eder.
 * Metin dosyasıysa 1, değilse 0 döndürür.
 */
int is_text_file(const char *path);

/**
 * Standart hata çıktısına hata mesajı yazdırır.
 */
void print_error(const char *msg);

/**
 * Verilen dizin yolunun var olmasını garantiler.
 * Yoksa oluşturur (gerekirse iç içe dizinler dahil).
 * Başarılıysa 0, hata varsa -1 döndürür.
 */
int ensure_directory(const char *path);

/**
 * Dosyanın izin bitlerini oktal string olarak döndürür (örn: "644").
 * Statik tampona yazar; döndürülen pointer sonraki çağrıda geçersiz olur.
 */
const char *get_file_permissions_str(const char *path);

#endif /* UTILS_H */
