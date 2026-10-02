// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

#include <stdlib.h>
#include "buffer.h"

static pthread_mutex_t buf_lock = PTHREAD_MUTEX_INITIALIZER;
static struct cpr_buffer *buf_list = NULL;

static void buf_free(struct cpr_buffer *entry) {
    if (!entry) return;
    pthread_mutex_lock(&buf_lock);
    entry->next = buf_list;
    buf_list = entry;
    pthread_mutex_unlock(&buf_lock);
}

static struct cpr_buffer *buf_make(void *item) {
    pthread_mutex_lock(&buf_lock);
    struct cpr_buffer *entry;
    if (buf_list) {
        entry = buf_list;
        buf_list = entry->next;
    } else {
        entry = malloc(sizeof(struct cpr_buffer));
    }
    pthread_mutex_unlock(&buf_lock);
    if (!entry) return NULL;
    entry->item = item;
    entry->next = NULL;
    return entry;
}

static void buf_clear(buffer_t *buf) {
    struct cpr_buffer *entry = buf->head;
    if (entry) {
        if (entry->item) buf->free(entry->item);
        buf->head = entry->next;
        if (!buf->head) buf->tail = NULL;
        buf_free(entry);
        --buf->count;
    }
}

static void buf_purge(buffer_t *buf) {
    while (buf->head)
        buf_clear(buf);
    buf->head = buf->tail = NULL;
    buf->count = 0;
}

void purge_buffers() {
    pthread_mutex_lock(&buf_lock);
    while (buf_list) {
        struct cpr_buffer *entry = buf_list;
        buf_list = entry->next;
        free(entry);
    }
    pthread_mutex_unlock(&buf_lock);
}

void close_buffer(buffer_t *buf) {
    if (!buf) return;
    mtx_lock(&buf->lock);
    if (!buf->closed) {
        cnd_broadcast(&buf->cond);
        thrd_yield();
        buf_purge(buf);
        buf->closed = true;
    }
    mtx_unlock(&buf->lock);
}

bool init_buffer(buffer_t *buf, size_t size, cpr_free_t ff) {
    if (!buf) return false;
    cpr_memset(buf, 0, sizeof(buffer_t));
    if (ff == NULL) ff = &free;
    mtx_init(&buf->lock, mtx_plain);
    cnd_init(&buf->cond);
    buf->head = buf->tail = NULL;
    buf->count = 0;
    buf->limit = size;
    buf->free = ff;
    buf->closed = false;
    return true;
}

void free_buffer(buffer_t *buf) {
    if (!buf) return;
    close_buffer(buf);
    mtx_destroy(&buf->lock);
    cnd_destroy(&buf->cond);
}

void *get_buffer(buffer_t *buf) {
    if (!buf) return NULL;
    pthread_mutex_lock(&buf_lock);
    while (!buf->closed && !buf->head)
        cnd_wait(&buf->cond, &buf->lock);
    void *item = NULL;
    struct cpr_buffer *entry = buf->head;
    if (entry) {
        buf->head = entry->next;
        if (!buf->head) buf->tail = NULL;
        --buf->count;
        item = entry->item;
    }
    pthread_mutex_unlock(&buf_lock);
    buf_free(entry);
    return item;
}

bool put_buffer(buffer_t *buf, void *item) {
    if (!buf) return false;
    bool result = false;
    pthread_mutex_lock(&buf_lock);
    if (!buf->limit || buf->count < buf->limit) {
        struct cpr_buffer *entry = buf_make(item);
        if (!entry) goto exit;
        if (buf->tail) {
            buf->tail->next = entry;
            buf->tail = entry;
        } else {
            buf->head = buf->tail = entry;
        }
        if (!buf->count) cnd_signal(&buf->cond);
        ++buf->count;
        result = true;
    }
exit:
    pthread_mutex_unlock(&buf_lock);
    return result;
}

bool pri_buffer(buffer_t *buf, void *item) {
    if (!buf) return false;
    bool result = false;
    pthread_mutex_lock(&buf_lock);
    if (!buf->limit || buf->count < buf->limit) {
        struct cpr_buffer *entry = buf_make(item);
        if (!entry) goto exit;
        if (buf->head) {
            entry->next = buf->head;
            buf->head = entry;
        } else {
            buf->head = buf->tail = entry;
        }
        if (!buf->count) cnd_signal(&buf->cond);
        ++buf->count;
        result = true;
    }
exit:
    pthread_mutex_unlock(&buf_lock);
    return result;
}
