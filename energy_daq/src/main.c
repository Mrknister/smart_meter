// The MIT License (MIT)

// Copyright (c) 2015 Thomas Kriechbaumer

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#define _GNU_SOURCE

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>
#include "daq_interface.h"

#include <libusb-1.0/libusb.h>
#include <ftdi.h>

#define NUMBER_OF_MEDAL_CHANNELS 7
#ifdef __MACH__
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 0
#define CLOCK_MONOTONIC_RAW 0
#include <sys/time.h>

//clock_gettime is not implemented on OSX
int clock_gettime(int clk_id, struct timespec* t) {
    struct timeval now;
    int rv = gettimeofday(&now, NULL);
    if (rv) return rv;
    t->tv_sec  = now.tv_sec;
    t->tv_nsec = now.tv_usec * 1000;
    return 0;
}
#endif

#define DEFAULT_VENDOR 0x0403
#define DEFAULT_PRODUCT 0x6014
#define DEFAULT_LATENCY 255
#define DEFAULT_PACKETS_PER_TRANSFER 1
#define DEFAULT_NUM_TRANSFERS 2048
#define DEFAULT_MAX_FILESIZE 80640000 // dividable by PACKET_LENGTH, yields 15min@6400Hz
#define DEFAULT_FREQUENCY 6400

#define MICROCONTROLLER_CLOCK_FREQUENCY 14745600
#define PACKET_LENGTH 14
#define DEVICE_TYPE_MICROCONTROLLER 0x1
#define DEVICE_TYPE_FPGA 0x2

#define WRITE_DATA(buffer, length) \
  do { \
    if (length == 0) { \
      continue; \
    } \
    if (fwrite(buffer, length, 1, state.current_file) != 1) { \
      write_log("ERROR: write failed"); \
      write_log("  buffer:%p length:%zu current_file:%p ftell:%lu\n", (void*)buffer, length, (void*)state.current_file, ftell(state.current_file)); \
      state.exit_requested = true; \
      return false; \
    } \
    state.bytes_written += length; \
  } while (0);


typedef struct config {
    int verbose;
    int vendor_id;
    int product_id;
    unsigned short latency;
    unsigned int packets_per_transfer;
    unsigned int num_transfers;
    size_t max_filesize;
    unsigned int frequency;
    char *output_filename;
    char *output_extension;
    char *post_split_script;
    uint64_t stop_after_bytes;
    uint64_t stop_after_seconds;
    uint64_t stop_after_splits;
} DAQConfig;
static DAQConfig config;

typedef struct state {
    int device_type;

    bool exit_requested;
    FILE *log_file;
    struct ftdi_context *ftdi;

    struct timespec start_time;

    FILE *current_file;
    char *current_filename;
    struct timeval current_starttime;

    bool trigger_id_check_initialized;
    uint16_t last_trigger_id;
    uint8_t last_incomplete_packet[PACKET_LENGTH];
    short last_incomplete_packet_length;

    uint64_t bytes_written;
    uint64_t split_file_index;


} DAQState;
static DAQState state;

void print_usage(const char *);

#if DEFAULT_MAX_FILESIZE % PACKET_LENGTH != 0
#error DEFAULT_MAX_FILESIZE must be a multiple of PACKET_LENGTH
#endif


/* Write a log message to stderr and the log file
 *
 * \param format a format string
 * \params vargs the arguments to be formatted
 */
static void write_log(const char *format, ...) {
    va_list args;
    va_start(args, format);

    vfprintf(stderr, format, args);
    fflush(stderr);

    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    struct tm lt;
    localtime_r(&current_time.tv_sec, &lt);

    char datetime_string[64] = {0};
    strftime(datetime_string, sizeof(datetime_string), "%Y-%m-%dT%H:%M:%S%z", &lt);

    fprintf(state.log_file, "[%s] ", datetime_string);

    va_end(args);
    va_start(args, format);
    vfprintf(state.log_file, format, args);
    fflush(state.log_file);

    va_end(args);
}

/* Signal Handler
 *
 * Handles a signal event to gracefully stop capturing
 */
static void signal_handler(int signum) {
    state.exit_requested = true;
}

