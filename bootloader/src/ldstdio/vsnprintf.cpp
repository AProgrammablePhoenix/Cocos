#include <limits.h>
#include <stdarg.h>
#include <wchar.h>

#include <efi/efi.h>
#include <efi/efi_datatypes.h>

#include <ldstdio.hpp>
#include <ldstdlib.hpp>

typedef enum {
    hh,
    h,
    none,
    l,
    ll,
    j,
    z,
    t,
    L  
} kvsnprintf_length_modifier;

namespace {
    #define case_fetch_ret(lmodif, type, ret_type) case lmodif: return (ret_type)va_arg(*vlist, type)
    static inline INTN fetch_signed_number(
        IN kvsnprintf_length_modifier lmodif,
        IN va_list* vlist,
        OUT BOOLEAN* success
    ) {
        *success = true;

        switch (lmodif) {
            case_fetch_ret(hh, int, INTN);
            case_fetch_ret(h, int, INTN);
            case_fetch_ret(none, int, INTN);
            case_fetch_ret(l, long, INTN);
            case_fetch_ret(ll, long long, INTN);
            case_fetch_ret(j, intmax_t, INTN);
            case_fetch_ret(z, size_t, INTN);
            case_fetch_ret(t, ptrdiff_t, INTN);
            default:
                *success = false;
        }

        return 0;
    }

    static inline UINTN fetch_unsigned_number(
        IN kvsnprintf_length_modifier lmodif,
        IN va_list* vlist,
        OUT BOOLEAN* success
    ) {
        *success = true;

        switch (lmodif) {
            case_fetch_ret(hh, unsigned int, UINTN);
            case_fetch_ret(h, unsigned int, UINTN);
            case_fetch_ret(none, unsigned int, UINTN);
            case_fetch_ret(l, unsigned long, UINTN);
            case_fetch_ret(ll, unsigned long long, UINTN);
            case_fetch_ret(j, uintmax_t, UINTN);
            case_fetch_ret(z, size_t, UINTN);
            case_fetch_ret(t, ptrdiff_t, UINTN);
            default:
                *success = false;
        }

        return 0;
    }
    #undef case_fetch_ret

    static inline void format_number_back_buffer(
        IN INTN len,
        IN INTN minimum_field_size,
        IN BOOLEAN leading_zeroes,
        IN BOOLEAN left_justify,
        IN const CHAR16 *number_buffer,
        IN OUT size_t *_back_length,
        OUT CHAR16 *back_buffer
    ) {
        size_t back_length = *_back_length;

        for (INTN i = 0; i < len; ++i) {
            back_buffer[back_length++] = number_buffer[i];
        }

        if (static_cast<INT128>(back_length) < static_cast<INT128>(minimum_field_size)) {
            if (left_justify) {
                for (INTN i = (INTN)back_length; i < minimum_field_size; ++i) {
                    back_buffer[i] = u' ';
                }
            }
            else {
                CHAR16 prefix = leading_zeroes ? u'0' : u' ';

                for (size_t i = 0; i < back_length; ++i) {
                    back_buffer[i + minimum_field_size - back_length] = back_buffer[i];
                }
                for (size_t i = 0; i < minimum_field_size - back_length; ++i) {
                    back_buffer[i] = prefix;
                }
            }

            back_length = minimum_field_size;
        }

        *_back_length = back_length;
    }

    static inline void precision_fill(
        IN OUT INTN* _len,
        IN INTN precision,
        OUT CHAR16* number_buffer
    ) {
        INTN len = *_len;

        if (precision > len) {
            for (size_t i = len; i > 0; --i) {
                number_buffer[i - 1 + precision - len] = number_buffer[i - 1];
            }
            for (INTN i = 0; i < precision - len; ++i) {
                number_buffer[i] = u'0';
            }
            *_len = precision;
        }
    }

