#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#define NODE_ID     125
#define SUBJECT_ID  3840
#define PRIORITY    16

static size_t encode_battery_info(uint8_t *buf) {
    size_t off = 0;
    uint32_t ts = 0;
    uint16_t voltage = 11100;
    int16_t current = -2500;
    int16_t temp = 2550;
    uint16_t rem = 8500, full = 10000, cyc = 12;
    uint16_t cells[8] = {3700, 3700, 3700, 0, 0, 0, 0, 0};

    memcpy(buf + off, &ts, 4); off += 4;
    buf[off++] = 0x03; // Status: зарядка
    buf[off++] = 0x00; // Health: OK
    memcpy(buf + off, &voltage, 2); off += 2;
    memcpy(buf + off, &current, 2); off += 2;
    memcpy(buf + off, &temp, 2); off += 2;
    memcpy(buf + off, &rem, 2); off += 2;
    memcpy(buf + off, &full, 2); off += 2;
    memcpy(buf + off, &cyc, 2); off += 2;
    memcpy(buf + off, cells, 16); off += 16;
    return off;
}

int main(void) {
    int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    struct ifreq ifr;
    struct sockaddr_can addr;
    uint8_t payload[64];
    size_t len = encode_battery_info(payload);
    uint8_t tid = 0;

    strncpy(ifr.ifr_name, "vcan0", IFNAMSIZ - 1);
    ioctl(sock, SIOCGIFINDEX, &ifr);
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));

    for (int i = 0; i < 5; i++) {
        size_t offset = 0;
        uint8_t toggle = 0;
        uint32_t can_id = ((PRIORITY & 0x1F) << 24) | ((SUBJECT_ID & 0xFFFF) << 8) | NODE_ID;

        while (offset < len) {
            struct can_frame frame;
            size_t chunk = (len - offset < 7) ? (len - offset) : 7;
            
            memset(&frame, 0, sizeof(frame));
            frame.can_id = can_id | CAN_EFF_FLAG;
            
            // UAVCAN Transfer Header: SOF, EOF, Toggle, TID
            uint8_t hdr = tid & 0x1F;
            if (offset == 0) hdr |= 0x80;
            if (offset + 7 >= len) hdr |= 0x40;
            hdr |= (toggle << 5);

            frame.data[0] = hdr;
            memcpy(&frame.data[1], &payload[offset], chunk);
            frame.can_dlc = 1 + chunk;

            write(sock, &frame, sizeof(frame));
            offset += chunk;
            toggle ^= 1;
        }
        printf("Sent BatteryInfo (TID=%d)\n", tid);
        tid = (tid + 1) & 0x1F;
        sleep(1);
    }
    close(sock);
    return 0;
}