/* Send a command to the microcontroller
 *
 * \param  cmd the raw command string
 * \param  check true if the response should be "OK"
 */
void send_command(char *command, bool check, char *response) {
    char *cmd = NULL;
    asprintf(&cmd, "%s\n", command);
    ftdi_write_data(state.ftdi, (const unsigned char *) cmd, strlen(cmd));

    if (check) {
        char read_buffer[512] = {0};
        int recv = 0;
        int retries = 100;
        while (recv == 0 && --retries) {
            recv = ftdi_read_data(state.ftdi, (unsigned char *) read_buffer, sizeof(read_buffer));
        }

        if (state.device_type == DEVICE_TYPE_MICROCONTROLLER) {
            if (retries == 0 || recv < 3 || strncmp("OK\n", (read_buffer + recv - 3), 3) != 0) {
                write_log("ERROR: Protocol handshake failed.\n");
                exit(1);
            }
        } else {
            if (retries == 0 || recv != 16 || strncmp("\r\nOK\r\n", read_buffer + recv - 6, 6)) {
                write_log("ERROR: Protocol handshake failed.\n");
                exit(1);
            }
        }

        if (response) {
            if (state.device_type == DEVICE_TYPE_MICROCONTROLLER) {
                strncpy(response, read_buffer, recv - 3);
                if (response[strlen(response) - 1] == '\n') {
                    response[strlen(response) - 1] = 0;
                }
            } else {
                strncpy(response, read_buffer + 2, recv - 8);
            }
        }
    }
}

/* Send the start-capturing command
 *
 */
void start_capture() {
    if (state.device_type == DEVICE_TYPE_MICROCONTROLLER) {
        send_command("ATR", false, NULL);
    } else {
        send_command("ATS0=1", false, NULL);
    }
}

/* Send the stop-capturing command
 *
 */
void stop_capture(bool check) {
    if (state.device_type == DEVICE_TYPE_MICROCONTROLLER) {
        send_command("ATX", check, NULL);
    } else {
        send_command("ATS0=1A", false, NULL);
        send_command("ATS0=0", false, NULL);
        if (check) {
            char response[64] = {0};
            send_command("ATS0?", true, response);
            write_log("ATS0 == %s\n", response);
        }
    }
}

/* Set the capturing frequency
 * Capturing must be stopped first
 *
 */
void set_frequency() {
    char buffer[16] = {0};
    if (state.device_type == DEVICE_TYPE_MICROCONTROLLER) {
        uint16_t div = (uint16_t) roundf(MICROCONTROLLER_CLOCK_FREQUENCY / ((float) config.frequency) - 1);
        snprintf(buffer, sizeof(buffer), "ATS0=%u", div);
        send_command(buffer, true, NULL);
    } else {
        uint32_t ticks = (uint32_t) (100000000 / config.frequency); // 10ns ticks
        snprintf(buffer, sizeof(buffer), "ATS1=%X", ticks);
        send_command(buffer, false, NULL);

        char response[64] = {0};
        send_command("ATS1?", true, response);
        write_log("ATS1 == %s\n", response);
    }
}

/* Print the current firmware version
 */
void print_firmware_version() {
    if (state.device_type == DEVICE_TYPE_MICROCONTROLLER) {
        char version[64] = {0};
        send_command("ATI", true, version);
        write_log("Firmware Version µC: %s\n", version);
    } else {
        write_log("Firmware Version FPGA: not available");
    }
}

/* Closes a file and removes .inprogress postfix
 *
 */
void finish_file() {
    if (state.current_file) {
        fclose(state.current_file);
        state.current_file = NULL;
    }

    if (state.current_filename) {
        char *new_filename = (char *) calloc(1, strlen(state.current_filename) + 1);
        strncpy(new_filename, state.current_filename, strlen(state.current_filename) - 11);
        rename(state.current_filename, new_filename);

        if (config.post_split_script) {
            if (fork() == 0) {
                setpgid(0, 0);

                char frequency[12];
                sprintf(frequency, "%u", config.frequency);

                errno = 0;
                int ret = execl(config.post_split_script, basename(config.post_split_script), new_filename, frequency,
                                (state.device_type == DEVICE_TYPE_MICROCONTROLLER ? "microcontroller" : "fpga"), NULL);
                if (ret) {
                    write_log("Post-split script execl failed! %s\n", strerror(errno));
                    exit(1);
                }
            }
        }

        free(new_filename);
        new_filename = NULL;
        free(state.current_filename);
        state.current_filename = NULL;
    }
}

