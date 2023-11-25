#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <openssl/rand.h>

#define FILENAME "password_prefs.txt"
#define HISTORY_FILENAME "password_history.txt"
#define MAX_HISTORY_ENTRIES 10
#define MAX_LENGTH 100

// Function to print the ASCII banner
void printBanner() {
    printf("\033[1;31m"); // Red color
    printf("░░░█▀▀░▀█▀░█▀█░█░█░█▀▀░█▀▄░█░█░█▀▀░█░█░░░█▀▀░█▀▀░█▀█░▀█▀░█▀▀\n");
    printf("░░░█░░░░█░░█▀▀░█▀█░█▀▀░█▀▄░█▀▄░█▀▀░░█░░░░█░█░█▀▀░█░█░░█░░█▀▀\n");
    printf("░░░▀▀▀░▀▀▀░▀░░░▀░▀░▀▀▀░▀░▀░▀░▀░▀▀▀░░▀░░░░▀▀▀░▀▀▀░▀░▀░▀▀▀░▀▀▀\n");
    printf("\033[0m"); // Reset color
}

// Function to generate a cryptographically secure random number within a range
int secureRandom(int min, int max) {
    unsigned int randNum;
    if (RAND_bytes((unsigned char *)&randNum, sizeof(randNum)) != 1) {
        perror("Error generating random number");
        exit(EXIT_FAILURE);
    }

    randNum = randNum % (max - min + 1) + min;
    return randNum;
}

