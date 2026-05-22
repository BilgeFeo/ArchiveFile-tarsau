#ifndef PARSER_H
#define PARSER_H

#include "tarsau.h"

/**
 * Komut satırı argümanlarını ayrıştırır.
 * Başarılıysa 0, hata varsa -1 döndürür.
 */
int parse_args(int argc, char *argv[], Args *args);

#endif /* PARSER_H */
