#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

typedef enum {
    POWER_MAINS = 0,
    POWER_BATTERY = 1
} PowerStatus;

const char* power_status_str(uint8_t status) {
    return status == POWER_MAINS ? "zasilanie sieciowe" : "zasilanie bateryjne";
}

#pragma pack(push,1)
typedef struct {
    uint32_t timestamp;
    uint16_t temperature;    
    uint8_t power_status;
    uint8_t measurement_id;
    uint8_t checksum;
} SensorPacket;
#pragma pack(pop)

uint8_t calculate_checksum(const SensorPacket *packet) {
    const uint8_t *bytes = (const uint8_t*)packet;
    uint8_t sum = 0;
    for (size_t i = 0; i < sizeof(SensorPacket) - 1; i++) {
        sum += bytes[i];
    }
    return sum;
}

uint16_t random_temperature() {
    return (uint16_t)((rand() % (1200 - 200 + 1)) + 200);
}

uint8_t random_power_status() {
    return (uint8_t)(rand() % 2);
}

void fill_packet(SensorPacket *packet, uint8_t measurement_id) {
    packet->timestamp = (uint32_t)time(NULL);
    packet->temperature = random_temperature();
    packet->power_status = random_power_status();
    packet->measurement_id = measurement_id;
    packet->checksum = 0;
    packet->checksum = calculate_checksum(packet);
}

void log_packet(FILE *log_file, const SensorPacket *packet) {
    time_t ts = (time_t)packet->timestamp;
    struct tm *tm_info = localtime(&ts);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(log_file,
        "Czas: %s, Temp: %.1f°C, Zasilanie: %s, ID: %u, CRC: %u\n",
        time_str,
        packet->temperature / 10.0,
        power_status_str(packet->power_status),
        packet->measurement_id,
        packet->checksum);

    fflush(log_file);
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Użycie: %s <IP> <PORT> <CYKL_WYSLANIA> <CYKL_LOGOWANIA> <ŚCIEŻKA_LOGU>\n", argv[0]);
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);
    int send_cycle = atoi(argv[3]);
    int log_cycle = atoi(argv[4]);
    const char *log_path = argv[5];

    if (port <= 0 || port > 65535 || send_cycle <= 0 || log_cycle <= 0) {
        fprintf(stderr, "Niepoprawne parametry.\n");
        return 1;
    }

    FILE *log_file = fopen(log_path, "a");
    if (!log_file) {
        perror("Nie można otworzyć pliku logu");
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Błąd tworzenia socketu");
        fclose(log_file);
        return 1;
    }

    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "Niepoprawny adres IP\n");
        fclose(log_file);
        close(sockfd);
        return 1;
    }

    SensorPacket packet = {0};
    uint8_t measurement_id = 1;

    time_t last_send = 0;
    time_t last_log = 0;

    srand(time(NULL));

    while (1) {
        time_t now = time(NULL);

        if (now - last_send >= send_cycle) {
            fill_packet(&packet, measurement_id);

            ssize_t sent = sendto(sockfd, &packet, sizeof(packet), 0,
                                  (struct sockaddr*)&servaddr, sizeof(servaddr));
            if (sent != sizeof(packet)) {
                fprintf(stderr, "Błąd wysyłania pakietu: %s\n", strerror(errno));
            }

            measurement_id++;
            if (measurement_id == 0) measurement_id = 1; 

            last_send = now;
        }

        if (now - last_log >= log_cycle) {
            log_packet(log_file, &packet);
            last_log = now;
        }

        usleep(100 * 1000); 
    }

    fclose(log_file);
    close(sockfd);
    return 0;
}