// Function to generate the password
void generatePassword(int minLength, int maxLength, int includeLower, int includeUpper, int includeNumbers, int includeSymbols) {
    const char* charsets[] = { "abcdefghijklmnopqrstuvwxyz", "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "0123456789", "!@#$%^&*()_+-=[]{}|;:,.<>?~" };
    const int numCharsets = 4;

    int selectedCharsets = includeLower + includeUpper + includeNumbers + includeSymbols;
    if (selectedCharsets == 0) {
        printf("Please select at least one character type.\n");
        return;
    }

    int length = secureRandom(minLength, maxLength);
    char password[length + 1]; // +1 for the null terminator
    memset(password, 0, sizeof(password));

    char charset[128];
    strcpy(charset, "");

    if (includeLower) strcat(charset, charsets[0]);
    if (includeUpper) strcat(charset, charsets[1]);
    if (includeNumbers) strcat(charset, charsets[2]);
    if (includeSymbols) strcat(charset, charsets[3]);

    int charsetLength = strlen(charset);

    printf("\nGenerated Password: ");

    for (int i = 0; i < length; ++i) {
        int index = secureRandom(0, charsetLength - 1);
        password[i] = charset[index];
        putchar(password[i]); // Display characters one by one
        fflush(stdout); // Ensure immediate display in real-time
        usleep(50000); // Small delay for better display (optional)
    }
    password[length] = '\0'; // Null-terminate the string

    printf("\n");

    int strength = length / 4; // Basic strength calculation
    printf("Password Strength: ");
    switch (strength) {
        case 0:
            printf("\033[0;31mWeak\033[0m\n"); // Red color for weak
            break;
        case 1:
            printf("\033[0;33mModerate\033[0m\n"); // Yellow color for moderate
            break;
        case 2:
            printf("\033[0;32mStrong\033[0m\n"); // Green color for strong
            break;
        default:
            printf("\033[0;36mVery Strong\033[0m\n"); // Cyan color for very strong
    }
}

// Function to calculate password strength
int calculateStrength(int length, int includeLower, int includeUpper, int includeNumbers, int includeSymbols) {
    int selectedCharsets = includeLower + includeUpper + includeNumbers + includeSymbols;
    if (selectedCharsets == 0) {
        return 0; // Cannot calculate strength without any character type selected
    }

    int charsetCount = 0;
    if (includeLower) charsetCount++;
    if (includeUpper) charsetCount++;
    if (includeNumbers) charsetCount++;
    if (includeSymbols) charsetCount++;

    // Calculating strength based on various factors
    int strength = length * charsetCount * selectedCharsets;

    return strength;
}

// Function to save user preferences to a file (with improved encryption)
void savePreferences(int minLength, int maxLength, int includeLower, int includeUpper, int includeNumbers, int includeSymbols) {
    FILE *prefsFile = fopen(FILENAME, "wb");
    if (prefsFile == NULL) {
        perror("Failed to open preferences file");
        return;
    }

    int encryptedPrefs[] = { minLength ^ 255, maxLength ^ 255, includeLower ^ 255, includeUpper ^ 255, includeNumbers ^ 255, includeSymbols ^ 255 };
    fwrite(encryptedPrefs, sizeof(int), 6, prefsFile);

    fclose(prefsFile);
}

// Function to load user preferences from a file (with improved decryption)
void loadPreferences(int *minLength, int *maxLength, int *includeLower, int *includeUpper, int *includeNumbers, int *includeSymbols) {
    FILE *prefsFile = fopen(FILENAME, "rb");
    if (prefsFile == NULL) {
        perror("Failed to open preferences file");
        return;
    }

    int encryptedPrefs[6];
    fread(encryptedPrefs, sizeof(int), 6, prefsFile);

    *minLength = encryptedPrefs[0] ^ 255;
    *maxLength = encryptedPrefs[1] ^ 255;
    *includeLower = encryptedPrefs[2] ^ 255;
    *includeUpper = encryptedPrefs[3] ^ 255;
    *includeNumbers = encryptedPrefs[4] ^ 255;
    *includeSymbols = encryptedPrefs[5] ^ 255;

    fclose(prefsFile);
}

// Function to add generated password to history
void addToHistory(const char *password) {
    FILE *historyFile = fopen(HISTORY_FILENAME, "a");
    if (historyFile == NULL) {
        perror("Failed to open history file");
        return;
    }

    fprintf(historyFile, "%s\n", password);

    fclose(historyFile);
}

// Function to display password history
void displayHistory() {
    printf("\nPassword History:\n");

    FILE *historyFile = fopen(HISTORY_FILENAME, "r");
    if (historyFile == NULL) {
        perror("Failed to open history file");
        return;
    }

    char buffer[100];
    int count = 0;
    while (fgets(buffer, sizeof(buffer), historyFile) && count < MAX_HISTORY_ENTRIES) {
        printf("%d. %s", count + 1, buffer);
        count++;
    }

    fclose(historyFile);
}

int main() {
    int minLength, maxLength, includeLower, includeUpper, includeNumbers, includeSymbols;

    printBanner();

    loadPreferences(&minLength, &maxLength, &includeLower, &includeUpper, &includeNumbers, &includeSymbols);

    int userInput;
    printf("1. Generate Password\n");
    printf("2. Set Preferences\n");
    printf("3. Show Password History\n");
    printf("Enter your choice: ");
    scanf("%d", &userInput);

    switch (userInput) {
        case 1: {
            printf("Generating Password...\n");
            char generatedPassword[MAX_LENGTH + 1]; // Declare password variable
            generatePassword(minLength, maxLength, includeLower, includeUpper, includeNumbers, includeSymbols, generatedPassword); // Pass the password variable to store generated password
            int strength = calculateStrength(strlen(generatedPassword), includeLower, includeUpper, includeNumbers, includeSymbols); // Calculate strength based on the generated password
            // Display password strength and handle it appropriately
            addToHistory(generatedPassword); // Add generated password to history
            break;
        }
        case 2:
             printf("Enter the minimum length of the password: ");
            scanf("%d", &minLength);
            printf("Enter the maximum length of the password: ");
            scanf("%d", &maxLength);
            printf("Include lowercase characters? (1/0): ");
            scanf("%d", &includeLower);
            printf("Include uppercase characters? (1/0): ");
            scanf("%d", &includeUpper);
            printf("Include numbers? (1/0): ");
            scanf("%d", &includeNumbers);
            printf("Include symbols? (1/0): ");
            scanf("%d", &includeSymbols);

            savePreferences(minLength, maxLength, includeLower, includeUpper, includeNumbers, includeSymbols);
            printf("Preferences saved.\n");
            break;
        case 3:
            displayHistory();
            break;
        default:
            printf("Invalid choice.\n");
    }

    return 0;
}
