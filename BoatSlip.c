#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Global constants
#define MAX_BOATS 120

// Enum to represent docking types
typedef enum { slip, land, trailer, storage} Docking;


// Union to store docking information
typedef union {
    int slip;
    char land;
    char *trailer;
    int storage;
} DockingInfo;


// Structure to store boat data
typedef struct {
    char name[127];
    float length;
    Docking type;
    DockingInfo info;
    float moneyOwed;
}Boat;


// Function Prototypes 
void addBoatFromInput(Boat **boats, int *count, char *input);
void loadBoatsFromFile(Boat **boats, int *count, const char *filename);
void saveBoatsToFile(Boat **boats, int count, const char *filename);
void freeBoats(Boat **boats, int count);
void printInventory(Boat **boats, int count);
void removeBoat(Boat **boats, int *count, const char *name);
void acceptPayment(Boat **boats, int count, const char *name, float amount);
void updateMonthlyCharges(Boat **boats, int count);
float calculateBoatCharge(Boat *boat);
void sortBoats(Boat **boats, int count);
Boat *findBoatByName(Boat **boats, int count, const char *name);



int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <BoatData.csv>\n", argv[0]);
        return 1;
    }
    Boat *boats[MAX_BOATS] = {NULL};
    int count = 0;
    loadBoatsFromFile(boats, &count, argv[1]);
    printf("Welcome to the Boat Management System\n");
    printf("-------------------------------------\n");
    char option;
    while (1) {
        printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        scanf(" %c", &option);
        if (strchr("iIarARpmPMxX", option) == NULL) {
            printf("Invalid option %c\n", option);  
            continue;  
        }
        option = tolower(option);  
        if (option == 'x') {
            printf("Exiting the Boat Management System\n");
            break;
        }
        switch (option) {
            case 'i':
                printInventory(boats, count);
                break;
            case 'a': {
                char input[256];
                printf("Please enter the boat data in CSV format                 : ");
                scanf(" %[^\n]", input); 
                addBoatFromInput(boats, &count, input);
                break;
            }
            case 'r': {
                char name[127];
                printf("Please enter the boat name                               : ");
                scanf(" %[^\n]", name);
                removeBoat(boats, &count, name);
                break;
            }
            case 'p': {
                char name[127];
                printf("Please enter the boat name                               : ");
                scanf(" %[^\n]", name);
                Boat *boat = findBoatByName(boats, count, name);
                if (boat) {
                    float amount;
                    printf("Please enter the amount to be paid                       : ");
                    scanf("%f", &amount);
                    if (amount > boat->moneyOwed) {
                        printf("That is more than the amount owed, $%.2f\n", boat->moneyOwed);
                    } else {
                        boat->moneyOwed -= amount;
                    }
                } else {
                    printf("No boat with that name\n");
                }
                break;
            }
            case 'm':
                updateMonthlyCharges(boats, count);
                break;
            default:
                printf("Invalid option %c\n", option);
                break;
        }
    }
    saveBoatsToFile(boats, count, argv[1]);
    freeBoats(boats, count);
    return 0;
}



// function to add a boat to the marina
void addBoatFromInput(Boat **boats, int *count, char *input) {
    if (*count >= MAX_BOATS) {
        fprintf(stderr, "Cannot add more boats. Marina is full.\n");
        return;
    }
    Boat *newBoat = malloc(sizeof(Boat));
    if (!newBoat) {
        fprintf(stderr, "Memory allocation failed.\n");
        return;
    }
    char typeStr[10], infoStr[100];
    sscanf(input, "%[^,],%f,%[^,],%[^,],%f",
           newBoat->name, &newBoat->length, typeStr, infoStr, &newBoat->moneyOwed);
    if (strcmp(typeStr, "slip") == 0) {
        newBoat->type = slip;
        newBoat->info.slip = atoi(infoStr);
    } else if (strcmp(typeStr, "land") == 0) {
        newBoat->type = land;
        sscanf(infoStr, " %c", &newBoat->info.land);
    } else if (strcmp(typeStr, "trailer") == 0) {
        newBoat->type = trailer;
        newBoat->info.trailer = malloc(strlen(infoStr) + 1);
        if (!newBoat->info.trailer) {
            fprintf(stderr, "Memory allocation for trailer tag failed.\n");
            free(newBoat);
            return;
        }
        strcpy(newBoat->info.trailer, infoStr);
    } else if (strcmp(typeStr, "storage") == 0) {
        newBoat->type = storage;
        newBoat->info.storage = atoi(infoStr);
    }
    boats[*count] = newBoat;
    (*count)++;
}


