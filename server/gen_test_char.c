/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000-2003 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 *
 * Portions of this software are based upon public domain software
 * originally written at the National Center for Supercomputing Applications,
 * University of Illinois, Urbana-Champaign.
 */

#include "apr.h"
#include "apr_lib.h"

#if APR_HAVE_STDIO_H
#include <stdio.h>
#endif
#if APR_HAVE_STRING_H
#include <string.h>
#endif

/* A bunch of functions in util.c scan strings looking for certain characters.
 * To make that more efficient we encode a lookup table.
 */
#define T_ESCAPE_SHELL_CMD    (0x01)
#define T_ESCAPE_PATH_SEGMENT (0x02)
#define T_OS_ESCAPE_PATH      (0x04)
#define T_HTTP_TOKEN_STOP     (0x08)

int main(int argc, char *argv[])
{
    unsigned c;
    unsigned char flags;

    printf("/* this file is automatically generated by gen_test_char, "
           "do not edit */\n"
           "#define T_ESCAPE_SHELL_CMD     (%u)\n"
           "#define T_ESCAPE_PATH_SEGMENT  (%u)\n"
           "#define T_OS_ESCAPE_PATH       (%u)\n"
           "#define T_HTTP_TOKEN_STOP      (%u)\n"
           "\n"
           "static const unsigned char test_char_table[256] = {\n"
           "    0,",
           T_ESCAPE_SHELL_CMD,
           T_ESCAPE_PATH_SEGMENT,
           T_OS_ESCAPE_PATH,
           T_HTTP_TOKEN_STOP);

    /* we explicitly dealt with NUL above
     * in case some strchr() do bogosity with it */

    for (c = 1; c < 256; ++c) {
        flags = 0;
        if (c % 20 == 0)
            printf("\n    ");

        /* escape_shell_cmd */
#if defined(WIN32) || defined(OS2)
        /* Win32/OS2 have many of the same vulnerable characters
         * as Unix sh, plus the carriage return and percent char.
         * The proper escaping of these characters varies from unix
         * since Win32/OS2 use carets or doubled-double quotes, 
         * and neither lf nor cr can be escaped.  We escape unix 
         * specific as well, to assure that cross-compiled unix 
         * applications behave similiarly when invoked on win32/os2.
         *
         * Rem please keep in-sync with apr's list in win32/filesys.c
         */
        if (strchr("&;`'\"|*?~<>^()[]{}$\\\n\r%", c)) {
            flags |= T_ESCAPE_SHELL_CMD;
        }
#else
        if (strchr("&;`'\"|*?~<>^()[]{}$\\\n", c)) {
            flags |= T_ESCAPE_SHELL_CMD;
        }
#endif

        if (!apr_isalnum(c) && !strchr("$-_.+!*'(),:@&=~", c)) {
            flags |= T_ESCAPE_PATH_SEGMENT;
        }

        if (!apr_isalnum(c) && !strchr("$-_.+!*'(),:@&=/~", c)) {
            flags |= T_OS_ESCAPE_PATH;
        }

        /* these are the "tspecials" from RFC2068 */
        if (apr_iscntrl(c) || strchr(" \t()<>@,;:\\/[]?={}", c)) {
            flags |= T_HTTP_TOKEN_STOP;
        }

        printf("%u%c", flags, (c < 255) ? ',' : ' ');

    }

    printf("\n};\n");

    return 0;
}
