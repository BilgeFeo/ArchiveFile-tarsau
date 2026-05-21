#ifndef UNPACK_H
#define UNPACK_H

#include "tarsau.h"

/**
 * .sau arşiv dosyasını açarak dosyaları hedef dizine çıkarır.
 * Başarılıysa 0, hata varsa -1 döndürür.
 */
int unpack_archive(Args *args);

#endif /* UNPACK_H */