/* Opens a new file for writing
 *
 */
bool start_file() {
    gettimeofday(&state.current_starttime, NULL);

    char datetime_string[32] = {0};
    char timezone_string[8] = {0};
    struct tm lt;
    localtime_r(&state.current_starttime.tv_sec, &lt);
    strftime(datetime_string, sizeof(datetime_string), "%Y-%m-%dT%H-%M-%S", &lt);
    strftime(timezone_string, sizeof(timezone_string), "%z", &lt);

    asprintf(&state.current_filename, "%s-%s.%06ldT%s-%07" PRIu64 ".%s.inprogress", config.output_filename,
             datetime_string, state.current_starttime.tv_usec, timezone_string, state.split_file_index,
             config.output_extension);

    state.current_file = fopen(state.current_filename, "w+");
    if (state.current_file == NULL) {
        write_log("ERROR: Can't open file %s: %s\n", state.current_filename, strerror(errno));
        return false;
    }

    write_log("Writing to file %llu: %s\n", state.split_file_index, state.current_filename);
    ++state.split_file_index;
    return true;
}

/* Checks the trigger ids of a single sample
 *
 */
bool check_trigger_id(uint8_t *buffer) {


    uint16_t trigger_id = 0;

    if (state.device_type == DEVICE_TYPE_MICROCONTROLLER) {
        trigger_id = buffer[0] << 8 | buffer[1];
    } else {
        trigger_id = buffer[1] << 8 | buffer[0];
    }

    if (state.trigger_id_check_initialized) {
        if (trigger_id != state.last_trigger_id + 1 && (trigger_id != 0 && state.last_trigger_id != 65535)) {
            return false;
        }
    } else {
        state.trigger_id_check_initialized = true;
    }
    state.last_trigger_id = trigger_id;

    // because we are lazy we also put the packet into the data_queue here
    uint16_t channels[NUMBER_OF_MEDAL_CHANNELS] = {0}; // medal channels + voltage

    int j = 0;
    for (; j < NUMBER_OF_MEDAL_CHANNELS; ++j) {
        int i_counter = 13;

        for (; i_counter >= 2; --i_counter) {

            channels[j] <<= 1;

            int or_value = (buffer[i_counter] & (1 << (j + 1))) >= 1 ;
            channels[j] |= or_value;


        }
    }


    float calibrated_channels[NUMBER_OF_MEDAL_CHANNELS];
    calibrated_channels[0] = (((float) channels[0]) -2500) * 0.015151515f; //  1 / 0.066 V/A * (4.096 / 4096)
    calibrated_channels[1] = (((float) channels[1]) -2500) * 0.005405405f; //  1 / 0.185 V/A * (4.096 / 4096)
    calibrated_channels[2] = (((float) channels[2]) -2500) *0.005405405f; //  1 / 0.185 V/A * (4.096 / 4096)
    calibrated_channels[3] = (((float) channels[3]) -2500) *0.005405405f; //  1 / 0.185 V/A * (4.096 / 4096)
    calibrated_channels[4] = (((float) channels[4]) -2500) *0.005405405f; //  1 / 0.185 V/A * (4.096 / 4096)
    calibrated_channels[5] = (((float) channels[5]) -2500) * 0.005405405f; //  1 / 0.185 V/A * (4.096 / 4096)
    calibrated_channels[6] = (((float) channels[6]) -2380) *
                             0.2853f; // 230V / (6V * 1.478 estimated IdleVolt) * (100000Ohm + 10000Ohm) / 10000Ohm) * (4.096 / 4096)
    addMEDALDataPoint(calibrated_channels[0], calibrated_channels[1], calibrated_channels[2], calibrated_channels[3],
                      calibrated_channels[4], calibrated_channels[5], calibrated_channels[6]);

    return true;
}

