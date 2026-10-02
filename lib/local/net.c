// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 202y David Sugar <tychosoft@gmail.com>

#include "net.h"
#include <cpr/memory.h>
#include <cpr/service.h>
#include <errno.h>
#include <stdarg.h>

#if __has_include(<sys/sendfile.h>)
#include <sys/sendfile.h>
#endif

#if defined(__linux__)
#include <linux/tcp.h>
#elif __has_include(<netinet/tcp.h>)
#include <netinet/tcp.h>
#endif

static int net_used = 0;
static int net_first = -1;

static int listener(void *server) {
    struct sockaddr_storage accepted;
    char *from = NULL;
    const net_accept_t *serve = (net_accept_t *)server;
    cpr_logger(serve->log, CPR_INFO, "%s thread startup", serve->tag);
    for (;;) {
        if (serve->sem && !cpr_semaphore_acquire(serve->sem)) break;
        for (;;) {
            thrd_t client;
            socklen_t len = sizeof(accepted);
            int so = accept(serve->so, (struct sockaddr *)&accepted, &len);
            if (so < 0) break;
            if (so > net_used) net_used = so;
            net_strmake(&from, &accepted);

            // TODO: Any source tests...

            cpr_logger(serve->log, CPR_DEBUG, "connect %s", from);
            net_session_t *session = (net_session_t *)malloc(sizeof(net_session_t));
            session->so = so;
            session->sem = serve->sem;
            session->origin = from;
            if (thrd_create(&client, serve->start, session) != thrd_success) {
                cpr_logger(serve->log, CPR_ERROR, "failed %s", from);
                cpr_sockclose(so);
                continue;
            }
            thrd_detach(client);
            from = NULL;
            break;
        }
    }
    cpr_logger(serve->log, CPR_INFO, "%s thread shutdown", serve->tag);
    cpr_sockclose(serve->so);
    return 0;
}

bool net_iface(int family, const char *iface, uint16_t port, sockaddr_t *store) {
    cpr_memset(store, 0, sizeof(struct sockaddr_storage));
    store->ss_family = family;
    cpr_addport(store, port);
    if (iface && !cpr_getbind(iface, family, store, port)) return false;
    return true;
}

int net_listen(const struct sockaddr *addr, int backlog) {
    int so = socket(addr->sa_family, SOCK_STREAM, 0);
    if (so < 0) {
        errno = cpr_sockerror();
        return -1;
    }

    if (bind(so, addr, cpr_socklen(addr))) {
        int err = cpr_sockerror();
        cpr_sockclose(so);
        errno = err;
        return -1;
    }

    if (listen(so, backlog)) {
        int err = cpr_sockerror();
        cpr_sockclose(so);
        errno = err;
        return -1;
    }
    if (net_first == -1) net_first = so;
    if (net_used < so) net_used = so;
    return so;
}

bool net_accept(thrd_t *thread, net_accept_t *server) {
    return thrd_create(thread, &listener, server) == thrd_success;
}

bool net_strmake(char **str, struct sockaddr_storage *store) {
    if (!str) return false;
    net_strfree(str);
    char buf[INET6_ADDRSTRLEN + 16];
    char *from = cpr_getaddr(store);
    uint16_t port = cpr_getport(store);
    if (store->ss_family == AF_INET6)
        snprintf(buf, sizeof(buf), "[%s]:%d", from, port);
    else
        snprintf(buf, sizeof(buf), "%s:%d", from, port);
    free(from);
    *str = cpr_strdup(buf, sizeof(buf));
    return true;
}

void net_strfree(char **str) {
    cpr_strfree(str);
}

void net_shutdown() {
    int so = net_first;
    if (so < 0) return;

    while (so <= net_used) {
        cpr_sockdown(so);
        ++so;
    }

    thrd_yield();
    so = net_first;
    while (so <= net_used) {
        cpr_sockclose(so);
        ++so;
    }
}