// function to load boats from a file
void loadBoatsFromFile(Boat **boats, int *count, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file){
        fprintf(stderr, "Could not open file %s for reading.\n", filename);
        exit(1);
    }
    char line[256];
    while (fgets(line, sizeof(line), file) && *count < MAX_BOATS ){
        Boat *newBoat = malloc(sizeof(Boat));
        if (!newBoat){
            fprintf(stderr, "Memory allocation failed.\n");
            fclose(file);
            freeBoats(boats, *count);
            exit(1);
        }
        char typeStr[10], infoStr[100];  
        sscanf(line, "%[^,],%f,%[^,],%[^,],%f",
               newBoat->name, &newBoat->length, typeStr, infoStr, &newBoat->moneyOwed);
        if (strcmp(typeStr, "slip") == 0) {
            newBoat->type = slip;
            newBoat->info.slip = atoi(infoStr);
        } else if (strcmp(typeStr, "land") == 0) {
            newBoat->type = land;
            sscanf(infoStr, " %c", &newBoat->info.land); 
        } else if (strcmp(typeStr, "trailer") == 0) {
            newBoat->type = trailer;
            newBoat->info.trailer = malloc(strlen(infoStr) + 1);
            if (!newBoat->info.trailer) {
                fprintf(stderr, "Memory allocation failed for trailer tag.\n");
                free(newBoat);  
                fclose(file);
                freeBoats(boats, *count);  
                exit(1);  
            }
            strcpy(newBoat->info.trailer, infoStr);  
        } else if (strcmp(typeStr, "storage") == 0) {
            newBoat->type = storage;
            newBoat->info.storage = atoi(infoStr);
        }
        boats[*count] = newBoat;  
        (*count)++;
    }
    fclose(file);
}



// function to save boats to a file
void saveBoatsToFile(Boat **boats, int count, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file){
        fprintf(stderr, "Could not open file %s for writing.\n", filename);
        exit(1);
    }
    for (int i=0; i < count; i++) {
        if (boats[i]){
            switch (boats[i] -> type) {
            case slip:
                fprintf(file, "%s,%.2f,slip,%d,%.2f\n",
                            boats[i]->name, boats[i]->length, boats[i]->info.slip, boats[i]->moneyOwed);
                break;
            case land:
                fprintf(file, "%s,%.2f,land,%c,%.2f\n",
                            boats[i]->name, boats[i]->length, boats[i]->info.land, boats[i]->moneyOwed);
                break;
            case trailer:
                fprintf(file, "%s,%.2f,trailer,%s,%.2f\n",
                            boats[i]->name, boats[i]->length, boats[i]->info.trailer, boats[i]->moneyOwed);
                break;
            case storage:
                fprintf(file, "%s,%.2f,storage,%d,%.2f\n",
                            boats[i]->name, boats[i]->length, boats[i]->info.storage, boats[i]->moneyOwed);
                break;
            }
        }
    }
    fclose(file);
}



// Free memory allocated for boats
void freeBoats(Boat **boats, int count) {
    for (int i = 0; i < count; i++) {
        if (boats[i]) {
            if (boats[i] -> type == trailer && boats[i] -> info.trailer) {
                free(boats[i] -> info.trailer);
            }
            free(boats[i]);
        }
    }
}



// function to print the inventory of boats, sorted by name
void printInventory(Boat **boats, int count) {
    sortBoats(boats, count);  
    for (int i = 0; i < count; i++) {
        if (boats[i]) {
            printf("%-20s %3.0f'  ", boats[i]->name, boats[i]->length);
            switch (boats[i]->type) {
                case slip:
                    printf("%-8s # %-6d", "slip", boats[i]->info.slip);
                    break;
                case land:
                    printf("%-8s   %-6c", "land", boats[i]->info.land);
                    break;
                case trailer:
                    printf("%-8s   %-6s", "trailer", boats[i]->info.trailer ? boats[i]->info.trailer : "N/A");
                    break;
                case storage:
                    printf("%-8s # %-6d", "storage", boats[i]->info.storage);
                    break;
            }
            printf("   Owes $%7.2f\n", boats[i]->moneyOwed);
        }
    }
}




// Function to remove a boat by name
void removeBoat(Boat **boats, int *count, const char *name) {
    for (int i = 0; i < *count; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            if (boats[i]->type == trailer && boats[i]->info.trailer) {
                free(boats[i]->info.trailer);
            }
            free(boats[i]);
            for (int j = i; j < *count - 1; j++) {
                boats[j] = boats[j + 1];
            }
            boats[*count - 1] = NULL;
            (*count)--;
            return;
        }
    }
    printf("No boat with that name\n");
}



// Function to accept payment for a boat
void acceptPayment(Boat **boats, int count, const char *name, float amount) {
    Boat *boat = findBoatByName(boats, count, name);  // Search for the boat by name
    if (boat) {
        if (amount > boat->moneyOwed) {
            printf("That is more than the amount owed, $%.2f\n", boat->moneyOwed);
        } else {
            boat->moneyOwed -= amount;
            printf("Payment accepted. Remaining balance: $%.2f\n", boat->moneyOwed);
        }
    } else {
        printf("No boat with that name\n");
    }
}



// Function to update monthly charges for all boats
void updateMonthlyCharges(Boat **boats, int count) {
    for (int i = 0; i < count; i++) {
        boats[i]->moneyOwed += calculateBoatCharge(boats[i]);
    }
}



// function to calculate boat charge
float calculateBoatCharge(Boat *boat) {
    switch (boat -> type) {
        case slip:
            return boat -> length * 12.50;
        case land:
            return boat -> length * 14.00;
        case trailer:
            return boat -> length * 25.00;
        case storage:
            return boat -> length * 11.20;
        default:
            return 0.0;
    }
}



// Function to sort boats alphabetically by name
void sortBoats(Boat **boats, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (strcasecmp(boats[i]->name, boats[j]->name) > 0) {
                Boat *temp = boats[i];
                boats[i] = boats[j];
                boats[j] = temp;
            }
        }
    }
}



// Function to find a boat by name
Boat *findBoatByName(Boat **boats, int count, const char *name) {
    for (int i = 0; i < count; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            return boats[i];
        }
    }
    return NULL;
}