/* Verifies that all trigger ids of the current buffer (and previous incomplete packets)
 * are strictly increasing (including overflow)
 */
bool verify_trigger_ids(uint8_t *buffer, size_t length) {
    if (state.last_incomplete_packet_length > 0) {
        memcpy(state.last_incomplete_packet + state.last_incomplete_packet_length, buffer,
               PACKET_LENGTH - state.last_incomplete_packet_length);
        if (!check_trigger_id(state.last_incomplete_packet)) {
            return false;
        }
    }

    size_t offset = (state.last_incomplete_packet_length ? PACKET_LENGTH - state.last_incomplete_packet_length : 0);
    size_t length_to_check = length - offset;
    state.last_incomplete_packet_length = length_to_check % PACKET_LENGTH;
    length_to_check -= state.last_incomplete_packet_length;

    size_t i;
    for (i = 0; i < length_to_check; i += PACKET_LENGTH) {
        if (!check_trigger_id(buffer + offset + i)) {
            return false;
        }
    }

    memset(state.last_incomplete_packet, 0, PACKET_LENGTH);
    memcpy(state.last_incomplete_packet, buffer + offset + length_to_check, state.last_incomplete_packet_length);

    return true;
}


/* Callback for received data buffers
 *
 * \param  buffer the memory location of the received data
 * \param  length of the received data
 *
 * returns true to stop capturing, false to continue capturing
 */
static bool read_callback(uint8_t *buffer, size_t length) {
    if (buffer == NULL || length == 0 || state.exit_requested) {
        // no buffer?
        // no data received?
        // exit early?
        return state.exit_requested;
    }

    if (state.current_file == NULL) {
        if (!start_file()) {
            state.exit_requested = true;
            return state.exit_requested;
        }
    }

    if (config.stop_after_bytes && state.bytes_written > config.stop_after_bytes) {
        finish_file();
        write_log("Stopping after %" PRIu64 " bytes.", config.stop_after_bytes);
        state.exit_requested = true;
        return state.exit_requested;
    }

    struct timespec current_monotonic_time;
    clock_gettime(CLOCK_MONOTONIC, &current_monotonic_time);
    if (config.stop_after_seconds &&
        (uint64_t) current_monotonic_time.tv_sec - (uint64_t) state.start_time.tv_sec > config.stop_after_seconds) {
        finish_file();
        write_log("Stopping after %" PRIu64 " seconds.", config.stop_after_seconds);
        state.exit_requested = true;
        return state.exit_requested;
    }

    if (!verify_trigger_ids(buffer, length)) {
        stop_capture(false);
        write_log("ERROR: Trigger IDs do not match. Received data is invalid.\n");
        state.exit_requested = true;
        return state.exit_requested;
    }

    size_t until_full = config.max_filesize - ftell(state.current_file);
    if (length > until_full) {
        WRITE_DATA(buffer, until_full);

        finish_file();

        if (config.stop_after_splits && state.split_file_index > config.stop_after_splits) {
            write_log("Stopping after %" PRIu64 " splits.", config.stop_after_splits);
            state.exit_requested = true;
            return state.exit_requested;
        }

        if (!start_file()) {
            state.exit_requested = true;
            return state.exit_requested;
        }

        buffer += until_full;
        length -= until_full;
    }


    WRITE_DATA(buffer, length);

    return state.exit_requested;
}

/* Handle callbacks
 *
 * With Exit request, free memory and release the transfer
 *
 * usb_state is only set when some error happens
 */
static void LIBUSB_CALL ftdi_readstream_cb(struct libusb_transfer *transfer) {
    int *usb_state = transfer->user_data;

    if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
        // FTDI always adds two status bytes in the front
        uint8_t *ptr = transfer->buffer + 2;
        size_t length = transfer->actual_length - 2;

        if (read_callback(ptr, length)) {
            free(transfer->buffer);
            libusb_free_transfer(transfer);
        } else {
            memset(transfer->buffer, 0, transfer->actual_length);
            transfer->status = -1;
            *usb_state = libusb_submit_transfer(transfer);
        }
    } else {
        write_log("ERROR: Unknown transfer status: %d\n", transfer->status);
        *usb_state = LIBUSB_ERROR_IO;
    }
}

