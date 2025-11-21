#include "interactive_input.h"

#include <dlfcn.h>


typedef int   (*gcd_func)(int a, int b);
typedef char* (*convert_func)(int num);

convert_func convert = NULL;

gcd_func gcd = NULL;


void open_library(void** library, const char* const* paths, size_t size) {
    if (!library) return;

    static int id = 0;

    const char* path = paths[id++ % size];

    *library = dlopen(path, RTLD_LOCAL | RTLD_NOW);
    if (!*library) {
        const char msg[] = "Failed to load library";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    convert = dlsym(*library, "convert");
    if (!convert) {
        const char msg[] = "Failed to find convert";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    gcd = dlsym(*library, "gcd");
    if (!gcd) {
        const char msg[] = "Failed to find gcd";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        const char msg[] = "Usage: ./test_dynamic_library <path_to_library1> <path_to_library2>";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    char* library_path1 = argv[1];
    char* library_path2 = argv[2];

    const char* paths[] = {library_path1, library_path2};
    size_t paths_size = sizeof(paths) / sizeof(paths[0]);


    void* library = NULL;
    open_library(&library, paths, paths_size);

    putstr("Commands list: 0 - swap library, 1 - convert, 2 - gcd, -1 - exit\n");

    while (1) {
        putstr("Enter command: ");

        int command;
        if (!read_int(&command) || command == -1)
            break;

        switch (command) {
        case 0:
        {
            dlclose(library);
            library = NULL;

            open_library(&library, paths, paths_size);
            putstr("Successfully loaded new library\n");
            break;
        }
        case 1: {
            putstr("Enter number: ");
            int num;
            if (!read_int(&num)) break;

            char *res = convert(num);
            putstr("Converted to number system: ");
            putstr(res);
            putstr("\n");
            free(res);
            break;
        }

        case 2: {
            int a, b;

            putstr("Enter first number: ");
            if (!read_int(&a)) break;

            putstr("Enter second number: ");
            if (!read_int(&b)) break;

            char outbuf[128];
            int n = snprintf(outbuf, sizeof(outbuf),
                             "GCD of %d and %d is %d\n",
                             a, b, gcd(a, b));
            write(1, outbuf, n);
            break;
        }

        default:
            putstr("Unknown command\n");
        }
    }

    dlclose(library);
    exit(EXIT_SUCCESS);
}
