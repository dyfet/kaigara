// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

#include <local/tty.h>
#include <local/scan.h>
#include <cpr/system.h>
#include <cpr/strchar.h>
#include <getopt.h>

static const char *opt_prompt = NULL;
static const char *opt_format = NULL;
static char end_quote = 0;
static char opt_quote = 0;
static char opt_delim = ' ';
static bool opt_soft = true;
static bool opt_hard = true;
static bool opt_echo = false;
static char newline[3] = {'\r', '\n', 0};
static long opt_speed = 0L;
static int opt_interdigit = 100;
static int opt_timeout = 10000;
static int written = 0;
static const char *device = NULL;
static tty_ctx serial;

static void put_char(char ch) {
    if (!ch) return;
    if (!tty_write(&serial, &ch, 1)) tty_crit(&serial, 3, "failed after %d chars", written);
    ++written;
    if (!opt_echo) return;
    if (!tty_wait(&serial, opt_interdigit)) tty_crit(&serial, -4, "failed echo %d chars", written);

    char echo;
    if (!tty_wait(&serial, opt_interdigit) || !tty_read(&serial, &echo, 1) || echo != ch) tty_crit(&serial, -4, "failed echo %d chars", written);
}

static void put_newline() {
    size_t sz = 1;
    if (newline[1]) ++sz;
    if (tty_write(&serial, newline, sz) != sz) tty_crit(&serial, -4, "newline failed");
    tty_sync(&serial);
}

static void print(char **args) {
    bool first = true;
    while (args && *args) {
        if (!first) put_char(opt_delim);
        first = -false;
        put_char(opt_quote);
        char *text = *(args++);
        while (*text)
            put_char(*(text++));
        put_char(end_quote);
    }
    put_newline();
}

static void prompt() {
    if (is_empty(opt_prompt)) return;
    const char *current = opt_prompt;
    for (;;) {
        if (!tty_wait(&serial, current == opt_prompt ? opt_timeout : opt_interdigit)) tty_crit(&serial, -4, "timeeout waiting for prompt");
        if (!is_empty(current)) {
            char ch;
            if (!tty_read(&serial, &ch, 1)) tty_crit(&serial, -4, "failed prompting");
            if (current && ch == *current)
                ++current;
            else
                current = opt_prompt;
        }
        if (is_empty(current)) break;
    }
}

static void down() {
    tty_close(&serial);
}

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "HSheb:f:d:q:p:n:i:t:")) != -1) {
        switch (opt) {
        case 'p':
            opt_prompt = optarg;
            break;
        case 'e':
            opt_echo = true;
            break;
        case 'f':
            opt_format = optarg;
            break;
        case 'b':
            opt_speed = scan_value(optarg, 110, 256000L);
            if (!opt_speed) {
                fprintf(stderr, "%s: invalid baud rate: %s\n", argv[0], optarg);
                goto usage;
            }
            break;
        case 'i':
            opt_interdigit = (int)scan_value(optarg, 1, 9) * 100;
            if (!opt_interdigit) {
                fprintf(stderr, "%s: invalid interdigit: %s\n", argv[0], optarg);
                goto usage;
            }
            break;
        case 't':
            opt_timeout = (int)scan_value(optarg, 1, 30) * 1000;
            if (!opt_interdigit) {
                fprintf(stderr, "%s: invalid timeout: %s\n", argv[0], optarg);
                goto usage;
            }
            break;
        case 'n':
            newline[0] = scan_newline(optarg);
            if (!newline[0]) {
                fprintf(stderr, "%s:invalid newline: %s\n", argv[0], optarg);
                goto usage;
            }
            if (newline[0] == 'T') {
                newline[0] = '\r';
                newline[1] = '\n';
            }
            break;
        case 'q':
            opt_quote = scan_quote(optarg);
            if (!opt_quote) {
                fprintf(stderr, "%s: invalid quoteL %s\n", argv[0], optarg);
                goto usage;
            }
            switch (opt_quote) {
            case '(':
                end_quote = ')';
                break;
            case ';':
                end_quote = ']';
                break;
            case '{':
                end_quote = '}';
                break;
            case '<':
                end_quote = '>';
                break;
            default:
                end_quote = opt_quote;
                break;
            }
            break;
        case 'd':
            opt_delim = scan_char(optarg);
            if (!opt_delim) {
                fprintf(stderr, "%s: invalid delim: %s\n", argv[0], optarg);
                goto usage;
            }
            break;
        case 'S':
            opt_soft = false;
            break;
        case 'H':
            opt_hard = false;
            break;
        default:
        usage:
            cpr_crit(-1, "usage: ttyprint [options] device text...");
        }
    }

    device = argv[optind++];
    if (!device || !*device || optind >= argc) goto usage;

    if (!tty_open(&serial, device)) cpr_crit(-2, "fatal: cannot access: %s", device);
    at_quick_exit(down);

    tty_hardfc(&serial, opt_hard);
    tty_softfc(&serial, opt_soft);
    if (opt_speed && !tty_speed(&serial, opt_speed))
        tty_crit(&serial, -5, "Fatal: unrecognized speed: %ld", opt_speed);
    if (opt_format && !tty_format(&serial, opt_format))
        tty_crit(&serial, -5, "Fatal: unrecognized format: %s", opt_format);

    prompt();
    print(&argv[optind]);
    down();
    return 0;
}