/* Streaming reading of data from the device
 *
 * Use asynchronous transfers in libusb-1.0 for high-performance
 * streaming of data from a device interface back to the PC. This
 * function continuously transfers data until either an error occurs
 * or the callback returns a nonzero value. This function returns
 * a libusb error code or the callback's return value.
 *
 * For every contiguous block of received data, the callback will
 * be invoked.
 *
 */
int read_stream() {
    struct libusb_transfer **transfers;
    int usb_state = 1;
    int bufferSize = config.packets_per_transfer * state.ftdi->max_packet_size;
    int err = 0;

    stop_capture(true);
    set_frequency();

    // set up all transfers
    transfers = calloc(config.num_transfers, sizeof(*transfers));
    if (!transfers) {
        err = LIBUSB_ERROR_NO_MEM;
        goto cleanup;
    }

    size_t i;
    for (i = 0; i < config.num_transfers; ++i) {
        struct libusb_transfer *transfer = libusb_alloc_transfer(0);
        transfers[i] = transfer;
        if (!transfer) {
            err = LIBUSB_ERROR_NO_MEM;
            goto cleanup;
        }

        libusb_fill_bulk_transfer(transfer, state.ftdi->usb_dev, state.ftdi->out_ep, malloc(bufferSize), bufferSize,
                                  ftdi_readstream_cb, &usb_state, 0);

        if (!transfer->buffer) {
            err = LIBUSB_ERROR_NO_MEM;
            goto cleanup;
        }

        memset(transfer->buffer, 0, bufferSize);
        transfer->status = -1;
        err = libusb_submit_transfer(transfer);
        if (err) {
            goto cleanup;
        }
    }

    // run the transfers, and periodically assess progress.
    clock_gettime(CLOCK_MONOTONIC, &state.start_time);
    start_capture();
    do {
        struct timeval timeout = {0, state.ftdi->usb_read_timeout * 1000};

        int err = libusb_handle_events_timeout(state.ftdi->usb_ctx, &timeout);
        if (err == LIBUSB_ERROR_INTERRUPTED) {
            // restart interrupted events
            err = libusb_handle_events_timeout(state.ftdi->usb_ctx, &timeout);
        }
        if (!usb_state) {
            usb_state = err;
        }
    } while (!state.exit_requested);

    // cancel any outstanding transfers, and free memory.
    cleanup:
    if (transfers) {
        free(transfers);
        transfers = NULL;
    }

    if (err) {
        write_log("ERROR: %d\n", err);
        return err;
    } else {
        return usb_state;
    }
}

/* Initialise the USB device with an FTDI context
 *
 * returns true on success
 */
bool init_ftdi() {
    state.ftdi = ftdi_new();
    if (state.ftdi == 0) {
        write_log("ERROR: ftdi_new failed\n");
        return false;
    }

    state.device_type = DEVICE_TYPE_MICROCONTROLLER;
    if (ftdi_usb_open_desc_index(state.ftdi, config.vendor_id, config.product_id, "ADC sampler", NULL, 0) < 0) {
        state.device_type = DEVICE_TYPE_FPGA;
        if (ftdi_usb_open_desc_index(state.ftdi, config.vendor_id, config.product_id, "6 channel sampling ADC", NULL,
                                     0) < 0) {
            write_log("ERROR: Can't open ftdi device: %s\n", ftdi_get_error_string(state.ftdi));
            ftdi_free(state.ftdi);
            return false;
        }
    }

    if (ftdi_set_latency_timer(state.ftdi, config.latency)) {
        write_log("ERROR: Can't set latency: %s\n", ftdi_get_error_string(state.ftdi));
        ftdi_usb_close(state.ftdi);
        ftdi_free(state.ftdi);
        return false;
    }

    if (ftdi_usb_purge_buffers(state.ftdi) < 0) {
        write_log("ERROR: Can't purge buffers.\n");
        ftdi_usb_close(state.ftdi);
        ftdi_free(state.ftdi);
        return false;
    }

    return true;
}

/* Parse environment configuration
 *
 */
