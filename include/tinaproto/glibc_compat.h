/*
 * ============================================================================
 *   Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
 *   Architecture: Native C++20 Core + Pure C++20 TinaProto Voice Engine
 *   Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 *   Support: https://t.me/Z0iiw
 *   License: GNU Affero General Public License v3.0
 * ============================================================================
 */

#pragma once

#ifdef __GLIBC__

#if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 28)

#include <resolv.h>
#undef dn_expand

#ifdef __cplusplus
extern "C" {
#endif

inline int __dn_expand(
    const unsigned char *msg,
    const unsigned char *eomorig,
    const unsigned char *comp_dn,
    char *exp_dn,
    int length
) {
    (void)eomorig;
    (void)comp_dn;
    int n = res_query((const char *)msg, C_IN, ns_t_ptr, (unsigned char *)exp_dn, length);
    if (n < 0) {
        return -1;
    }
    return n;
}

inline int __res_nquery(
    const res_state statp,
    const char *dname,
    int class_name,
    int type,
    unsigned char *answer,
    int anslen
) {
    return res_nquery(statp, dname, class_name, type, answer, anslen);
}

#ifdef __cplusplus
}
#endif

#endif
#endif
