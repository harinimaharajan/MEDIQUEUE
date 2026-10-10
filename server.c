#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define MAX_PATIENTS 500

typedef struct
{
    char patient_id[40];
    char name[100];
    int age;
    char arrival_date[20];
    char departure_date[20];
    char service[40];
    float satisfaction;
    int stay_days;
} Patient;

Patient patients[MAX_PATIENTS];
int patient_count = 0;

/* -------------------------------------------------- */
/* Utility Functions                                  */
/* -------------------------------------------------- */

int is_emergency(const char *service)
{
    if (_stricmp(service, "emergency") == 0)
        return 1;

    if (_stricmp(service, "icu") == 0)
        return 1;

    if (_stricmp(service, "surgery") == 0)
        return 1;

    return 0;
}

int get_priority(const char *service)
{
    if (_stricmp(service, "emergency") == 0)
        return 5;

    if (_stricmp(service, "icu") == 0)
        return 4;

    if (_stricmp(service, "surgery") == 0)
        return 3;

    if (_stricmp(service, "cardiology") == 0)
        return 2;

    return 1;
}

/* -------------------------------------------------- */
/* Date Difference                                    */
/* -------------------------------------------------- */

int is_leap_year(int year)
{
    return ((year % 4 == 0 && year % 100 != 0) ||
            (year % 400 == 0));
}

int days_before_year(int year)
{
    int y;
    int days = 0;

    for (y = 1; y < year; y++)
    {
        days += is_leap_year(y) ? 366 : 365;
    }

    return days;
}

int days_before_month(int year, int month)
{
    int days_in_month[] =
        {31, 28, 31, 30, 31, 30,
         31, 31, 30, 31, 30, 31};

    int i;
    int days = 0;

    for (i = 0; i < month - 1; i++)
    {
        days += days_in_month[i];
    }

    if (month > 2 && is_leap_year(year))
        days++;

    return days;
}

int date_to_days(const char *date)
{
    int year, month, day;

    if (sscanf(date, "%d-%d-%d",
               &year, &month, &day) != 3)
    {
        return 0;
    }

    return days_before_year(year) +
           days_before_month(year, month) +
           day;
}

int calculate_stay(const char *arrival,
                   const char *departure)
{
    int a = date_to_days(arrival);
    int d = date_to_days(departure);

    if (d >= a)
        return d - a;

    return 0;
}

/* -------------------------------------------------- */
/* Load CSV                                           */
/* -------------------------------------------------- */

void load_patients()
{
    FILE *file;
    char line[500];

    file = fopen("patients.csv", "r");

    if (file == NULL)
    {
        printf("Error: patients.csv not found!\n");
        return;
    }

    /* Skip header */
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file) != NULL &&
           patient_count < MAX_PATIENTS)
    {
        char *token;

        token = strtok(line, ",");
        if (token == NULL)
            continue;

        strcpy(patients[patient_count].patient_id, token);

        token = strtok(NULL, ",");
        if (token == NULL)
            continue;

        strcpy(patients[patient_count].name, token);

        token = strtok(NULL, ",");
        if (token == NULL)
            continue;

        patients[patient_count].age = atoi(token);

        token = strtok(NULL, ",");
        if (token == NULL)
            continue;

        strcpy(patients[patient_count].arrival_date, token);

        token = strtok(NULL, ",");
        if (token == NULL)
            continue;

        strcpy(patients[patient_count].departure_date, token);

        token = strtok(NULL, ",");
        if (token == NULL)
            continue;

        strcpy(patients[patient_count].service, token);

        token = strtok(NULL, ",");
        if (token == NULL)
            continue;

        patients[patient_count].satisfaction =
            (float)atof(token);

        patients[patient_count].stay_days =
            calculate_stay(
                patients[patient_count].arrival_date,
                patients[patient_count].departure_date
            );

        patient_count++;
    }

    fclose(file);
}

/* -------------------------------------------------- */
/* Send Complete Data                                 */
/* -------------------------------------------------- */

void send_all(SOCKET client,
              const char *data,
              int length)
{
    int total = 0;

    while (total < length)
    {
        int sent = send(
            client,
            data + total,
            length - total,
            0
        );

        if (sent <= 0)
            break;

        total += sent;
    }
}

/* -------------------------------------------------- */
/* Send Text Response                                 */
/* -------------------------------------------------- */

void send_text(SOCKET client,
               const char *status,
               const char *content_type,
               const char *body)
{
    char header[1000];

    int body_length = (int)strlen(body);

    sprintf(
        header,
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n",
        status,
        content_type,
        body_length
    );

    send_all(client, header, strlen(header));
    send_all(client, body, body_length);
}

/* -------------------------------------------------- */
/* Send File                                          */
/* -------------------------------------------------- */

