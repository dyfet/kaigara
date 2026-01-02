// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include "mempager.h"

mempager_t pager_create(size_t pagesize, unsigned max) {
    mempager_t pager = (mempager_t)malloc(sizeof(struct _mempager));
    if (!pager)
        return NULL;

    pager->limit = max;
    pager->size = pagesize;
    pager->count = 0;
    pager->head = pager->tail = pager->free = NULL;
    return pager;
}

void *pager_data(mempage_t page) {
    if (!page)
        return NULL;

    return ((uint8_t *)page) + page->used;
}

void pager_reset(mempager_t pager) {
    mempage_t next, page = pager->free;
    while (page) {
        next = page->next;
        free(page);
        page = next;
    }

    page = pager->head;
    while (page) {
        next = page->next;
        free(page);
        page = next;
    }

    pager->count = 0;
    pager->head = pager->tail = pager->free = NULL;
}

void pager_free(mempager_t pager) {
    if (!pager)
        return;

    pager_reset(pager);
    free(pager);
}

mempage_t pager_request(mempager_t pager) {
    mempage_t page = pager->free;
    if (page)
        pager->free = page->next;
    else if (pager->limit && pager->count >= pager->limit)
        return NULL;
    else {
        page = (mempage_t)malloc(pager->size);
        if (!page)
            return NULL;
        ++pager->count;
    }

    if (pager->tail)
        pager->tail->next = page;
    else
        pager->head = page;

    page->prev = pager->tail;
    page->next = NULL;
    page->used = sizeof(struct _mempage);
    pager->tail = page;
    return page;
}

void *pager_alloc(mempager_t pager, size_t size) {
    uint8_t *data;
    mempage_t page;

    if (size + sizeof(struct _mempage) > pager->size)
        return NULL;

    page = pager->head;
    while (page) {
        if (page->used + size <= pager->size) {
            data = ((uint8_t *)page) + page->used;
            page->used += size;
            return data;
        }
        page = page->next;
    }

    page = pager_request(pager);
    if (!page)
        return NULL;

    page->used += size;
    return ((uint8_t *)page) + sizeof(struct _mempage);
}

char *pager_strdup(mempager_t pager, const char *str) {
    size_t size = cpr_strlen(str, pager->size - sizeof(struct _mempage) - 1);
    char *out = pager_alloc(pager, ++size);
    if (!out)
        return NULL;

    cpr_strcpy(out, str, size);
    return out;
}