int net_sendtext(int so, const char *text, size_t size) {
    if (text == NULL) return EINVAL;
    size = cpr_strlen(text, size);
    if (!size) return EINVAL;
    ssize_t len = send(so, text, size, 0);
    if (len < 0) return cpr_sockerror();
    if (len < size) return ECONNRESET;
    return 0;
}

int net_wait(int so, int timeout) {
    struct pollfd pfd;
    pfd.fd = so;
    pfd.revents = 0;
    pfd.events = POLLIN;
    int status = cpr_sockpoll(&pfd, 1, timeout);
    if (status < 0) return cpr_sockerror();
    if (!status) return ETIMEDOUT;
    if (pfd.revents & POLLHUP) return ECONNRESET; // NOLINT
    if (pfd.revents & POLLIN) return 0;           // NOLINT
    return EPROTO;
}

int net_recvline(int so, char *buf, size_t size, int timeout) {
    size_t pos = 0;
    char *nl = NULL;
    buf[0] = 0;
    --size; // terminating null
    while (!nl && pos < size) {
        int err = net_wait(so, timeout);
        if (err) return err;
        ssize_t len = recv(so, buf + pos, size - pos, 0);
        if (len < 0) return cpr_sockerror();
        if (len == 0) return ECONNRESET;
        buf[pos + len] = 0;
        nl = strchr(buf, '\n');
        pos += (size_t)len;
    }
    if (nl) {
        *(nl--) = 0;
        if (nl >= buf && *nl == '\r') *nl = 0;
        return 0;
    }
    return EOVERFLOW;
}

__attribute__((format(printf, 3, 4))) int net_printf(int so, size_t size, const char *fmt, ...) {
    if (!fmt) return EINVAL;
    char buf[size];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (len < 0) return errno;
    if (!len) return 0;
    if (len >= size) return EOVERFLOW;
    return net_sendtext(so, buf, size);
}

int net_session(int so, bool live) {
    int enable = live ? 1 : 0;
    int mss = 0;
    setsockopt(so, SOL_SOCKET, SO_KEEPALIVE, (char *)&enable, sizeof(enable));
#if defined(__linux__)
    struct tcp_info info;
    socklen_t len = sizeof(info);
    if (getsockopt(so, IPPROTO_TCP, TCP_INFO, &info, &len) == 0)
        mss = (int)info.tcpi_snd_mss;
    else
        mss = 1220;
#elif defined(__APPLE__)
    // macOS
    struct tcp_connection_info info;
    socklen_t len = sizeof(info);
    if (getsockopt(so, IPPROTO_TCP, TCP_CONNECTION_INFO, &info, &len) == 0)
        mss = info.tcpi_snd_mss;
    else
        mss = 1220;

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    socklen_t len = sizeof(mss);
    if (getsockopt(so, IPPROTO_TCP, TCP_MAXSEG, &mss, &len) != 0) mss = -1;
#elif defined(_WIN32)
    mss = 1220;
#else
    mss = 1220;
#endif
    return mss;
}

size_t net_sendfd(int so, int fd, size_t count, size_t chunk) {
    if (so < 0 || fd < 0) return 0;
    size_t total = 0;
#if defined(__linux__)
    (void)chunk;
    while (total < count) {
        size_t remaining = count - total;
        ssize_t sent = sendfile(so, fd, NULL, remaining);
        if (sent <= 0) {
            if (sent < 0 && (errno == EINTR || errno == EAGAIN)) continue;
            break;
        }
        total += (size_t)sent;
    }
#elif __has_include(<sys/sendfile.h>)
    (void)chunk;
    off_t offset = lseek(fd, 0, SEEK_CUR);
    if (offset < 0) return total;
    while (total < count) {
        size_t remaining = count - total;
        off_t sbytes = 0;
        int ret = sendfile(fd, so, offset, remaining, NULL, &sbytes, 0);
        if (ret < 0) {
            if (errno == EINTR || errno == EAGAIN) continue;
            break;
        }

        if (sbytes <= 0) break;
        offset += sbytes;
        total += (size_t)sbytes;
        if ((size_t)sbytes < remaining) break;
    }
    lseek(fd, offset, SEEK_SET);
#else
    char buf[chunk];
    while (total < count) {
        size_t remaining = count - total;
        if (remaining > chunk) remaining = chunk;
        ssize_t from = read(fd, buf, remaining);
        if (from <= 0) {
            if (from < 0 && errno == EINTR) continue;
            break;
        }

        size_t written = 0;
        while (written < (size_t)from) {
            ssize_t to = send(so, buf + written, (size_t)from - written, 0);
            if (to <= 0) {
                if (to < 0 && (errno == EINTR || errno == EAGAIN)) continue;
                return total;
            }
            written += (size_t)to;
        }
        total += (size_t)from;
    }
#endif
    return total;
}

