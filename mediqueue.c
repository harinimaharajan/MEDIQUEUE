
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PATIENTS 500
#define MAX_QUEUE 500
#define MAX_STACK 100

/* ================= PATIENT STRUCTURE ================= */

typedef struct {
    char patient_id[40];
    char name[100];
    int age;
    char arrival_date[20];
    char departure_date[20];
    char service[40];
    float satisfaction;
    int stay_days;
} Patient;

/* ================= LINKED LIST ================= */

typedef struct Node {
    Patient patient;
    struct Node *next;
} Node;

/* ================= QUEUE ================= */

typedef struct {
    Patient data[MAX_QUEUE];
    int front;
    int rear;
} Queue;

/* ================= PRIORITY QUEUE ================= */

typedef struct {
    Patient data[MAX_QUEUE];
    int priority[MAX_QUEUE];
    int size;
} PriorityQueue;

/* ================= STACK ================= */

typedef struct {
    char activities[MAX_STACK][150];
    int top;
} Stack;

/* ================= GLOBALS ================= */

Patient patients[MAX_PATIENTS];
int patientCount = 0;

Node *head = NULL;

Queue normalQueue;
PriorityQueue emergencyQueue;
Stack activityStack;

/* ================= UTILITY ================= */

void removeNewline(char *str) {
    str[strcspn(str, "\r\n")] = '\0';
}

