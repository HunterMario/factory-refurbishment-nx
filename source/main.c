#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <switch.h>

#define PROGRAM_COUNTDOWN 10
#define POST_COUNTDOWN 5

// Simple function to exit app
void exitApp() {
    nsExit();
    appletExit();
    consoleExit(NULL);
}

/*
 * Gets elapsed number of seconds since start_tick
 * Credit to HATS-Tools by Sthetix for the code
 */
s64 getElapsedSeconds(u64 start_tick) {
	if (!start_tick) {
		return 0;
	}
	return armTicksToNs(armGetSystemTick() - start_tick) / 1'000'000'000ULL;
}

int main(int argc, char* argv[])
{
    consoleInit(NULL);


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
    
    u64 start_tick = armGetSystemTick(); // Used for countdown

    // Checking if HOS is at least 3.0.0 because the service command this program
    // relies on was added in HOS 3.0.0
    if (!hosversionAtLeast(3, 0, 0)) {
        printf("This app can only be used on HOS 3.0.0 and above.\n"
               "Consider using SuchMemeManySkill's systemwipe instead.\n\n"
               "The app will exit automatically in 5 seconds.");
        while (getElapsedSeconds(start_tick) < POST_COUNTDOWN);
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
        if (!canFactoryReset && (getElapsedSeconds(start_tick) >= PROGRAM_COUNTDOWN)) {
                canFactoryReset = true;
                printf("To factory reset your console, please push the A button.\n");
        } else if (canFactoryReset && (kDown & HidNpadButton_A)) {
		u64 local_tick = armGetSystemTick();
                // Calling factory reset code
                rc = nsResetToFactorySettingsForRefurbishment();
                // Check the result
                if (R_SUCCEEDED(rc)) {
		    printf("Successully factory reset the console. The console will reboot in 5 seconds.\n");
		    while (getElapsedSeconds(local_tick) < POST_COUNTDOWN);
                    appletRequestToReboot();
                } else {
                    printf("Failed to reset console to factory settings. Error: 0x%x\n", rc);
		    while (getElapsedSeconds(local_tick) < POST_COUNTDOWN);
                }
            }
        consoleUpdate(NULL);
    }
    exitApp();
    return 0;
}
