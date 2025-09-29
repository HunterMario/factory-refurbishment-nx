#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <switch.h>

// Simple function to exit app
void exitApp() {
    nsExit();
    appletExit();
    consoleExit(NULL);
}

int main(int argc, char* argv[])
{
    consoleInit(NULL);

    // Constant
    const float SECONDS_TO_WAIT = 10.;
    const int MIN_HOS_VERSION_MAJOR = 3;
    const int MIN_HOS_VERSION_MINOR = 0;

    // Determines if system can factory reset (used for warning)
    bool canFactoryReset = false;
    
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);

    // Initializing the ns library
    Result rc;
    rc = nsInitialize();
    if (R_FAILED(rc)) {
        printf("Failed to initialize ns service. Error: 0x%x\n", rc);
        consoleExit(NULL);
        return -1;
    }
    // Initializing the applet library
    rc = appletInitialize();
    if (R_FAILED(rc)) {
        printf("Failed to initialize applet service. Error: 0x%x\n", rc);
        consoleExit(NULL);
        return -1;
    }
    
    // Checking if HOS is at least 3.0.0 because the service command this program
    // relies on was added in HOS 3.0.0
    if (!hosversionAtLeast(MIN_HOS_VERSION_MAJOR, MIN_HOS_VERSION_MINOR, MIN_HOS_VERSION_MINOR)) {
        printf("This app can only be used on HOS 3.0.0 and above.\n"
               "Consider using SuchMemeManySkill's systemwipe instead.\n\n"
               "The app will exit automatically in 10 seconds.");
        while ((float) clock() / CLOCKS_PER_SEC < SECONDS_TO_WAIT);
        exitApp();
        return 0;
    }

    printf("factory-refurbishment-nx \n\n");
    printf("This program resets your console to factory settings.\n\n");
    printf("PLEASE BE ABSOLUTELY SURE YOU WANT TO DO THIS.\n\n");
    printf("Please wait 10 seconds before deciding to factory reset. "
        "You may press - at any time to abort the factory reset process.\n\n");

    // Main loop
    while (appletMainLoop())
    {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Minus) {
            printf("Minus button pressed. Aborting...\n");
            break;
        }
        if (!canFactoryReset) {
            if ((float) clock() / CLOCKS_PER_SEC >= SECONDS_TO_WAIT) {
                canFactoryReset = true;
                printf("To factory reset your console, please the A button. "
                    "The system will reboot shortly afterwards.\n\n");
            }
        } else if (kDown & HidNpadButton_A) {
                // Calling factory reset code
                rc = nsResetToFactorySettingsForRefurbishment();
                // Check the result
                if (R_SUCCEEDED(rc)) {
                    appletRequestToReboot();
                } else {
                    printf("Failed to reset console to factory settings. Error: 0x%x\n", rc);
                }
            }
        consoleUpdate(NULL);
    }
    exitApp();
    return 0;
}