size_t net_recvfd(int so, int fd, size_t count, size_t chunk) {
    if (so < 0 || fd < 0) return 0;
    char buf[chunk];
    size_t total = 0;
    while (total < count) {
        size_t remaining = count - total;
        if (remaining > chunk) remaining = chunk;
        ssize_t from = recv(so, buf, remaining, 0);
        if (from <= 0) {
            if (from < 0 && (errno == EINTR || errno == EAGAIN)) continue;
            break;
        }

        size_t written = 0;
        while (written < (size_t)from) {
            ssize_t to = write(fd, buf + written, (size_t)from - written);
            if (to <= 0) {
                if (to < 0 && (errno == EINTR || errno == EAGAIN)) continue;
                return total;
            }
            written += (size_t)to;
        }
        total += (size_t)from;
    }
    return total;
}

int net_proxy(const char *host, uint16_t port, const char *ident, const struct sockaddr *addr) {
    if (!addr || !host || ((addr->sa_family != AF_INET) && (addr->sa_family != AF_INET6))) return -1;
    if (!ident) ident = "";

    int so = -1;
    struct sockaddr_storage proxy = {0};
    socklen_t len;
    cpr_memset(&proxy, 0, sizeof(proxy));
    if (strchr(host, ':')) {
        struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)&proxy;
        sa6->sin6_family = AF_INET6;
        sa6->sin6_port = htons(port);
        if (inet_pton(AF_INET6, host, &sa6->sin6_addr) != 1) return -1;
        len = sizeof(struct sockaddr_in6);
        so = socket(AF_INET6, SOCK_STREAM, 0);
    } else {
        if (!cpr_gethost(host, NULL, AF_INET, SOCK_STREAM, &proxy)) return -1;
        cpr_addport(&proxy, port);
        len = sizeof(struct sockaddr_in);
        so = socket(AF_INET, SOCK_STREAM, 0);
    }
    if (so < 0) return so;
    if (connect(so, (struct sockaddr *)&proxy, len) < 0) {
        cpr_sockclose(so);
        return -1;
    }
    uint8_t greet[3] = {0x05, 0x01, 0x02};
    uint8_t reply[4];
    if (send(so, greet, sizeof(greet), MSG_WAITALL) < 3) goto fail;
    if (recv(so, reply, 2, 0) < 2) goto fail;
    if (reply[0] != 0x05 || reply[1] != 0x02) goto fail;

    size_t ulen = cpr_strlen(ident, 256);
    const char *password = "";
    size_t plen = 0;
    if (ulen > 255 || plen > 255) goto fail;

    uint8_t auth[1 + 1 + 255 + 1 + 255];
    size_t p = 0;
    auth[p++] = 0x01;
    auth[p++] = (uint8_t)ulen;
    if (ulen) cpr_memcpy(&auth[p], sizeof(auth) - p, ident, ulen);
    p += ulen;
    auth[p++] = (uint8_t)plen;
    if (plen) cpr_memcpy(&auth[p], sizeof(auth) - p, password, plen);
    p += plen;
    if (send(so, auth, p, MSG_WAITALL) < p) goto fail;
    if (recv(so, reply, 2, 0) < 2) goto fail;
    if (reply[0] != 0x01 || reply[1] != 0x00) goto fail;

    uint8_t req[4 + 16 + 2];
    p = 0;
    req[p++] = 0x05;
    req[p++] = 0x01;
    req[p++] = 0x00;
    if (addr->sa_family == AF_INET) {
        const struct sockaddr_in *in = (const struct sockaddr_in *)addr;
        req[p++] = 0x01;
        cpr_memcpy(&req[p], 4, &in->sin_addr, 4);
        p += 4;
        cpr_memcpy(&req[p], 2, &in->sin_port, 2);
        p += 2;
    } else {
        const struct sockaddr_in6 *in6 = (const struct sockaddr_in6 *)addr;
        req[p++] = 0x04;
        cpr_memcpy(&req[p], 16, &in6->sin6_addr, 16);
        p += 16;
        cpr_memcpy(&req[p], 2, &in6->sin6_port, 2);
        p += 2;
    }
    if (send(so, req, p, MSG_WAITALL) < p) goto fail;
    if (recv(so, reply, 4, 0) < 4) goto fail;

    uint8_t atype = reply[3];
    size_t alen =
    (atype == 0x01 ? 4 : atype == 0x04   ? 16
                         : atype == 0x03 ? 1
                                         : 0);
    if (alen == 0) goto fail;

    uint8_t addrbuf[256];
    if (alen > sizeof(addrbuf)) goto fail;
    if (recv(so, addrbuf, alen, 0) < alen) goto fail;

    uint8_t portbuf[2];
    if (recv(so, portbuf, 2, 0) < 2) goto fail;
    return so;

