#include <windows.h>      // Windows API
#include <tchar.h>        // For _TCHAR type, compatible with Unicode and ANSI
#include <stdio.h>        // Standard input/output (e.g., printf)
#include "SimConnect.h"   // SimConnect SDK header

HANDLE hSimConnect = NULL; // Global handle for the SimConnect connection
int quit = 0;              // Flag to know when to stop the loop (if used)

// --------------------------
// ENUMS FOR EVENT MANAGEMENT
// --------------------------

// Groups help organize events (e.g., input, system)
enum GROUP_ID {
    GROUP0,  // Just one group in this case
};

// Custom client event ID
enum EVENT_ID {
    EVENT_EXIT,  // We’ll map this to P3D's internal "EXIT" event
};

// ------------------------------------
// EVENT DISPATCH CALLBACK (called by SimConnect)
// ------------------------------------
void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{
    // Handle received messages from SimConnect
    switch (pData->dwID)
    {
    case SIMCONNECT_RECV_ID_QUIT:
        // Received when P3D is about to close
        quit = 1;
        break;

    default:
        // Other message types ignored
        break;
    }
}

// ------------------------------------
// MAIN FUNCTION TO SEND EXIT COMMAND
// ------------------------------------
void sendExitEvent()
{
    // Try connecting to SimConnect (i.e., to P3D)
    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Exit P3D", NULL, 0, NULL, 0)))
    {
        // Map our custom EVENT_EXIT to the Sim event named "EXIT"
        // This tells SimConnect what action our event represents
        SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_EXIT, "EXIT");

        // Add this event to a notification group so we can send/receive it
        SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP0, EVENT_EXIT, FALSE);

        // Set group priority — HIGHEST means it’s not filtered out by accident
        SimConnect_SetNotificationGroupPriority(hSimConnect, GROUP0, SIMCONNECT_GROUP_PRIORITY_HIGHEST);

        // Send the EXIT event to the user aircraft (SIMCONNECT_OBJECT_ID_USER)
        SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, EVENT_EXIT, 0,
            GROUP0, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);

        // Confirm to user
        printf("P3D exit command received, please wait...\n");

        // Optional: Wait for confirmation that P3D is closing
        for (int i = 0; i < 100 && !quit; i++)
        {
            SimConnect_CallDispatch(hSimConnect, MyDispatchProc, NULL);
            Sleep(50);  // Wait 50 ms per cycle = 5 seconds total max
        }

        // Close the SimConnect connection
        SimConnect_Close(hSimConnect);
    }
    else
    {
        // SimConnect failed to connect (maybe P3D isn’t running yet)
        printf("Unable to connect to P3D via SimConnect.\n");
    }
}

// -----------------------------
// MAIN ENTRY POINT (like main())
// -----------------------------
int __cdecl _tmain(int argc, _TCHAR* argv[])
{
    sendExitEvent();  // Start the shutdown process
    return 0;         // Exit the app
}
