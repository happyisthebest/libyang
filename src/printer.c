/**
 * @file printer.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Wrapper for all libyang printers.
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#define _GNU_SOURCE /* vdprintf() */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "tree_schema.h"
#include "tree_data.h"
#include "printer.h"

int
ly_print(struct lyout *out, const char *format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);

    switch(out->type) {
    case LYOUT_FD:
        count = vdprintf(out->method.fd, format, ap);
        break;
    case LYOUT_STREAM:
        count = vfprintf(out->method.f, format, ap);
        break;
    }

    va_end(ap);
    return count;
}

int
ly_write(struct lyout *out, const char *buf, size_t count)
{
    switch(out->type) {
    case LYOUT_FD:
        return write(out->method.fd, buf, count);
    case LYOUT_STREAM:
        return fwrite(buf, sizeof *buf, count, out->method.f);
    }

    return 0;
}

API int
lys_print(FILE *f, struct lys_module *module, LYS_OUTFORMAT format, const char *target_node)
{
    struct lyout out;

    if (!f || !module) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_STREAM;
    out.method.f = f;

    switch (format) {
    case LYS_OUT_YIN:
        LOGERR(LY_EINVAL, "YIN output format not supported yet.");
        return EXIT_FAILURE;
    case LYS_OUT_YANG:
        return yang_print_model(&out, module);
    case LYS_OUT_TREE:
        return tree_print_model(&out, module);
    case LYS_OUT_INFO:
        return info_print_model(&out, module, target_node);
    default:
        LOGERR(LY_EINVAL, "Unknown output format.");
        return EXIT_FAILURE;
    }
}

API int
lyd_print(FILE * f, struct lyd_node *root, LYD_FORMAT format)
{
    struct lyout out;

    if (!f || !root) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_STREAM;
    out.method.f = f;

    switch (format) {
    case LYD_XML:
        return xml_print_data(&out, root);
    case LYD_JSON:
        return json_print_data(&out, root);
    default:
        LOGERR(LY_EINVAL, "Unknown output format.");
        return EXIT_FAILURE;
    }
}
