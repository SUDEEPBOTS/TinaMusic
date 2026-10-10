// Created by Laky64 on 23/04/2025.
// This header file provides compatibility for glibc >= 2.28
// for _dn_expand and __res_nquery functions.

#pragma once

#ifdef __GLIBC__

#if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 28)

#include <resolv.h>
#undef dn_expand

int __dn_expand(
    const unsigned char *msg,
    const unsigned char *eomorig,
    const unsigned char *comp_dn,
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
    int class,
    int type,
    unsigned char *answer,
    int anslen
) {
    return res_nquery(statp, dname, class, type, answer, anslen);
}

#endif
#endif