    static inline void flush_number_format_to_buffer(
        IN size_t bufsz,
        IN OUT size_t *_written,
        IN INTN len,
        IN INTN number,
        IN INTN minimum_field_size,
        IN INTN precision,
        IN BOOLEAN leading_zeroes,
        IN BOOLEAN left_justify,
        IN CHAR16 *number_buffer,
        IN size_t back_length,
        IN OUT CHAR16 *back_buffer,
        OUT CHAR16* buffer
    ) {
        size_t written = *_written;

        if (precision > 0 || number != 0) {
            format_number_back_buffer(
                len, minimum_field_size, leading_zeroes,
                left_justify, number_buffer, &back_length,
                back_buffer
            );
        }

        for (size_t i = 0; i < back_length && written < bufsz; ++i) {
            buffer[written++] = back_buffer[i];
        }

        EFI::sys->BootServices->FreePool(number_buffer);
        EFI::sys->BootServices->FreePool(back_buffer);

        *_written = written;
    }
}

size_t Loader::vsnprintf(
    OUT CHAR16* restrict buffer,
    IN size_t bufsz,
    IN const CHAR16* restrict format,
    IN va_list vlist
) {
    size_t written = 0;

    CHAR16 c;

    while (*format != u'\0' && written < bufsz) {
        c = *format++;

        if (c == u'%') {
            BOOLEAN left_justify = false,
                    force_sign = false,
                    prepend_space = false,
                    alternative_conv = false,
                    leading_zeroes = false;

            while (true) {
                c = *format++;
                
                if (c == u'%') {
                    goto kvsnprintf_write_as_is;
                }

                BOOLEAN exit_loop = false;

                switch (c) {
                    case u'-':
                        left_justify = true;
                        leading_zeroes = false;
                        break;
                    case u'+':
                        force_sign = true;
                        prepend_space = false;
                        break;
                    case u' ':
                        prepend_space = !force_sign;
                        break;
                    case u'#':
                        alternative_conv = true;
                        break;
                    case u'0':
                        leading_zeroes = !left_justify;
                        break;
                    default:
                        exit_loop = true;
                        --format;
                        break;
                }

                if (exit_loop) {
                    break;
                }
            }

            INTN minimum_field_size = 0;
            c = *format++;

            if (c == u'*') {
                minimum_field_size = va_arg(vlist, INTN);
            } else if (c == u'-' || (c >= u'0' && c <= u'9')) {
                bool neg = c == u'-';

                if (neg) {
                    c = *format++;
                }

                while (c >= u'0' && c <= u'9') {
                    minimum_field_size *= 10;
                    minimum_field_size += static_cast<INTN>((c - u'9') + 9);
                    c = *format++;
                }

                --format;

                if (neg) {
                    minimum_field_size = -minimum_field_size;
                }
            } else {
                --format;
            }

            INTN precision = -1;

            if (*format == '.') {
                ++format;
                c = *format++;

                if (c == '*') {
                    precision = va_arg(vlist, INTN);
                }
                else {
                    precision = 0;
                    bool neg = c == '-';

                    if (neg) {
                        c = *format++;
                    }

                    if (c >= u'0' && c <= u'9') { 
                        while (c >= u'0' && c <= u'9') {
                            precision *= 10;
                            precision += static_cast<INTN>((c - u'9') + 9);
                            c = *format++;
                        }

                        --format;

                        if (neg) {
                            precision = -precision;
                        }
                    } else {
                        precision = -1;
                    }
                }
            }

            kvsnprintf_length_modifier lmodif = none;
            c = *format++;

            switch (c) {
                case u'h':
                    if (*format++ == u'h') {
                        lmodif = hh;
                    }
                    else {
                        --format;
                        lmodif = h;
                    }
                    break;
                case u'l':
                    if (*format++ == u'l') {
                        lmodif = ll;
                    }
                    else {
                        --format;
                        lmodif = l;
                    }
                    break;
                case u'j':
                    ++format;
                    lmodif = j;
                    break;
                case u'z':
                    ++format;
                    lmodif = z;
                    break;
                case u't':
                    ++format;
                    lmodif = t;
                    break;
                case u'L':
                    ++format;
                    lmodif = L;
                    break;
                default:
                    --format;
                    lmodif = none;
                    break;
            }

            c = *format++;

            switch (c) {
                case u'c': {
                    CHAR16 back_buffer = u'\0';
                    switch (lmodif) {
                        case none:
                            back_buffer = static_cast<CHAR16>(va_arg(vlist, int));
                            break;
                        case l:
                            back_buffer = static_cast<CHAR16>(va_arg(vlist, int));
                            break;
                        default:
                            continue;
                    }

                    if (left_justify) {
                        buffer[written++] = back_buffer;
                        for (INTN i = 0; i < minimum_field_size - 1 && written < bufsz; ++i) {
                            buffer[written++] = u' ';
                        }
                    } else {
                        for (INTN i = 0; i < minimum_field_size - 1 && written < bufsz; ++i) {
                            buffer[written++] = u' ';
                        }
                        if (written < bufsz) {
                            buffer[written++] = back_buffer;
                        }
                    }

                    break;
                }
                case u's': {
                    INTN _s_len = 0;

                    switch (lmodif) {
                        case none: {
                            const char* _s = va_arg(vlist, const char*);

                            if (left_justify) {
                                while (*_s != '\0' && written < bufsz) {
                                    buffer[written++] = static_cast<CHAR16>(*_s++);
                                    ++_s_len;
                                }
                                for (INTN i = 0; i < minimum_field_size - _s_len && written < bufsz; ++i) {
                                    buffer[written++] = u' ';
                                }
                            } else {
                                const char* _s_tmp = _s;
                                for (; *_s_tmp != '\0'; ++_s_tmp);
                                _s_len = (_s_tmp - _s) / sizeof(char);

                                for (INTN i = 0; i < minimum_field_size - _s_len && written < bufsz; ++i) {
                                    buffer[written++] = u' ';
                                }
                                for (INTN i = 0; i < _s_len && written < bufsz; ++i) {
                                    buffer[written++] = *_s++;
                                }
                            }

                            break;
                        }
                        case l: {
                            const wchar_t* _ws = va_arg(vlist, const wchar_t*);

                            if (left_justify) {
                                while (*_ws != '\0' && written < bufsz) {
                                    buffer[written++] = static_cast<CHAR16>(*_ws++);
                                    ++_s_len;
                                }
                                for (INTN i = 0; i < minimum_field_size - _s_len && written < bufsz; ++i) {
                                    buffer[written++] = u' ';
                                }
                            } else {
                                const wchar_t* _ws_tmp = _ws;
                                for (; *_ws_tmp != '\0'; ++_ws_tmp);
                                _s_len = (_ws_tmp - _ws) / sizeof(char);

                                for (INTN i = 0; i < minimum_field_size - _s_len && written < bufsz; ++i) {
                                    buffer[written++] = u' ';
                                }
                                for (INTN i = 0; i < _s_len && written < bufsz; ++i) {
                                    buffer[written++] = *_ws++;
                                }
                            }
                            break;
                        }
                        default:
                            continue;
                    }
                    break;
                }
                case u'd':
                case u'i': {
                    if (precision < 0) {
                        precision = 1;
                    }

                    BOOLEAN fetch_success = true;

                    INTN number = fetch_signed_number(lmodif, &vlist, &fetch_success);

                    if (!fetch_success) {
                        precision = 0;
                    }

                    CHAR16 *number_buffer;
                    EFI::sys->BootServices->AllocatePool(
                        EfiLoaderData,
                        sizeof(CHAR16) * (precision > 32 ? precision : 32),
                        reinterpret_cast<VOID**>(&number_buffer)
                    );
                    INTN len = Loader::itoa(number, number_buffer, 10);

                    precision_fill(&len, precision, number_buffer);

                    CHAR16 *back_buffer;
                    EFI::sys->BootServices->AllocatePool(
                        EfiLoaderData,
                        sizeof(CHAR16) * ((precision > minimum_field_size ? precision : minimum_field_size) + 32),
                        reinterpret_cast<VOID**>(&back_buffer)
                    );
                    size_t back_length = 0;

                    if (force_sign && number >= 0) {
                        back_buffer[back_length++] = u'+';
                    }
                    else if (prepend_space && number >= 0) {
                        back_buffer[back_length++] = u' ';
                    }

                    flush_number_format_to_buffer(bufsz, &written, len, number,
                        minimum_field_size, precision, leading_zeroes, left_justify,
                        number_buffer, back_length, back_buffer, buffer
                    );

                    break;
                }
                case u'o': {
                    if (precision < 0) {
                        precision = 1;
                    }

                    BOOLEAN fetch_success = true;

                    UINTN number = fetch_unsigned_number(lmodif, &vlist, &fetch_success);

                    if (!fetch_success) {
                        precision = 0;
                    }

                    CHAR16 *number_buffer;
                    EFI::sys->BootServices->AllocatePool(
                        EfiLoaderData,
                        sizeof(CHAR16) * (precision > 32 ? precision : 32),
                        reinterpret_cast<VOID**>(&number_buffer)
                    );
                    INTN len = Loader::utoa(number, number_buffer, 8);

                    precision_fill(&len, precision, number_buffer);

                    CHAR16 *back_buffer;
                    EFI::sys->BootServices->AllocatePool(
                        EfiLoaderData,
                        sizeof(CHAR16) * ((precision > minimum_field_size ? precision : minimum_field_size) + 32),
                        reinterpret_cast<VOID**>(&back_buffer)
                    );
                    size_t back_length = 0;

                    if (alternative_conv) {
                        back_buffer[back_length++] = u'0';
                    }

                    flush_number_format_to_buffer(bufsz, &written, len, number,
                        minimum_field_size, precision, leading_zeroes, left_justify,
                        number_buffer, back_length, back_buffer, buffer
                    );

                    break;
                }
                case u'x':
                case u'X': {
                    if (precision < 0) {
                        precision = 1;
                    }

                    BOOLEAN fetch_success = true;

                    UINTN number = fetch_unsigned_number(lmodif, &vlist, &fetch_success);

                    if (!fetch_success) {
                        precision = 0;
                    }

                    CHAR16 *number_buffer;
                    EFI::sys->BootServices->AllocatePool(
                        EfiLoaderData,
                        sizeof(CHAR16) * (precision > 32 ? precision : 32),
                        reinterpret_cast<VOID**>(&number_buffer)
                    );
                    INTN len = Loader::utoa(number, number_buffer, 16);

                    if (c == u'X') {
                        for (size_t i = len; i > 0; --i) {
                            if (number_buffer[i - 1] >= u'a' && number_buffer[i - 1] <= u'f') {
                                number_buffer[i - 1] -= (u'a' - u'A');
                            }
                        }
                    }

                    precision_fill(&len, precision, number_buffer);

                    CHAR16 *back_buffer;
                    EFI::sys->BootServices->AllocatePool(
                        EfiLoaderData,
                        sizeof(CHAR16) * ((precision > minimum_field_size ? precision : minimum_field_size) + 32),
                        reinterpret_cast<VOID**>(&back_buffer)
                    );
                    size_t back_length = 0;

                    if (alternative_conv && number != 0) {
                        back_buffer[back_length++] = u'0';
                        back_buffer[back_length++] = u'x';
                    }

                    flush_number_format_to_buffer(bufsz, &written, len, number,
                        minimum_field_size, precision, leading_zeroes, left_justify,
                        number_buffer, back_length, back_buffer, buffer
                    );

                    break;
                }
                case u'u': {
                    if (precision < 0) {
                        precision = 1;
                    }

                    BOOLEAN fetch_success = true;

                    UINTN number = fetch_unsigned_number(lmodif, &vlist, &fetch_success);

                    if (!fetch_success) {
                        precision = 0;
                    }

                    CHAR16 *number_buffer;
                    EFI::sys->BootServices->AllocatePool(
                        EfiLoaderData,
                        sizeof(CHAR16) * (precision > 32 ? precision : 32),
                        reinterpret_cast<VOID**>(&number_buffer)
                    );
                    INTN len = Loader::utoa(number, number_buffer, 10);

                    precision_fill(&len, precision, number_buffer);

                    CHAR16 *back_buffer;
                    EFI::sys->BootServices->AllocatePool(
                        EfiLoaderData,
                        sizeof(CHAR16) * ((precision > minimum_field_size ? precision : minimum_field_size) + 32),
                        reinterpret_cast<VOID**>(&back_buffer)
                    );
                    size_t back_length = 0;

                    flush_number_format_to_buffer(bufsz, &written, len, number,
                        minimum_field_size, precision, leading_zeroes, left_justify,
                        number_buffer, back_length, back_buffer, buffer
                    );

                    break;
                }
            }
        } else {
kvsnprintf_write_as_is:
            buffer[written++] = c;
        }
    }

    if (written == bufsz) {
        buffer[bufsz - 1] = '\0';
    }
    else {
        buffer[written] = '\0';
    }

    return written;
}