fail:
    cpr_sockclose(so);
    return -1;
}

bool net_host(int family, int type, const char *host, uint16_t port, sockaddr_t *store) {
    if (!cpr_gethost(host, NULL, family, type, store)) return false;
    cpr_addport(store, port);
    return true;
}

bool net_service(int family, int type, const char *host, const char *service, uint16_t port, sockaddr_t *store) {
    if (!cpr_gethost(host, service, family, type, store))
        return net_host(family, type, host, port, store);
    return true;
}

int net_connect(const struct sockaddr *addr) {
    int so = socket(addr->sa_family, SOCK_STREAM, 0);
    if (so < 0) {
        errno = cpr_sockerror();
        return -1;
    }
    if (connect(so, addr, cpr_socklen(addr))) {
        int err = cpr_sockerror();
        cpr_sockclose(so);
        errno = err;
        return -1;
    }
    return so;
}

int net_readline(int so, char *buf, size_t size, int timeout) {
    size_t pos = 0;
    char *nl = NULL;
    buf[0] = 0;
    --size; // terminating null
    while (!nl && pos < size) {
        int err = net_wait(so, timeout);
        if (err) return err;
        ssize_t len = recv(so, buf + pos, size - pos, MSG_PEEK);
        if (len < 0) return cpr_sockerror();
        if (len == 0) return ECONNRESET;
        buf[pos + len] = 0;
        nl = strchr(buf, '\n');
        size_t max = size;
        if (nl) max = nl - buf + 1;
        ;
        len = recv(so, buf + pos, max - pos, 0);
        if (len < 0) return cpr_sockerror();
        if (len == 0) return ECONNRESET;
        pos += (size_t)len;
    }
    if (nl) {
        *(nl--) = 0;
        if (nl >= buf && *nl == '\r') *nl = 0;
        return 0;
    }
    return EOVERFLOW;
}

