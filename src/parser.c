#include "parser.h"
#include "utils.h"

/**
 * Komut satırı argümanlarını ayrıştırır.
 *
 * Kullanım:
 *   tarsau -b dosya1 dosya2 ... [-o çıktı.sau]
 *   tarsau -a arşiv.sau [dizin]
 */
int parse_args(int argc, char *argv[], Args *args)
{
    /* Yapıyı temizle */
    memset(args, 0, sizeof(Args));
    args->mode = MODE_NONE;
    strcpy(args->output_file, DEFAULT_OUTPUT);

    if (argc < 3) {
        print_error("Kullanim: tarsau -b dosya1 dosya2 ... [-o cikti.sau]\n"
                     "          tarsau -a arsiv.sau [dizin]");
        return -1;
    }

    /* ─── MOD BELİRLE ─── */
    if (strcmp(argv[1], "-b") == 0) {
        args->mode = MODE_PACK;
    } else if (strcmp(argv[1], "-a") == 0) {
        args->mode = MODE_UNPACK;
    } else {
        print_error("Gecersiz mod. -b (birlestir) veya -a (ac) kullanin.");
        return -1;
    }

    /* ─── PACK MODU ARG PARSE ─── */
    if (args->mode == MODE_PACK) {
        int i;
        for (i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                /* -o'dan sonra çıktı dosya adı gelmeli */
                if (i + 1 >= argc) {
                    print_error("-o parametresinden sonra dosya adi belirtilmelidir.");
                    return -1;
                }
                strncpy(args->output_file, argv[i + 1], sizeof(args->output_file) - 1);
                args->output_file[sizeof(args->output_file) - 1] = '\0';
                i++; /* -o'nun değerini atla */
            } else {
                /* Giriş dosyası */
                if (args->input_count >= MAX_FILES) {
                    print_error("Giris dosya sayisi en fazla 32 olabilir.");
                    return -1;
                }
                strncpy(args->input_files[args->input_count], argv[i],
                        sizeof(args->input_files[0]) - 1);
                args->input_files[args->input_count][sizeof(args->input_files[0]) - 1] = '\0';
                args->input_count++;
            }
        }

        if (args->input_count == 0) {
            print_error("En az bir giris dosyasi belirtilmelidir.");
            return -1;
        }
    }

    /* ─── UNPACK MODU ARG PARSE ─── */
    if (args->mode == MODE_UNPACK) {
        /* -a'dan sonra en fazla 2 parametre */
        if (argc - 2 > 2) {
            print_error("-a parametresinden sonra en fazla 2 parametre alinabilir.");
            return -1;
        }

        /* İlk parametre: arşiv dosyası */
        strncpy(args->archive_file, argv[2], sizeof(args->archive_file) - 1);
        args->archive_file[sizeof(args->archive_file) - 1] = '\0';

        /* .sau uzantı kontrolü */
        {
            size_t len = strlen(args->archive_file);
            if (len < 5 || strcmp(args->archive_file + len - 4, SAU_EXTENSION) != 0) {
                print_error("Arsiv dosyasi uygunsuz veya bozuk!");
                return -1;
            }
        }

        /* İkinci parametre: hedef dizin (opsiyonel) */
        if (argc >= 4) {
            strncpy(args->dest_dir, argv[3], sizeof(args->dest_dir) - 1);
            args->dest_dir[sizeof(args->dest_dir) - 1] = '\0';
        }
        /* dest_dir boşsa, mevcut dizin kullanılacak */
    }

    return 0;
}
