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
        exit(__LINE__);
    }
    if (line[len_read - 1] != delim) {
        exit(__LINE__);
    }
    line[len_read - 1] = '\0';

    cred->bv_len = len_read - 1;
    cred->bv_val = line;
}

int main (int argc, char **argv)
{
    (void) argc;
    (void) argv;

#if 0
    // Read from FD 3 per
    //     http://code.google.com/p/mod-auth-external/wiki/ConfigApache24
    //     #2.1._External_Password_Authenticators
    FILE *f = fdopen(3, "r");
    if (!f) {
        exit(4);
    }
#endif

    struct berval user, pass;
    readln(&user, '\n', stdin);
    readln(&pass, '\n', stdin);

    // ensure username contains lower case ASCII letters only
    for (size_t i = 0; i < user.bv_len; ++i) {
        if (!('a' <= user.bv_val[i] && user.bv_val[i] <= 'z') &&
            !('0' <= user.bv_val[i] && user.bv_val[i] <= '9'))  {
            exit(__LINE__);
        }
    }

    // open LDAP connection
    LDAP *ldap;
    int res = ldap_initialize(&ldap, SERVER);
    if (res != LDAP_SUCCESS) {
        exit(__LINE__);
    }

    // set protocol version
    int version = LDAP_VERSION3;
    res = ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, &version);
    if (res != LDAP_SUCCESS) {
        exit(__LINE__);
    }

    // concat group and username
    char *dn;
    int dn_len = snprintf(NULL, 0, "%s\\%.*s", CN,
                          (int) user.bv_len, user.bv_val);
    ++dn_len;
    dn = malloc(dn_len);
    snprintf(dn, dn_len, "%s\\%.*s", CN, (int) user.bv_len, user.bv_val);

    // connect to server (using ldaps, so no need for DIGEST-MD5)
    res = ldap_sasl_bind_s(ldap, dn, NULL, &pass, NULL, NULL, NULL);
    if (res != LDAP_SUCCESS) {
        exit(__LINE__);
    }

    int result = 0;

    // double check that we are connected
    struct berval *authzid;
    res = ldap_whoami_s(ldap, &authzid, NULL, NULL);
    if (res != LDAP_SUCCESS) {
        result = __LINE__;
    }

    // close connection
    res = ldap_unbind_ext_s(ldap, NULL, NULL);
    if (res != LDAP_SUCCESS) {
        exit(__LINE__);
    }

    exit(result);
}