void send_file(SOCKET client,
               const char *filename,
               const char *content_type)
{
    FILE *file;
    long file_size;
    char *buffer;
    char header[1000];

    file = fopen(filename, "rb");

    if (file == NULL)
    {
        send_text(
            client,
            "404 Not Found",
            "text/plain",
            "File not found"
        );
        return;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    buffer = (char *)malloc(file_size);

    if (buffer == NULL)
    {
        fclose(file);

        send_text(
            client,
            "500 Internal Server Error",
            "text/plain",
            "Memory allocation failed"
        );

        return;
    }

    fread(buffer, 1, file_size, file);
    fclose(file);

    sprintf(
        header,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n",
        content_type,
        file_size
    );

    send_all(client, header, strlen(header));
    send_all(client, buffer, file_size);

    free(buffer);
}

/* -------------------------------------------------- */
/* Statistics API                                     */
/* -------------------------------------------------- */

void send_stats(SOCKET client)
{
    int i;

    int emergency = 0;
    int icu = 0;
    int normal = 0;

    float total_age = 0;
    float total_satisfaction = 0;
    float total_stay = 0;

    for (i = 0; i < patient_count; i++)
    {
        if (is_emergency(patients[i].service))
            emergency++;

        if (_stricmp(patients[i].service, "icu") == 0)
            icu++;

        if (get_priority(patients[i].service) < 3)
            normal++;

        total_age += patients[i].age;
        total_satisfaction += patients[i].satisfaction;
        total_stay += patients[i].stay_days;
    }

    float average_age =
        patient_count > 0
            ? total_age / patient_count
            : 0;

    float average_satisfaction =
        patient_count > 0
            ? total_satisfaction / patient_count
            : 0;

    float average_stay =
        patient_count > 0
            ? total_stay / patient_count
            : 0;

    char json[5000];

    sprintf(
        json,
        "{"
        "\"total_patients\":%d,"
        "\"emergency_cases\":%d,"
        "\"icu_cases\":%d,"
        "\"normal_queue\":%d,"
        "\"average_age\":%.1f,"
        "\"average_satisfaction\":%.1f,"
        "\"average_stay\":%.1f"
        "}",
        patient_count,
        emergency,
        icu,
        normal,
        average_age,
        average_satisfaction,
        average_stay
    );

    send_text(
        client,
        "200 OK",
        "application/json",
        json
    );
}

/* -------------------------------------------------- */
/* Search Patient API                                 */
/* -------------------------------------------------- */

void send_search(SOCKET client,
                 const char *patient_id)
{
    int i;

    for (i = 0; i < patient_count; i++)
    {
        if (_stricmp(
                patients[i].patient_id,
                patient_id
            ) == 0)
        {
            char json[2000];

            sprintf(
                json,
                "{"
                "\"found\":true,"
                "\"patient_id\":\"%s\","
                "\"name\":\"%s\","
                "\"age\":%d,"
                "\"arrival_date\":\"%s\","
                "\"departure_date\":\"%s\","
                "\"service\":\"%s\","
                "\"satisfaction\":%.1f,"
                "\"stay_days\":%d"
                "}",
                patients[i].patient_id,
                patients[i].name,
                patients[i].age,
                patients[i].arrival_date,
                patients[i].departure_date,
                patients[i].service,
                patients[i].satisfaction,
                patients[i].stay_days
            );

            send_text(
                client,
                "200 OK",
                "application/json",
                json
            );

            return;
        }
    }

    send_text(
        client,
        "200 OK",
        "application/json",
        "{\"found\":false}"
    );
}

/* -------------------------------------------------- */
/* Emergency Queue API                                */
/* -------------------------------------------------- */

void send_emergency(SOCKET client)
{
    char json[30000];
    int i;
    int first = 1;

    strcpy(json, "[");

    for (i = 0; i < patient_count; i++)
    {
        if (is_emergency(patients[i].service))
        {
            char item[500];

            sprintf(
                item,
                "%s{"
                "\"patient_id\":\"%s\","
                "\"name\":\"%s\","
                "\"age\":%d,"
                "\"service\":\"%s\","
                "\"priority\":%d"
                "}",
                first ? "" : ",",
                patients[i].patient_id,
                patients[i].name,
                patients[i].age,
                patients[i].service,
                get_priority(patients[i].service)
            );

            strcat(json, item);

            first = 0;
        }
    }

    strcat(json, "]");

    send_text(
        client,
        "200 OK",
        "application/json",
        json
    );
}

/* -------------------------------------------------- */
/* Smart Bed Allocation API                            */
/* -------------------------------------------------- */

void send_bed_allocation(SOCKET client,
                         const char *service)
{
    char bed[100];
    char status[] = "Allocated";
    char json[500];

    if (_stricmp(service, "general medicine") == 0)
    {
        strcpy(bed, "General Ward Bed");
    }
    else if (_stricmp(service, "cardiology") == 0)
    {
        strcpy(bed, "Cardiac Care Bed");
    }
    else if (_stricmp(service, "icu") == 0)
    {
        strcpy(bed, "Critical Care Bed");
    }
    else if (_stricmp(service, "surgery") == 0)
    {
        strcpy(bed, "Surgical Ward Bed");
    }
    else if (_stricmp(service, "emergency") == 0)
    {
        strcpy(bed, "Emergency Care Bed");
    }
    else
    {
        strcpy(bed, "General Ward Bed");
    }

    sprintf(
        json,
        "{"
        "\"service\":\"%s\","
        "\"bed\":\"%s\","
        "\"status\":\"%s\""
        "}",
        service,
        bed,
        status
    );

    send_text(
        client,
        "200 OK",
        "application/json",
        json
    );
}

/* -------------------------------------------------- */
/* Handle HTTP Request                                */
/* -------------------------------------------------- */

void handle_request(SOCKET client,
                    const char *request)
{
    /* Home page */
    if (strstr(request, "GET / ") != NULL)
    {
        send_file(
            client,
            "frontend/index.html",
            "text/html"
        );
    }

    /* CSS */
    else if (strstr(request, "GET /style.css") != NULL)
    {
        send_file(
            client,
            "frontend/style.css",
            "text/css"
        );
    }

    /* JavaScript */
    else if (strstr(request, "GET /script.js") != NULL)
    {
        send_file(
            client,
            "frontend/script.js",
            "application/javascript"
        );
    }

    /* Background image */
    else if (
        strstr(
            request,
            "GET /images/background_image.jpg"
        ) != NULL
    )
    {
        send_file(
            client,
            "images/background_image.jpg",
            "image/jpeg"
        );
    }

    /* Statistics */
    else if (
        strstr(request, "GET /api/stats") != NULL
    )
    {
        send_stats(client);
    }

    /* Search Patient */
    else if (
        strstr(request, "GET /api/search?id=") != NULL
    )
    {
        char patient_id[100];

        const char *start =
            strstr(request, "GET /api/search?id=");

        start += strlen("GET /api/search?id=");

        int i = 0;

        while (
            start[i] != ' ' &&
            start[i] != '&' &&
            start[i] != '\0' &&
            i < 99
        )
        {
            patient_id[i] = start[i];
            i++;
        }

        patient_id[i] = '\0';

        send_search(
            client,
            patient_id
        );
    }

    /* Emergency Queue */
    else if (
        strstr(request, "GET /api/emergency") != NULL
    )
    {
        send_emergency(client);
    }

    /* Bed Allocation */
    else if (
        strstr(request, "GET /api/bed?service=") != NULL
    )
    {
        char service[100];

        const char *start =
            strstr(request, "GET /api/bed?service=");

        start += strlen("GET /api/bed?service=");

        int i = 0;

        while (
            start[i] != ' ' &&
            start[i] != '&' &&
            start[i] != '\0' &&
            i < 99
        )
        {
            service[i] = start[i];
            i++;
        }

        service[i] = '\0';

        send_bed_allocation(
            client,
            service
        );
    }

    /* Unknown request */
    else
    {
        send_text(
            client,
            "404 Not Found",
            "text/plain",
            "MEDIQUEUE API - Resource Not Found"
        );
    }
}

/* -------------------------------------------------- */
/* Main Server                                        */
/* -------------------------------------------------- */

int main()
{
    WSADATA wsa;
    SOCKET server_socket;
    SOCKET client_socket;

    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    int client_length;
    char request[8192];

    printf("=====================================\n");
    printf("       MEDIQUEUE C SERVER\n");
    printf("=====================================\n");

    /* Start Winsock */
    if (WSAStartup(
            MAKEWORD(2, 2),
            &wsa
        ) != 0)
    {
        printf("Winsock initialization failed.\n");
        return 1;
    }

    /* Load CSV */
    load_patients();

    printf(
        "Patients loaded : %d\n",
        patient_count
    );

    /* Create socket */
    server_socket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (server_socket == INVALID_SOCKET)
    {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    /* Server address */
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    /* Bind */
    if (bind(
            server_socket,
            (struct sockaddr *)&server_address,
            sizeof(server_address)
        ) == SOCKET_ERROR)
    {
        printf("Bind failed.\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    /* Listen */
    if (listen(server_socket, 10) == SOCKET_ERROR)
    {
        printf("Listen failed.\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Server running at:\n");
    printf("http://localhost:8080\n");
    printf("=====================================\n");

    while (1)
    {
        client_length =
            sizeof(client_address);

        client_socket = accept(
            server_socket,
            (struct sockaddr *)&client_address,
            &client_length
        );

        if (client_socket == INVALID_SOCKET)
        {
            continue;
        }

        memset(
            request,
            0,
            sizeof(request)
        );

        recv(
            client_socket,
            request,
            sizeof(request) - 1,
            0
        );

        handle_request(
            client_socket,
            request
        );

        closesocket(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();

    return 0;
}