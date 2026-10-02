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

static void prompt() {
    if (is_empty(opt_prompt)) return;
    size_t sz = cpr_strlen(opt_prompt, 256);
    if (tty_write(&serial, opt_prompt, sz) != sz) tty_crit(&serial, -4, "prompt failed");
    tty_sync(&serial);
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

    tty_line(&serial, newline);
    tty_echo(&serial, opt_echo);
    tty_hardfc(&serial, opt_hard);
    tty_softfc(&serial, opt_soft);
    if (opt_speed && !tty_speed(&serial, opt_speed))
        tty_crit(&serial, -5, "Fatal: unrecognized speed: %ld", opt_speed);
    if (opt_format && !tty_format(&serial, opt_format))
        tty_crit(&serial, -5, "Fatal: unrecognized format: %s", opt_format);

    char buf[tty_size(&serial) + 1];
    prompt();
    size_t size = tty_gets(&serial, buf, sizeof(buf));
    buf[size] = 0;
    char *cp = strchr(buf, newline[0]);
    if (cp) *cp = 0;
    printf("%s\n", buf);
    down();
    return 0;
}