bool parse_env() {
    errno = 0;

    char *value = getenv("ENERGY_DAQ_VERBOSE");
    config.verbose = value ? strcasecmp(value, "yes") == 0 || strcasecmp(value, "true") == 0 ||
                             strcmp(value, "1") == 0 : true;

    value = getenv("ENERGY_DAQ_VENDOR_ID");
    config.vendor_id = value ? strtol(value, NULL, 0) : DEFAULT_VENDOR;

    value = getenv("ENERGY_DAQ_PRODUCT_ID");
    config.product_id = value ? strtol(value, NULL, 0) : DEFAULT_PRODUCT;

    value = getenv("ENERGY_DAQ_LATENCY");
    config.latency = value ? strtol(value, NULL, 0) : DEFAULT_LATENCY;

    value = getenv("ENERGY_DAQ_PACKETS_PER_TRANSFER");
    config.packets_per_transfer = value ? strtoul(value, NULL, 0) : DEFAULT_PACKETS_PER_TRANSFER;

    value = getenv("ENERGY_DAQ_NUM_TRANSFERS");
    config.num_transfers = value ? strtoul(value, NULL, 0) : DEFAULT_NUM_TRANSFERS;

    value = getenv("ENERGY_DAQ_MAX_FILESIZE");
    config.max_filesize = value ? strtoul(value, NULL, 0) : DEFAULT_MAX_FILESIZE;

    value = getenv("ENERGY_DAQ_FREQUENCY");
    config.frequency = value ? strtol(value, NULL, 0) : DEFAULT_FREQUENCY;

    config.output_filename = getenv("ENERGY_DAQ_OUTPUT_FILENAME");

    config.output_extension = getenv("ENERGY_DAQ_OUTPUT_EXTENSION");

    config.post_split_script = getenv("ENERGY_DAQ_POST_SPLIT_SCRIPT");

    value = getenv("ENERGY_DAQ_STOP_AFTER_BYTES");
    config.stop_after_bytes = value ? strtoul(value, NULL, 0) : 0;

    value = getenv("ENERGY_DAQ_STOP_AFTER_SECONDS");
    config.stop_after_seconds = value ? strtoul(value, NULL, 0) : 0;

    value = getenv("ENERGY_DAQ_STOP_AFTER_SPLITS");
    config.stop_after_splits = value ? strtoul(value, NULL, 0) : 0;

    return errno == 0;
}

/* Parse argv configuration
 *
 */
