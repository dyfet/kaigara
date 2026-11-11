// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#ifndef LOCAL_SSL_H
#define LOCAL_SSL_H

#include <defines.h>

#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <cpr/thread.h>
#include <cpr/socket.h>

typedef enum {
    PEER_UNVERIFIED,
    PEER_INSECURE,
    PEER_UNSIGNED,
    PEER_SELF_SIGNED,
    PEER_EXPIRED,
    PEER_MISMATCH,
    PEER_NOT_YET_VALID,
    PEER_SIGNED,
    PEER_VERIFIED
} ssl_verify_t;

typedef struct {
    const char *capath;
    const char *keyfile;
    const char *crtfile;
} ssl_certs_t;

SSL_CTX *ssl_server(ssl_certs_t *certs);
SSL_CTX *ssl_client(ssl_certs_t *certs);
SSL *ssl_promote(int so, SSL_CTX *ctx);
SSL *ssl_connect(int so, SSL_CTX *ctx, const char *host);
ssl_verify_t ssl_verify(SSL *ssl, X509 *peer);
X509 *ssl_peercert(SSL *ssl);
void ssl_close(SSL *ssl);
void ssl_stop(const SSL *ssl);
bool ssl_certname(X509 *cert, char *buf, size_t size);
size_t ssl_readall(SSL *ssl, void *data, size_t len);
size_t ssl_writeall(SSL *ssl, const void *data, size_t lem);

#endif
#endif
