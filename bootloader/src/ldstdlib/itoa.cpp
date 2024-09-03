#include <efi/efi_datatypes.h>

#include <ldstdlib.hpp>

INTN Loader::itoa(INTN x, CHAR16* buffer, INT32 radix) {
    CHAR16 tmp[12];
    CHAR16 *tp = tmp;

    INTN i;
    UINTN v;

    BOOLEAN sign = (radix == 10 && x < 0);

    if (sign) {
        v = -x;
    } else {
        v = static_cast<UINTN>(x);
    }

    while (v || tp == tmp) {
        i = v % radix;
        v /= radix;

        if (i < 10) {
            *tp++ = i + u'0';
        } else {
            *tp++ = i + u'a' - 10;
        }
    }

    INTN len = tp - tmp;
    
    if (sign) {
        *buffer++ = '-';
        ++len;
    }

    while (tp > tmp) {
        *buffer++ = *--tp;
    }
    *buffer++ = '\0';

    return len;
}

INTN Loader::utoa(UINTN x, CHAR16* buffer, INT32 radix) {
    CHAR16 tmp[12];
    CHAR16 *tp = tmp;

    UINTN i;
    UINTN v = x;

    while (v || tp == tmp) {
        i = v % radix;
        v /= radix;

        if (i < 10) {
            *tp++ = i + u'0';
        } else {
            *tp++ = i + u'a' - 10;
        }
    }

    INTN len = tp - tmp;

    while (tp > tmp) {
        *buffer++ = *--tp;
    }
    *buffer++ = '\0';

    return len;
}