bool parse_argv(int argc, char **argv) {
    while (true) {
        static struct option long_options[] = {{"verbose",            no_argument,       &(config.verbose), true},
                                               {"silent",             no_argument,       &(config.verbose), false},
                                               {"quiet",              no_argument,       &(config.verbose), false},
                                               {"vendor",             required_argument, 0, 'v'},
                                               {"product",            required_argument, 0, 'p'},
                                               {"latency",            required_argument, 0, 'l'},
                                               {"transfers",          required_argument, 0, 't'},
                                               {"split",              required_argument, 0, 's'},
                                               {"frequency",          required_argument, 0, 'f'},
                                               {"post-split-script",  required_argument, 0, '0'},
                                               {"stop-after-bytes",   required_argument, 0, '1'},
                                               {"stop-after-seconds", required_argument, 0, '2'},
                                               {"stop-after-splits",  required_argument, 0, '3'},
                                               {0, 0,                                    0, 0}};

        int option_index = 0;

        int c = getopt_long(argc, argv, "hv:p:l:t:s:f:", long_options, &option_index);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0) {
                    break;
                }
                fprintf(stderr, "option %s", long_options[option_index].name);
                if (optarg) {
                    fprintf(stderr, " with arg %s", optarg);
                }
                fprintf(stderr, "\n");
                break;

            case 'v':
                errno = 0;
                config.vendor_id = strtol(optarg, NULL, 0);
                if ((config.vendor_id == 0 && errno) || config.vendor_id > 0xFFFF) {
                    fprintf(stderr, "vendor must be between 0x0 and 0xFFFF, given: %s\n", optarg);
                    return false;
                }
                break;

            case 'p':
                errno = 0;
                config.product_id = strtol(optarg, NULL, 0);
                if ((config.product_id == 0 && errno) || config.product_id > 0xFFFF) {
                    fprintf(stderr, "product must be between 0x0 and 0xFFFF, given: %s\n", optarg);
                    return false;
                }
                break;

            case 'l':
                errno = 0;
                config.latency = strtol(optarg, NULL, 0);
                if ((config.latency == 0 && errno) || config.latency < 1 || config.latency > 255) {
                    fprintf(stderr, "latency must be between 1 and 255, given: %s\n", optarg);
                    return false;
                }
                break;

            case 't':
                errno = 0;
                config.num_transfers = strtoul(optarg, NULL, 0);
                if ((config.num_transfers == 0 && errno) || config.num_transfers < 1 || config.num_transfers > 4096) {
                    fprintf(stderr, "transfer queue must be between 1 and 4096, given: %s\n", optarg);
                    return false;
                }
                break;

            case 's':
                errno = 0;
                config.max_filesize = strtoul(optarg, NULL, 0);
                if ((config.max_filesize == 0 && errno) || config.max_filesize % PACKET_LENGTH != 0 ||
                    config.max_filesize < 512 || config.max_filesize > 4294967295u) {
                    fprintf(stderr,
                            "max. filesize must be multiple of %u, greater than 512 and lower than 4294967295, given: %s\n",
                            PACKET_LENGTH, optarg);
                    return false;
                }
                break;

            case 'f':
                errno = 0;
                config.frequency = strtol(optarg, NULL, 0);
                if ((config.frequency == 0 && errno) || config.frequency < 300 || config.frequency > 50000) {
                    fprintf(stderr, "frequency must be between 300 and 50000 Hz, given: %s\n", optarg);
                    return false;
                }
                break;

            case '0':
                config.post_split_script = optarg;
                break;

            case '1':
                errno = 0;
                config.stop_after_bytes = strtoul(optarg, NULL, 0);
                if ((config.stop_after_bytes == 0 && errno) || config.stop_after_bytes < 1) {
                    fprintf(stderr, "stop-after-bytes must be greater or equal than 1, given: %s\n", optarg);
                    return false;
                }
                break;

            case '2':
                errno = 0;
                config.stop_after_seconds = strtoul(optarg, NULL, 0);
                if ((config.stop_after_seconds == 0 && errno) || config.stop_after_seconds < 1) {
                    fprintf(stderr, "stop-after-seconds must be greater or equal than 1, given: %s\n", optarg);
                    return false;
                }
                break;

            case '3':
                errno = 0;
                config.stop_after_splits = strtoul(optarg, NULL, 0);
                if ((config.stop_after_splits == 0 && errno) || config.stop_after_splits < 1) {
                    fprintf(stderr, "stop-after-splits must be greater or equal than 1, given: %s\n", optarg);
                    return false;
                }
                break;

            case 'h':
                print_usage(argv[0]);
                exit(0);
                break;

            case '?':
            default:
                print_usage(argv[0]);
                return false;
        }
    }

    if (argc == optind + 1) {
        config.output_filename = argv[optind++];
        const char *dot = strrchr(config.output_filename, '.');
        if (!dot || dot == config.output_filename) {
            config.output_extension = "bin";
        } else {
            int filename_length = strlen(config.output_filename) - strlen(dot + 1) - 1;
            asprintf(&config.output_filename, "%.*s", filename_length, config.output_filename);
            asprintf(&config.output_extension, "%s", dot + 1);
        }
    } else {
        fprintf(stderr, "File name missing\n");
        print_usage(argv[0]);
        return false;
    }

    if (config.verbose) {
        write_log("USB device: %04x:%04x\n", config.vendor_id, config.product_id);
        write_log("Latency: %u ms\n", config.latency);
        write_log("Transfer requests: %u\n", config.num_transfers);
        write_log("Maximum file size: %zu byte\n", config.max_filesize);
        write_log("Target frequency: %u Hz\n", config.frequency);
        write_log("Filename: %s | %s\n", config.output_filename, config.output_extension);
        write_log("Post-split script: %s\n", config.post_split_script ? config.post_split_script : "-");

        if (config.stop_after_bytes) {
            write_log("stop after %" PRIu64 " bytes\n", config.stop_after_bytes);
        }
        if (config.stop_after_seconds) {
            write_log("stop after %" PRIu64 " seconds\n", config.stop_after_seconds);
        }
        if (config.stop_after_splits) {
            write_log("stop after %" PRIu64 " file splits\n", config.stop_after_splits);
        }
    }

    return true;
}

