#include <service/terminal.h>
#include "ldacs_role.h"
#include "http_core.h"
#include "cconfig.h"

is_stop volatile stop_flag = FALSE;

mem_size_t max_mem = 2 * GB + 1000 * MB + 1000 * KB;
mem_size_t mempool_size = 1 * GB + 500 * MB + 500 * KB;

config_t config = {
    .port = 8081,
    .debug = FALSE,
    .timeout = 999,
    .worker = 4,
    .ip_ver = IPVERSION_4,
    .init_fl_freq = 960.0,
    .use_http = FALSE,
    .auto_auth = TRUE,
    .UA = 0,
    .is_merged = FALSE,
    .role = LD_UNDEFINED,
    .direct = FALSE,
    .pipe_fd = 0,
};

static void sigint_handler(int signum) {
    if (config.role == LD_AS || config.role == LD_SGW) {
        stop_rcu();
        usleep(500000);
    }
    stop_flag = TRUE;
    usleep(500000);

    /* free the static directories and other objects  */
    // mempool_clear(global_mp);

    log_info("ldacs simulator(PID: %u) exit...", getpid());
    //    kill(-getpid(), SIGINT);
    exit(0);
}

void init_signal() {
    signal(SIGINT, sigint_handler);
    signal(SIGABRT, sigint_handler);
    signal(SIGQUIT, sigint_handler);
    signal(SIGTERM, sigint_handler);
    signal(SIGPIPE, SIG_IGN); // client close, server write will recv sigpipe
}


static int init_config_path() {
    char *role_config = NULL;
    if (config.config_path[0] == '\0') {
        switch (config.role) {
            case LD_AS: {
                role_config = DEFAULT_AS_CONFIG_NAME;
                break;
            }
            case LD_GS: {
                role_config = DEFAULT_GS_CONFIG_NAME;
                break;
            }
            case LD_SGW: {
                role_config = DEFAULT_SGW_CONFIG_NAME;
                break;
            }
            default:
                return -1;
        }
        snprintf(config.config_path, sizeof(config.config_path), "%s/%s/%s", INSTALL_PREFIX, DEFAULT_CONFIG_PATH,
                 role_config);
    }
    snprintf(config.log_dir, sizeof(config.log_dir), "%s/%s", INSTALL_PREFIX, DEFAULT_LOG_PATH);
    return 0;
}

int opt_parse(int argc, char *const *argv) {
    int c;
    while ((c = getopt(argc, argv, "p:f:dt:w:c:AGWHMBED")) != -1) {
        switch (c) {
            case 'p': {
                config.port = strtol(optarg, NULL, 10);
                break;
            }
            case 'd': {
                config.debug = TRUE;
                break;
            }
            case 't': {
                config.timeout = strtol(optarg, NULL, 10);
                break;
            }
            case 'w': {
                config.worker = strtol(optarg, NULL, 10);
                if (config.worker > sysconf(_SC_NPROCESSORS_ONLN)) {
                    fprintf(stderr,
                            "Config ERROR: worker num greater than cpu available cores.\n");
                    return ERROR;
                }
                break;
            }
            case 'c': {
                memcpy(config.config_path, optarg, strlen(optarg));
                if (init_config_path() != OK) {
                    return ERROR;
                }
                parse_config(&config, config.config_path);
                break;
            }
            case 'A':
            case 'G':
            case 'W': {
                if (config.role != LD_UNDEFINED) {
                    return ERROR;
                }
                switch (c) {
                    case 'A':
                        config.role = LD_AS;
                        break;
                    case 'G':
                        config.role = LD_GS;
                        break;
                    case 'W':
                        config.role = LD_SGW;
                        break;
                    default:
                        return ERROR;
                }
                if (init_config_path() != OK) {
                    return ERROR;
                }
                parse_config(&config, config.config_path);
            }
            case 'H': {
                config.use_http = TRUE;
                break;
            }
            case 'D': {
                config.direct = TRUE;
                break;
            }
            case 'B': {
                if (config.role == LD_SGW) {
                    config.port = strtol("55551", NULL, 10);
                }
                config.is_merged = TRUE;
                config.is_beihang = TRUE;
                config.is_e304 = FALSE;
                break;
            }
            case 'E': {
                if (config.role == LD_SGW) {
                    config.port = strtol("55551", NULL, 10);
                }
                config.is_merged = TRUE;
                config.is_beihang = FALSE;
                config.is_e304 = TRUE;
                break;
            }
            case 'f': {
                config.pipe_fd = (int)strtol(optarg, NULL, 10);
                config.pipe_file = fdopen(config.pipe_fd, "w");
                if (config.pipe_file == NULL) {
                    perror("fdopen");
                    exit(EXIT_FAILURE);
                }
                printf("AS: %d ( 子进程 PID: %d ) 启动\n", config.UA, getpid());
                // 重定向标准输出到 /dev/null，这样子进程的printf就不会显示在父进程终端上
                int dev_null = open("/dev/null", O_WRONLY);
                if (dev_null != -1) {
                    dup2(dev_null, STDERR_FILENO); // 重定向stdout
                    dup2(dev_null, STDOUT_FILENO); // 重定向stdout
                    close(dev_null);
                }
                break;
            }
            default: {
                return ERROR;
            }
        }
    }
    return OK;
}

static void usage(const char *executable) {
    printf("Usage: %s [-c config path] "
           "[-H (http mode)] [-M (Merge mode)]\n",
           executable);
}


int main(int argc, char **argv) {
    /* init memory pool */
    // if ((global_mp = mempool_init(max_mem, mempool_size)) == NULL) {
    //     log_error("Memory Pool init failed!");
    //     exit(0);
    // }
    //
    // log_warn("%p %p", global_mp->mlist, global_mp->mlist->alloc_list);

    if (argc < 2 || opt_parse(argc, argv) != OK) {
        usage(argv[0]);
        exit(ERROR);
    }

    log_init(LOG_DEBUG, config.log_dir, roles_str[config.role]);
    init_signal();

    if (config.role == LD_AS || config.role == LD_GS)
        init_rcu(config.use_http == FALSE ? terminal_service : http_service);
    else if (config.role == LD_SGW) {
        run_sgw();
    }

    return 0;
}


