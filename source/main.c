// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

// Include the main libnx system header, for Switch development
#include <switch.h>
// #include <ns.h>

// Main program entrypointa
int main(int argc, char* argv[])
{
    // This example uses a text console, as a simple way to output text to the screen.
    // If you want to write a software-rendered graphics application,
    //   take a look at the graphics/simplegfx example, which uses the libnx Framebuffer API instead.
    // If on the other hand you want to write an OpenGL based application,
    //   take a look at the graphics/opengl set of examples, which uses EGL instead.
    consoleInit(NULL);
    int numberOfIterations = 0;
    bool canFactoryReset = false;

    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
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

    printf("factory-refurbishment-nx \n\n");
    printf("This program resets your console to factory settings.\n\n");
    printf("PLEASE BE ABSOLUTELY SURE YOU WANT TO DO THIS.\n\n");
    printf("Please wait 10 seconds before deciding to factory reset. You may press - at any time to abort the factory reset process.\n\n");

    // Main loop
    while (appletMainLoop())
    {
        numberOfIterations++;
        // Scan the gamepad. This should be done once for each frame
        padUpdate(&pad);

        // padGetButtonsDown returns the set of buttons that have been
        // newly pressed in this frame compared to the previous one
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Minus) {
            printf("Minus button pressed. Aborting...\n");
            break; // break in order to return to hbmenu
        }
        
        if (numberOfIterations == 600) {
            canFactoryReset = true;
            printf("To factory reset your console, please the A button. The system will reboot shortly afterwards.\n\n");
        }

        if (canFactoryReset) {
            if (kDown & HidNpadButton_A){
                // Calling factory reset code
                rc = nsResetToFactorySettingsForRefurbishment();
                // Check the result
                if (R_SUCCEEDED(rc)) {
                    appletRequestToReboot();
                } else {
                    printf("Failed to reset console to factory settings. Error: 0x%x\n", rc);
                }
            }
        }

        // Update the console, sending a new frame to the display
        consoleUpdate(NULL);
    }

    // Deinitialize and clean up resources used by the console (important!)
    nsExit();
    appletExit();
    consoleExit(NULL);
    return 0;
}
