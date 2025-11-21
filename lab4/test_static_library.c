#include "interactive_input.h"
#include "contracts.h"


int main() {
    putstr("Commands list: 1 - convert, 2 - gcd, -1 - exit\n");

    while (1) {
        putstr("Enter command: ");

        int command;
        if (!read_int(&command) || command == -1)
            break;

        switch (command) {
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
    exit(EXIT_SUCCESS);
}
