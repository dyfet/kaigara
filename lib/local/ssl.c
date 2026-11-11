// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#ifdef HAVE_OPENSSL
#include "ssl.h"

SSL_CTX *ssl_server(ssl_certs_t *certs) {
    if (!certs) return NULL;
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) return NULL;
    if (certs->crtfile) SSL_CTX_use_certificate_file(ctx, certs->crtfile, SSL_FILETYPE_PEM);
    if (certs->keyfile) SSL_CTX_use_PrivateKey_file(ctx, certs->keyfile, SSL_FILETYPE_PEM);
    return ctx;

fail:
    SSL_CTX_free(ctx);
    return NULL;
}

SSL_CTX *ssl_client(ssl_certs_t *certs) {
    if (!certs) return NULL;
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) return NULL;
    if (certs->capath)
        SSL_CTX_load_verify_locations(ctx, certs->capath, NULL);
    else
        SSL_CTX_set_default_verify_paths(ctx);

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    return ctx;

fail:
    SSL_CTX_free(ctx);
    return NULL;
}

void ssl_stop(const SSL *s) {
    if (!s) return;

    const int so = SSL_get_fd(s);
    if (so >= 0) cpr_sockclose(so);
}

void ssl_close(SSL *s) {
    if (!s) return;
    SSL_shutdown(s);
    const int so = SSL_get_fd(s);
    if (so >= 0) cpr_sockdown(so);
    SSL_free(s);
}

SSL *ssl_promote(int so, SSL_CTX *ctx) {
    SSL *ssl = SSL_new(ctx);
    if (!ssl) return NULL;
    SSL_set_fd(ssl, so);
    if (SSL_accept(ssl) <= 0) {
        SSL_free(ssl);
        return NULL;
    }
    return ssl;
}

size_t ssl_readall(SSL *ssl, void *data, size_t len) {
    if (!ssl) return 0;

    char *p = (char *)data;
    size_t received = 0;
    while (received < len) {
        const ssize_t n = SSL_read(ssl, p + received, len - received);
        if (n <= 0) break;
        received += (size_t)n;
    }
    return received;
}

size_t ssl_writeall(SSL *ssl, const void *data, size_t len) {
    if (!ssl) return 0;

    const char *p = (const char *)data;
    size_t written = 0;
    while (written < len) {
        const ssize_t n = SSL_write(ssl, p + written, len - written);
        if (n <= 0) break;
        written += (size_t)n;
    }
    return written;
}

X509 *ssl_peercert(SSL *ssl) {
    if (!ssl) return NULL;
    X509 *cert = SSL_get_peer_certificate(ssl);
    if (cert) X509_up_ref(cert);
    return cert;
}

bool ssl_certname(X509 *cert, char *buf, size_t size) {
    if (!size || !buf || !cert) return false;
    return X509_NAME_get_text_by_NID(X509_get_subject_name(cert), NID_commonName, buf, sizeof(buf)) > 0;
}

ssl_verify_t ssl_verify(SSL *ssl, X509 *cert) {
    if (!ssl) return PEER_INSECURE;
    if (!cert) return PEER_UNSIGNED;

    const int rc = SSL_get_verify_result(ssl);
    switch (rc) {
#ifdef X509_V_ERR_HOSTNAME_MISMATCH
    case X509_V_ERR_HOSTNAME_MISMATCH:
        return PEER_MISMATCH;
#endif
    case X509_V_OK:
        return PEER_VERIFIED;
    case X509_V_ERR_CERT_HAS_EXPIRED:
        return PEER_EXPIRED;
    case X509_V_ERR_CERT_NOT_YET_VALID:
        return PEER_NOT_YET_VALID;
    case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
    case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
        return PEER_SELF_SIGNED;
    default:
        return PEER_SIGNED;
    }
}

SSL *ssl_connect(int so, SSL_CTX *ctx, const char *host) {
    if (so < 0 || !ctx || !host) return NULL;
    SSL *ssl = SSL_new(ctx);
    if (!ssl) return NULL;
    SSL_set_fd(ssl, so);
    if (host && *host) {
        SSL_set_tlsext_host_name(ssl, host);
        SSL_set1_host(ssl, host);
    }
    if (SSL_connect(ssl) <= 0) {
        SSL_free(ssl);
        return NULL;
    }
    return ssl;
}
#endif
