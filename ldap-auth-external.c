/**
 * Copyright (c) 2014 Veterinärmedizinische Bibliothek - Fachbereich Veterinärmedizin der Freien Universität Berlin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **
 * Author: René Kijewski <kijewski@library.vetmed.fu-berlin.de
 */

#include <ldap.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static const char CN[] = "FU-BERLIN";
static const char SERVER[] = "ldaps://windc1.fu-berlin.de:3269";

static void readln (struct berval *cred, char delim, FILE *f)
{
    char *line = NULL;
    size_t len_allocated = 0;
    ssize_t len_read = getdelim(&line, &len_allocated, delim, f);
    if (len_read == -1) {
        exit(1);
    }
    if (line[len_read - 1] != delim) {
        exit(1);
    }
    line[len_read - 1] = '\0';

    cred->bv_len = len_read - 1;
    cred->bv_val = line;
}

int main (int argc, char **argv)
{
    (void) argc;
    (void) argv;

    struct berval user, pass;
    readln(&user, '\n', stdin);
    readln(&pass, '\n', stdin);

    if (user.bv_len == 0) {
        exit(1);
    }
    if (pass.bv_len == 0) {
        exit(1);
    }

    // ensure username contains lower case ASCII letters only
    for (size_t i = 0; i < user.bv_len; ++i) {
        if (!('a' <= user.bv_val[i] && user.bv_val[i] <= 'z') &&
            !('0' <= user.bv_val[i] && user.bv_val[i] <= '9'))  {
            exit(1);
        }
    }

    // open LDAP connection
    LDAP *ldap;
    int res = ldap_initialize(&ldap, SERVER);
    if (res != LDAP_SUCCESS) {
        exit(1);
    }

    // set protocol version
    int version = LDAP_VERSION3;
    res = ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, &version);
    if (res != LDAP_SUCCESS) {
        exit(1);
    }

    // concat group and username
    char *dn;
    int dn_len = snprintf(NULL, 0, "%s\\%.*s", CN,
                          (int) user.bv_len, user.bv_val);
    ++dn_len;
    dn = malloc(dn_len); // no need to free()
    if (!dn) {
        exit(1);
    }
    snprintf(dn, dn_len, "%s\\%.*s", CN, (int) user.bv_len, user.bv_val);

    // connect to server (using ldaps, so no need for DIGEST-MD5)
    res = ldap_sasl_bind_s(ldap, dn, NULL, &pass, NULL, NULL, NULL);
    if (res != LDAP_SUCCESS) {
        exit(1);
    }

    int result = 0;

    // double check that we are connected
    struct berval *authzid;
    res = ldap_whoami_s(ldap, &authzid, NULL, NULL);
    if (res != LDAP_SUCCESS) {
        result = 1;
    }

    // close connection
    res = ldap_unbind_ext_s(ldap, NULL, NULL);
    if (res != LDAP_SUCCESS) {
        exit(1);
    }

    exit(result);
}
