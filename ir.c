#include "ir.h"
#include <gpiod.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#define CHIP "/dev/gpiochip0"
#define GPIO 18

#define START_MIN   13000000ULL
#define START_MAX   14000000ULL

#define REPEAT_MIN  10800000ULL
#define REPEAT_MAX  11600000ULL

#define BIT0_MIN     900000ULL
#define BIT0_MAX    1400000ULL

#define BIT1_MIN    1900000ULL
#define BIT1_MAX    2600000ULL

static uint64_t micros(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    return ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;
}

void *read_ir_th(void *param) {

    struct ir_t *ir = (struct ir_t*) param;

    struct gpiod_chip *chip;
    struct gpiod_line_settings *settings;
    struct gpiod_line_config *line_cfg;
    struct gpiod_request_config *req_cfg;
    struct gpiod_line_request *request;
    struct gpiod_edge_event_buffer *buffer;

    chip = gpiod_chip_open(CHIP);
    if (!chip) {
        perror("gpiod_chip_open");
        return NULL;
    }

    settings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings,
                                      GPIOD_LINE_DIRECTION_INPUT);

    gpiod_line_settings_set_edge_detection(
            settings,
            GPIOD_LINE_EDGE_BOTH);

    line_cfg = gpiod_line_config_new();

    unsigned int offset = GPIO;

    gpiod_line_config_add_line_settings(
            line_cfg,
            &offset,
            1,
            settings);

    req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "ir");

    request = gpiod_chip_request_lines(
                    chip,
                    req_cfg,
                    line_cfg);

    if (!request)
    {
        perror("gpiod_chip_request_lines");
        return NULL;
    }

    buffer = gpiod_edge_event_buffer_new(64);

    uint64_t last = micros();

    printf("Waiting IR...\n");

bool lastWasRising = false;

uint32_t code = 0;
int bit = 0;

while (1)
{
    if (gpiod_line_request_wait_edge_events(request, -1) <= 0)
        continue;

    int n = gpiod_line_request_read_edge_events(request, buffer, 64);

    for (int i = 0; i < n; i++)
    {
        struct gpiod_edge_event *ev =
            gpiod_edge_event_buffer_get_event(buffer, i);

        uint64_t now = micros();
        uint32_t dt = now - last;
        last = now;

        bool rising =
            gpiod_edge_event_get_event_type(ev) ==
            GPIOD_EDGE_EVENT_RISING_EDGE;

        if (lastWasRising && !rising)
        {
            /* SPACE */

            if (dt > 3500 && dt < 5000)
            {
                bit = 0;
                code = 0;
                continue;
            }

            if (bit >= 0)
            {
                if (dt > 300 && dt < 900)
                {
                    /* bit 0 */
                    bit++;
                }
                else if (dt > 1200 && dt < 2200)
                {
                    /* bit 1 */
                    code |= (1u << bit);
                    bit++;
                }
                else
                {
                    bit = 0;
                    code = 0;
                }

                if (bit == 32)
                {
                    printf("the code 0x%08X\n", code);

                    switch (code) {
                        case 0x8D72FB04:
                            ir->key = 'R';
                            ir->new_key = 1;
                            printf("KeY RED\n");
                            break;

                        case 0x8E71FB04:
                            ir->key = 'G';
                            ir->new_key = 1;
                            printf("KeY Green\n");
                            break;

                        case 0x9C63FB04:
                            ir->key = 'Y';
                            ir->new_key = 1;
                            printf("KeY Yellow\n");
                            break;

                        case 0x9E61FB04:
                            ir->key = 'B';
                            ir->new_key = 1;
                            printf("KeY Blue\n");
                            break;

                        case 0xBF40FB04:
                            ir->key = 'u';
                            ir->new_key = 1;
                            printf("KeY UP\n");
                            break;

                        case 0xBB44FB04:
                            ir->key = 'o';
                            ir->new_key = 1;
                            printf("KeY OK\n");
                            break;

                        case 0xBE41FB04:
                            ir->key = 'd';
                            ir->new_key = 1;
                            printf("KeY Down\n");
                            break;

                        case 0xD728FB04:
                            ir->key = 'r';
                            ir->new_key = 1;
                            printf("KeY Down\n");
                            break;

                        default:
                    }
                    bit = 0;
                    code = 0;

                }
            }
        }

        lastWasRising = rising;
    }
}

    gpiod_edge_event_buffer_free(buffer);
    gpiod_line_request_release(request);
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);
}

struct ir_t *read_ir_init() {

    pthread_t th;
    struct ir_t *ir = (struct ir_t*) malloc(sizeof(struct ir_t));
    ir->key = 0;
    ir->new_key = 0;

    pthread_create(&th, NULL, read_ir_th, ir);

    return ir;
}

char ir_get_key(struct ir_t *ir) {

    if (ir->new_key == 1) {
        ir->new_key = 0;
        return ir->key;
    }

    return 0;
}