int equalsIgnoreCase(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

int containsIgnoreCase(const char *text, const char *pattern) {
    char t[150], p[100];

    snprintf(t, sizeof(t), "%s", text);
    snprintf(p, sizeof(p), "%s", pattern);

    for (int i = 0; t[i]; i++)
        t[i] = (char)tolower((unsigned char)t[i]);

    for (int i = 0; p[i]; i++)
        p[i] = (char)tolower((unsigned char)p[i]);

    return strstr(t, p) != NULL;
}

/* ================= DATE FUNCTIONS ================= */

int isLeapYear(int year) {
    return (year % 400 == 0) ||
           (year % 100 != 0 && year % 4 == 0);
}

int dateToDays(const char *date) {
    int year, month, day;

    if (sscanf(date, "%d-%d-%d", &year, &month, &day) != 3)
        return 0;

    int days = 0;

    for (int y = 1970; y < year; y++) {
        days += isLeapYear(y) ? 366 : 365;
    }

    int monthDays[] = {
        31,28,31,30,31,30,
        31,31,30,31,30,31
    };

    for (int m = 1; m < month; m++) {
        days += monthDays[m - 1];

        if (m == 2 && isLeapYear(year))
            days++;
    }

    days += day;

    return days;
}

int calculateStayDays(const char *arrival, const char *departure) {
    int start = dateToDays(arrival);
    int end = dateToDays(departure);

    int stay = end - start;

    if (stay < 0)
        stay = 0;

    return stay;
}

/* ================= ACTIVITY STACK ================= */

void pushActivity(const char *activity) {
    if (activityStack.top < MAX_STACK - 1) {
        activityStack.top++;
        strcpy(activityStack.activities[activityStack.top], activity);
    }
}

void displayActivityHistory() {
    printf("\n============================================\n");
    printf("          ACTIVITY HISTORY\n");
    printf("============================================\n");

    if (activityStack.top == -1) {
        printf("No activities recorded.\n");
        return;
    }

    for (int i = activityStack.top; i >= 0; i--) {
        printf("%d. %s\n",
               activityStack.top - i + 1,
               activityStack.activities[i]);
    }
}

/* ================= LINKED LIST ================= */

void insertLinkedList(Patient p) {
    Node *newNode = (Node *)malloc(sizeof(Node));

    if (newNode == NULL)
        return;

    newNode->patient = p;
    newNode->next = NULL;

    if (head == NULL) {
        head = newNode;
    } else {
        Node *temp = head;

        while (temp->next != NULL)
            temp = temp->next;

        temp->next = newNode;
    }
}

/* ================= NORMAL QUEUE ================= */

void initQueue() {
    normalQueue.front = 0;
    normalQueue.rear = -1;
}

void enqueue(Patient p) {
    if (normalQueue.rear < MAX_QUEUE - 1) {
        normalQueue.rear++;
        normalQueue.data[normalQueue.rear] = p;
    }
}

void displayQueue() {
    printf("\n============================================\n");
    printf("             NORMAL PATIENT QUEUE\n");
    printf("============================================\n");

    if (normalQueue.rear < normalQueue.front) {
        printf("Queue is empty.\n");
        return;
    }

    for (int i = normalQueue.front; i <= normalQueue.rear; i++) {
        printf("%d. %s - %s\n",
               i - normalQueue.front + 1,
               normalQueue.data[i].patient_id,
               normalQueue.data[i].name);
    }
}

/* ================= PRIORITY QUEUE ================= */

int getPriority(const char *service) {

    if (equalsIgnoreCase(service, "emergency"))
        return 5;

    if (equalsIgnoreCase(service, "icu"))
        return 4;

    if (equalsIgnoreCase(service, "surgery"))
        return 3;

    if (equalsIgnoreCase(service, "cardiology"))
        return 2;

    return 1;
}

void initPriorityQueue() {
    emergencyQueue.size = 0;
}

void insertPriorityQueue(Patient p) {

    if (emergencyQueue.size >= MAX_QUEUE)
        return;

    int pos = emergencyQueue.size;
    int pValue = getPriority(p.service);

    while (pos > 0 &&
           emergencyQueue.priority[pos - 1] < pValue) {

        emergencyQueue.data[pos] =
            emergencyQueue.data[pos - 1];

        emergencyQueue.priority[pos] =
            emergencyQueue.priority[pos - 1];

        pos--;
    }

    emergencyQueue.data[pos] = p;
    emergencyQueue.priority[pos] = pValue;

    emergencyQueue.size++;
}

void buildEmergencyQueue() {

    initPriorityQueue();

    for (int i = 0; i < patientCount; i++) {

        if (getPriority(patients[i].service) >= 3) {
            insertPriorityQueue(patients[i]);
        }
    }
}

void displayEmergencyQueue() {

    buildEmergencyQueue();

    printf("\n============================================\n");
    printf("             EMERGENCY PRIORITY QUEUE\n");
    printf("============================================\n");

    if (emergencyQueue.size == 0) {
        printf("No emergency / critical patients.\n");
        return;
    }

    for (int i = 0; i < emergencyQueue.size; i++) {

        printf("%d. %s | %s | %s | Priority: %d\n",
               i + 1,
               emergencyQueue.data[i].patient_id,
               emergencyQueue.data[i].name,
               emergencyQueue.data[i].service,
               emergencyQueue.priority[i]);
    }
}

/* ================= CSV LOADING ================= */

void loadPatients() {

    FILE *file = fopen("patients.csv", "r");

    if (file == NULL) {
        printf("\nERROR: patients.csv not found!\n");
        printf("Make sure patients.csv is inside the MEDIQUEUE folder.\n");
        return;
    }

    char line[500];

    /* Skip header */
    fgets(line, sizeof(line), file);

    patientCount = 0;

    while (fgets(line, sizeof(line), file) &&
           patientCount < MAX_PATIENTS) {

        removeNewline(line);

        Patient p;

        char *token;

        token = strtok(line, ",");
        if (!token) continue;
        strcpy(p.patient_id, token);

        token = strtok(NULL, ",");
        if (!token) continue;
        strcpy(p.name, token);

        token = strtok(NULL, ",");
        if (!token) continue;
        p.age = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        strcpy(p.arrival_date, token);

        token = strtok(NULL, ",");
        if (!token) continue;
        strcpy(p.departure_date, token);

        token = strtok(NULL, ",");
        if (!token) continue;
        strcpy(p.service, token);

        token = strtok(NULL, ",");
        if (!token) continue;
        p.satisfaction = (float)atof(token);

        p.stay_days =
            calculateStayDays(
                p.arrival_date,
                p.departure_date
            );

        patients[patientCount] = p;

        insertLinkedList(p);

        /* General patients go into normal queue */
        if (getPriority(p.service) < 3) {
            enqueue(p);
        }

        patientCount++;
    }

    fclose(file);

    printf("\nPatient data loaded successfully!\n");
    printf("Total records loaded: %d\n", patientCount);
}

/* ================= PATIENT MANAGEMENT ================= */

void displayPatients() {

    printf("\n============================================================\n");
    printf("                 PATIENT MANAGEMENT\n");
    printf("============================================================\n");

    if (patientCount == 0) {
        printf("No patient records found.\n");
        return;
    }

    for (int i = 0; i < patientCount; i++) {

        printf("\nPatient ID      : %s", patients[i].patient_id);
        printf("\nName            : %s", patients[i].name);
        printf("\nAge             : %d", patients[i].age);
        printf("\nArrival Date    : %s", patients[i].arrival_date);
        printf("\nDeparture Date  : %s", patients[i].departure_date);
        printf("\nService         : %s", patients[i].service);
        printf("\nSatisfaction    : %.1f", patients[i].satisfaction);
        printf("\nStay Days       : %d", patients[i].stay_days);
        printf("\n--------------------------------------------\n");
    }

    pushActivity("Patient records viewed");
}

/* ================= SEARCH ================= */

void searchPatient() {

    int choice;
    char searchValue[100];

    printf("\n============================================\n");
    printf("             SEARCH PATIENT\n");
    printf("============================================\n");

    printf("1. Search by Patient ID\n");
    printf("2. Search by Name\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    getchar();

    printf("Enter search value: ");
    fgets(searchValue, sizeof(searchValue), stdin);
    removeNewline(searchValue);

    int found = 0;

    for (int i = 0; i < patientCount; i++) {

        int match = 0;

        if (choice == 1) {
            match = equalsIgnoreCase(
                patients[i].patient_id,
                searchValue
            );
        }
        else if (choice == 2) {
            match = containsIgnoreCase(
                patients[i].name,
                searchValue
            );
        }

        if (match) {

            printf("\nPatient Found!\n");
            printf("Patient ID     : %s\n",
                   patients[i].patient_id);
            printf("Name           : %s\n",
                   patients[i].name);
            printf("Age            : %d\n",
                   patients[i].age);
            printf("Service        : %s\n",
                   patients[i].service);
            printf("Stay Days      : %d\n",
                   patients[i].stay_days);
            printf("Satisfaction   : %.1f\n",
                   patients[i].satisfaction);

            found = 1;
        }
    }

    if (!found)
        printf("\nPatient not found.\n");

    pushActivity("Patient search performed");
}

/* ================= SMART BED ALLOCATION ================= */

void smartBedAllocation() {

    int age;
    int choice;

    printf("\n============================================\n");
    printf("             SMART BED ALLOCATION\n");
    printf("============================================\n");

    printf("Enter patient age: ");
    scanf("%d", &age);

    printf("\nSelect Service:\n");
    printf("1. General Medicine\n");
    printf("2. Cardiology\n");
    printf("3. ICU\n");
    printf("4. Surgery\n");

    printf("Enter choice: ");
    scanf("%d", &choice);

    char service[40];
    char bed[50];
    float predictedStay;

    if (choice == 1) {
        strcpy(service, "General Medicine");
        strcpy(bed, "General Ward Bed");
        predictedStay = age > 60 ? 5.5 : 3.0;
    }
    else if (choice == 2) {
        strcpy(service, "Cardiology");
        strcpy(bed, "Cardiac Care Bed");
        predictedStay = age > 60 ? 7.0 : 5.0;
    }
    else if (choice == 3) {
        strcpy(service, "ICU");
        strcpy(bed, "Critical Care Bed");
        predictedStay = age > 60 ? 8.5 : 6.0;
    }
    else if (choice == 4) {
        strcpy(service, "Surgery");
        strcpy(bed, "Surgical Ward Bed");
        predictedStay = age > 60 ? 8.0 : 5.5;
    }
    else {
        printf("\nInvalid service choice.\n");
        return;
    }

    printf("\n----------- ALLOCATION RESULT -----------\n");
    printf("Service          : %s\n", service);
    printf("Predicted Stay   : %.1f days\n", predictedStay);
    printf("Allocated Bed    : %s\n", bed);
    printf("Allocation Status: SUCCESS\n");

    pushActivity("Smart bed allocation performed");
}

/* ================= DOCTOR MANAGEMENT ================= */

typedef struct {
    int id;
    char name[60];
    char department[50];
    char status[30];
} Doctor;

Doctor doctors[] = {

    {101, "Dr. Arun", "Cardiology", "Available"},
    {102, "Dr. Priya", "Emergency", "Available"},
    {103, "Dr. Kumar", "General Medicine", "Busy"},
    {104, "Dr. Divya", "Surgery", "Available"}
};

void doctorManagement() {

    int count =
        sizeof(doctors) / sizeof(doctors[0]);

    printf("\n============================================\n");
    printf("             DOCTOR MANAGEMENT\n");
    printf("============================================\n");

    printf("%-8s %-20s %-20s %-15s\n",
           "ID", "Doctor", "Department", "Status");

    printf("------------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {

        printf("%-8d %-20s %-20s %-15s\n",
               doctors[i].id,
               doctors[i].name,
               doctors[i].department,
               doctors[i].status);
    }

    pushActivity("Doctor management viewed");
}

/* ================= QUEUE ANALYTICS ================= */

void queueAnalytics() {

    if (patientCount == 0) {
        printf("\nNo data available.\n");
        return;
    }

    float totalAge = 0;
    float totalSatisfaction = 0;
    float totalStay = 0;

    int emergency = 0;
    int icu = 0;

    for (int i = 0; i < patientCount; i++) {

        totalAge += patients[i].age;
        totalSatisfaction += patients[i].satisfaction;
        totalStay += patients[i].stay_days;

        if (equalsIgnoreCase(
                patients[i].service,
                "emergency"))
            emergency++;

        if (equalsIgnoreCase(
                patients[i].service,
                "icu"))
            icu++;
    }

    printf("\n============================================\n");
    printf("              QUEUE ANALYTICS\n");
    printf("============================================\n");

    printf("Patient Records       : %d\n",
           patientCount);

    printf("Emergency Cases       : %d\n",
           emergency);

    printf("ICU Cases             : %d\n",
           icu);

    printf("Average Age           : %.1f\n",
           totalAge / patientCount);

    printf("Average Satisfaction   : %.1f\n",
           totalSatisfaction / patientCount);

    printf("Average Stay          : %.1f days\n",
           totalStay / patientCount);

    pushActivity("Queue analytics viewed");
}

/* ================= MISSION DASHBOARD ================= */

void missionDashboard() {

    if (patientCount == 0) {
        printf("\nNo patient data available.\n");
        return;
    }

    int emergency = 0;
    int icu = 0;
    float totalStay = 0;

    for (int i = 0; i < patientCount; i++) {

        if (equalsIgnoreCase(
                patients[i].service,
                "emergency"))
            emergency++;

        if (equalsIgnoreCase(
                patients[i].service,
                "icu"))
            icu++;

        totalStay += patients[i].stay_days;
    }

    printf("\n============================================\n");
    printf("              MEDIQUEUE\n");
    printf("           MISSION DASHBOARD\n");
    printf("============================================\n");

    printf("\nTotal Patients       : %d\n",
           patientCount);

    printf("Emergency Cases      : %d\n",
           emergency);

    printf("ICU Cases            : %d\n",
           icu);

    printf("Average Stay         : %.1f days\n",
           totalStay / patientCount);

    printf("Smart Allocation     : ONLINE\n");

    printf("\n--------------------------------------------\n");
    printf("Prioritize Care. Optimize Resources.\n");
    printf("--------------------------------------------\n");

    pushActivity("Mission dashboard viewed");
}

/* ================= MAIN MENU ================= */

void displayMenu() {

    printf("\n\n============================================\n");
    printf("              MEDIQUEUE SYSTEM\n");
    printf("============================================\n");

    printf("1. Mission Dashboard\n");
    printf("2. Patient Management\n");
    printf("3. Search Patient\n");
    printf("4. Emergency Priority Queue\n");
    printf("5. Normal Patient Queue\n");
    printf("6. Smart Bed Allocation\n");
    printf("7. Doctor Management\n");
    printf("8. Queue Analytics\n");
    printf("9. Activity History\n");
    printf("0. Exit\n");

    printf("============================================\n");
    printf("Enter your choice: ");
}

/* ================= MAIN ================= */

int main() {

    initQueue();
    initPriorityQueue();

    activityStack.top = -1;

    loadPatients();

    pushActivity("MEDIQUEUE system started");

    int choice;

    do {

        displayMenu();
        scanf("%d", &choice);

        switch (choice) {

            case 1:
                missionDashboard();
                break;

            case 2:
                displayPatients();
                break;

            case 3:
                searchPatient();
                break;

            case 4:
                displayEmergencyQueue();
                break;

            case 5:
                displayQueue();
                break;

            case 6:
                smartBedAllocation();
                break;

            case 7:
                doctorManagement();
                break;

            case 8:
                queueAnalytics();
                break;

            case 9:
                displayActivityHistory();
                break;

            case 0:
                printf("\nThank you for using MEDIQUEUE!\n");
                break;

            default:
                printf("\nInvalid choice. Please try again.\n");
        }

    } while (choice != 0);

    return 0;
}