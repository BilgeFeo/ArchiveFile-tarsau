#ifndef PACK_H
#define PACK_H

#include "tarsau.h"

/**
 * Girdi dosyalarını doğrular, header ve içerikleri oluşturarak
 * .sau arşiv dosyasına yazar.
 * Başarılıysa 0, hata varsa -1 döndürür.
 */
int pack_files(Args *args);

#endif /* PACK_H */
