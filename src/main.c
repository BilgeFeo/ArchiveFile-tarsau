#include "tarsau.h"
#include "parser.h"
#include "pack.h"
#include "unpack.h"
#include "utils.h"

int main(int argc, char *argv[])
{
    Args args;

    if (parse_args(argc, argv, &args) != 0) {
        return EXIT_FAILURE;
    }

    switch (args.mode) {
        case MODE_PACK:
            if (pack_files(&args) != 0)
                return EXIT_FAILURE;
            break;
        case MODE_UNPACK:
            if (unpack_archive(&args) != 0)
                return EXIT_FAILURE;
            break;
        default:
            print_error("Bilinmeyen mod.");
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
