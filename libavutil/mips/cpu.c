/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/cpu.h"
#include "libavutil/cpu_internal.h"
#include "config.h"
#if defined __linux__ || defined __ANDROID__
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/auxv.h>
#include "asmdefs.h"
#include "libavutil/avstring.h"
#endif

#if defined __linux__ || defined __ANDROID__

static int cpu_flags_cpuinfo(void)
{
    FILE *f = fopen("/proc/cpuinfo", "r");
    char buf[200];
    int flags = 0;

    if (!f)
        return -1;

    while (fgets(buf, sizeof(buf), f)) {
        /* Legacy kernel may not export MMI in ASEs implemented */
        if (av_strstart(buf, "cpu model", NULL)) {
            if (strstr(buf, "Loongson-3 "))
                flags |= AV_CPU_FLAG_MMI;
        }

        if (av_strstart(buf, "ASEs implemented", NULL)) {
            if (strstr(buf, " loongson-mmi"))
                flags |= AV_CPU_FLAG_MMI;
            if (strstr(buf, " msa"))
                flags |= AV_CPU_FLAG_MSA;

            break;
        }
    }
    fclose(f);
    return flags;
}
#endif

int ff_get_cpu_flags_mips(void)
{
#if defined __linux__ || defined __ANDROID__
    return cpu_flags_cpuinfo();
#else
    /* Assume no SIMD ASE supported */
    return 0;
#endif
}

size_t ff_get_cpu_max_align_mips(void)
{
    int flags = av_get_cpu_flags();

    if (flags & AV_CPU_FLAG_MSA)
        return 16;

    /*
     * MMI itself is 64-bit but quad word load & store
     * needs 128-bit align.
     */
    if (flags & AV_CPU_FLAG_MMI)
        return 16;

    return 8;
}
