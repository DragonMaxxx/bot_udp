#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

// Constant
#define POWER_NETWORK 0
#define POWER_BATTERY 1

// Packet
typedef struct {
    uint32_t timestamp;
    uint16_t temperature;      
    uint8_t power_status;      
    uint8_t measurement_id;
    uint8_t checksum;
} SensorPacket;

// Functions
void print_usage(const char *progname);
uint8_t calculate_checksum(SensorPacket *packet);
void log_packet(const SensorPacket *packet, FILE *log_file);
SensorPacket generate_packet(uint8_t measurement_id);
void send_packet(int sock, struct sockaddr_in *server_addr, SensorPacket *packet);
void parse_args(int argc, char *argv[], char **ip, int *port, int *send_interval, int *log_interval, char **log_path);


void print_usage(const char *progname) {
    printf("Użycie: %s <IP> <PORT> <CYKL_WYSLANIA_S> <CYKL_LOGOWANIA_S> <SCIEZKA_LOGU>\n", progname);
}

uint8_t calculate_checksum(SensorPacket *packet) {
    uint8_t *data = (uint8_t *)packet;
    uint8_t sum = 0;
    for (size_t i = 0; i < sizeof(SensorPacket) - 1; i++) {
        sum += data[i];
    }
    return sum;
}

void log_packet(const SensorPacket *packet, FILE *log_file) {
    const char *power_str = packet->power_status == POWER_NETWORK ? "zasilanie sieciowe" : "zasilanie bateryjne";
    float temp = packet->temperature / 10.0f;
    fprintf(log_file, "Czas: %u, Temp: %.1f°C, Zasilanie: %s, ID: %u, CRC: %u\n",
        packet->timestamp, temp, power_str, packet->measurement_id, packet->checksum);
    fflush(log_file);
}

SensorPacket generate_packet(uint8_t measurement_id) {
    SensorPacket packet;
    packet.timestamp = (uint32_t)time(NULL);
    packet.temperature = (rand() % 1001) + 200; 
    packet.power_status = rand() % 2; 
    packet.measurement_id = measurement_id;
    packet.checksum = 0;
    packet.checksum = calculate_checksum(&packet);
    return packet;
}

void send_packet(int sock, struct sockaddr_in *server_addr, SensorPacket *packet) {
    sendto(sock, packet, sizeof(SensorPacket), 0,
           (struct sockaddr *)server_addr, sizeof(*server_addr));
}

void parse_args(int argc, char *argv[], char **ip, int *port, int *send_interval, int *log_interval, char **log_path) {
    if (argc != 6) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    *ip = argv[1];
    *port = atoi(argv[2]);
    *send_interval = atoi(argv[3]);
    *log_interval = atoi(argv[4]);
    *log_path = argv[5];
}

int main(int argc, char *argv[]) {
    char *ip, *log_path;
    int port, send_interval, log_interval;

    parse_args(argc, argv, &ip, &port, &send_interval, &log_interval, &log_path);

    FILE *log_file = fopen(log_path, "a");
    if (!log_file) {
        perror("Nie można otworzyć pliku logu");
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Błąd tworzenia gniazda");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    uint8_t measurement_id = 1;
    time_t last_log = 0;

    while (1) {
        SensorPacket packet = generate_packet(measurement_id++);
        send_packet(sock, &server_addr, &packet);

        time_t now = time(NULL);
        if (now - last_log >= log_interval) {
            log_packet(&packet, log_file);
            last_log = now;
        }

        sleep(send_interval);
    }

    fclose(log_file);
    close(sock);
    return 0;
}