bool net_getiface(const sockaddr_t *addr, char *buf, size_t size) {
    if (!addr) return false;
    if (addr->ss_family != AF_INET && addr->ss_family != AF_INET6) return false;
    struct sockaddr_in *in = (struct sockaddr_in *)addr;
    struct sockaddr_in6 *in6 = (struct sockaddr_in6 *)addr;
#ifdef _WIN32
    ULONG bufsize = 8192;
    PIP_ADAPTER_ADDRESSES list = (PIP_ADAPTER_ADDRESSES)(malloc(bufsize));
    if (!list) return false;
    WORD result = GetAdaptersAddresses(addr->ss_family, GAA_FLAG_INCLUDE_PREFIX, NULL, list, &bufsize);
    if (result == ERROR_BUFFER_OVERFLOW) {
        free(list);
        list = (PIP_ADAPTER_ADDRESSES)(malloc(bufsize));
        result = GetAdaptersAddresses(addr->ss_family, GAA_FLAG_INCLUDE_PREFIX, NULL, list, &bufsize);
    };

    char utf8[512];
    const char *name = NULL;
    for (IP_ADAPTER_ADDRESSES *entry = list; entry != NULL; entry = entry->Next) {
        if (name) break;
        if (!entry->FriendlyName) continue;
        if (!entry->FriendlyName) continue;
        WideCharToMultiByte(CP_UTF8, 0, entry->FriendlyName, -1, utf8, sizeof utf8, NULL, NULL);

        for (IP_ADAPTER_UNICAST_ADDRESS *unicast = entry->FirstUnicastAddress; unicast != NULL; unicast = unicast->Next) {
            if (name) break;
            const struct sockaddr *sa = unicast->Address.lpSockaddr;
            if (!sa) continue;
            if (addr->ss_family == AF_INET) {
                const struct sockaddr_in *t = (const struct sockaddr_in *)addr;
                const struct sockaddr_in *a = (const struct sockaddr_in *)sa;
                const int prefix = unicast->OnLinkPrefixLength;
                const uint32_t mask = prefix ? htonl(~((1u << (32 - prefix)) - 1)) : 0;
                if ((a->sin_addr.s_addr & mask) == (t->sin_addr.s_addr & mask)) name = utf8;
            } else {
                const struct sockaddr_in6 *t6 = (const struct sockaddr_in6 *)addr;
                const struct sockaddr_in6 *a6 = (const struct sockaddr_in6 *)sa;
                const int prefix = unicast->OnLinkPrefixLength;
                int bits = (int)prefix;
                ;
                for (int i = 0; i < 16; ++i) {
                    const uint8_t mask = bits >= 8 ? 0xFF : (bits > 0 ? ~(0xFF >> bits) : 0);
                    bits -= 8;
                    if ((a6->sin6_addr.s6_addr[i] & mask) != (t6->sin6_addr.s6_addr[i] & mask)) {
                        name = utf8;
                        break;
                    }
                }
            }
        }
    }

    if (buf && name && !cpr_strcpy(buf, name, size)) name = NULL;
    free(list);
    return name != NULL;
#else
    iface_t list = NULL, node = NULL;
    if (getifaddrs(&list)) return false;
    node = list;
    while (node) {
        if (!node->ifa_addr) goto skip;
        if (node->ifa_addr->sa_family != addr->ss_family) goto skip;
        if (addr->ss_family == AF_INET) {
            const struct sockaddr_in *t = (const struct sockaddr_in *)addr;
            const struct sockaddr_in *a = (const struct sockaddr_in *)node->ifa_addr;
            const struct sockaddr_in *m = (const struct sockaddr_in *)node->ifa_netmask;
            if ((a->sin_addr.s_addr & m->sin_addr.s_addr) == (t->sin_addr.s_addr & m->sin_addr.s_addr)) break;
        } else {
            const struct sockaddr_in6 *t6 = (const struct sockaddr_in6 *)addr;
            const struct sockaddr_in6 *a6 = (const struct sockaddr_in6 *)node->ifa_addr;
            const struct sockaddr_in6 *m6 = (const struct sockaddr_in6 *)node->ifa_netmask;
            bool match = true;
            for (int i = 0; i < 16; ++i) {
                if ((a6->sin6_addr.s6_addr[i] & m6->sin6_addr.s6_addr[i]) != (t6->sin6_addr.s6_addr[i] & m6->sin6_addr.s6_addr[i])) {
                    match = false;
                    break;
                }
            }
            if (match) break;
        }
    skip:
        node = node->ifa_next;
    }
    if (buf && node && !cpr_strcpy(buf, node->ifa_name, size)) node = NULL;
    if (list) freeifaddrs(list);
    return node != NULL;
#endif
}
