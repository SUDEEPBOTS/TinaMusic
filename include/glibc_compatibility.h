#pragma once

#ifdef __GLIBC__
#if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 28)

#include <resolv.h>
#undef dn_expand

extern "C" {

int __dn_expand(
    const unsigned char *msg,
    const unsigned char *,
    const unsigned char *,
    char *exp_dn,
    int length
) {
    int n = res_query((char *)msg, C_IN, T_PTR, (unsigned char *)exp_dn, length);
    if (n < 0) {
        return -1;
    }
    return n;
}

int __res_nquery(
    const res_state statp,
    const char *dname,
    int class_name,
    int type,
    unsigned char *answer,
    int anslen
) {
    return res_nquery(statp, dname, class_name, type, answer, anslen);
}

}

#endif
#endif