/* Prints the command line options and usage information
 *
 */
void print_usage(const char *name) {
    fprintf(stderr, "Usage: %s [options] file\n", name);
    fprintf(stderr, "  Data will be written to a file with timestamp inserted into the file name.\n\n");
    fprintf(stderr, "  --verbose --silent          Toggles debug information output on and off.\n");
    fprintf(stderr, "  -v --vendor id              USB vendor ID, between 0x0 and 0xFFFF, hex or decimal.\n");
    fprintf(stderr, "  -p --product id             USB product ID, between 0x0 and 0xFFFF, hex or decimal.\n");
    fprintf(stderr,
            "  -l --latency value          Latency before the FIFO buffer is sent to the host, between 1 and 255.\n");
    fprintf(stderr, "  -t --transfers value        USB bulk transfer requests in the queue, between 1 and 4096.\n");
    fprintf(stderr,
            "  -s --split value            Split capture file after [value] bytes into a new file, use 0 to disable.\n");
    fprintf(stderr, "  -f --frequency value        Set ADC sampling frequency, between 300 and 50000 Hz.\n");
    fprintf(stderr,
            "  --post-split-script script  Execute [script] after file split occured. Arguments: file, frequency, and device type\n");
    fprintf(stderr, "  --stop-after-bytes value    Stops recording after [value] bytes have been written.\n");
    fprintf(stderr, "  --stop-after-seconds value  Stops recording after [value] seconds have elapsed.\n");
    fprintf(stderr, "  --stop-after-splits value   Stops recording after [value] file splits have occured.\n");
    fprintf(stderr,
            "  file                        Filename to write the captured data to. Existing files will be overwritten.\n");
}

/* The Main function
 *
 */
int main(int argc, char **argv) {
    errno = 0;
    if ((state.log_file = fopen("files/daq.log", "a")) == NULL) {
        fprintf(stderr, "Opening log file failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if (!parse_env()) {
        fprintf(stderr, "Parsing environment variables failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if (!parse_argv(argc, argv)) {
        fprintf(stderr, "Parsing command line arguments failed.\n");
        return EXIT_FAILURE;
    }

    if (!init_ftdi()) {
        return EXIT_FAILURE;
    }
    init_daq_interface();
    stop_capture(true);

    print_firmware_version();

    write_log("\n");
    write_log("Starting data capture...\n");

    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGCHLD, SIG_IGN);

    // TODO: Call Smart screen initialization
    int err = read_stream();
    free_daq_interface();

    stop_capture(false);

    if (err < 0 && !state.exit_requested) {
        write_log("ERROR: Capturing failed: %d\n", err);
        return EXIT_FAILURE;
    }

    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    time_t seconds = end_time.tv_sec - state.start_time.tv_sec;
    long nanoseconds = end_time.tv_nsec - state.start_time.tv_nsec;
    if (nanoseconds < 0) {
        seconds -= 1;
        nanoseconds = 1e9 + nanoseconds;
    }

    uint64_t packets_written = state.bytes_written / PACKET_LENGTH;
    double time_spent = seconds;
    time_spent += 1e-9 * nanoseconds;
    float effective_frequency = packets_written / time_spent;

    if (config.verbose) {
        write_log("\n\n");
        write_log("Capturing ended!\n");
        write_log("Written %llu bytes in %llu packets\n", state.bytes_written, packets_written);
        write_log("Timespan: %ld.%02d seconds\n", seconds, (int) (nanoseconds / 1e7));
        write_log("Effective sampling frequency: approx. %.2f Hz.\n", effective_frequency);
    }

    finish_file();

    ftdi_usb_close(state.ftdi);
    ftdi_free(state.ftdi);
    state.ftdi = NULL;

    signal(SIGINT, SIG_DFL);

    fclose(state.log_file);
    state.log_file = NULL;

    return EXIT_SUCCESS;
}
