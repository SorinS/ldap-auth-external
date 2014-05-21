ldap-auth-external
==================

A small LDAP authenticator for [Apache HTTPD][mod-auth-external] and [ProFTPD][mod_external_auth].
It reads two lines from stdin: the username and the password.
The exit code tells whether the credentials are correct or not.

Differently from other LDAP authentication modules, this module does not require any credentials in the configuration.
It just tries to log in to the LDAP server, and run "whoami".

The group and server are static. Change them in the `.c` file.

  [mod-auth-external]: https://code.google.com/p/mod-auth-external/
  [mod_external_auth]: https://github.com/fub-vetbib/proftpd-mod_external_auth
