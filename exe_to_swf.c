#include <stdio.h>
#include <windows.h>

int main() {
    // Declare variables for file size, file handle, memory heap, and bytes read/written
    int     szFile;                       // Size of the input file
    char    dataEnd[8];                   // Buffer to store the last 8 bytes of the input file
    DWORD   lpNumberOfBytesRW;            // Number of bytes read/written
    HANDLE  hfInput,                      // Handle for input file
            hfOutput,                     // Handle for output file
            hMemHeap,                     // Handle for allocated heap memory
            hHeap;                        // Handle to the process heap
    
    // Open the input file "Nv2_Current.exe" with read access
    hfInput = CreateFile("Nv2_Current.exe", GENERIC_READ, 0, NULL, 
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    // Check if the file was successfully opened
    if (hfInput == INVALID_HANDLE_VALUE) {
        printf("Cannot open input file for reading!\r\n");
    } else {
        // Get the size of the input file
        szFile = GetFileSize(hfInput, NULL);
        
        // Move the file pointer to 8 bytes before the end of the file
        SetFilePointer(hfInput, szFile - 8, NULL, FILE_BEGIN);
        
        // Read the last 8 bytes of the input file into dataEnd
        ReadFile(hfInput, dataEnd, 8, &lpNumberOfBytesRW, NULL);
        
        // Check if the first 4 bytes of dataEnd match the signature 0xFA123456
        if (*(DWORD*)&dataEnd == 0xFA123456) { // Check the signature
            // Get the handle to the process heap
            hHeap = GetProcessHeap();
            
            if (hHeap) {
                // Allocate heap memory based on the last 4 bytes of dataEnd (file size information)
                hMemHeap = (PHANDLE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, *(DWORD*)&dataEnd[4]);
                
                // Move the file pointer to the location where the SWF data starts
                SetFilePointer(hfInput, szFile - 8 - *(DWORD*)&dataEnd[4], NULL, FILE_BEGIN);
                
                // Read the SWF data into the allocated memory
                ReadFile(hfInput, hMemHeap, *(DWORD*)&dataEnd[4], &lpNumberOfBytesRW, NULL);
                
                // Create an output file "dumped.swf" to write the extracted data
                hfOutput = CreateFile("dumped.swf", GENERIC_WRITE, 0, NULL, 
                                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                
                // Check if the output file was successfully created
                if (hfOutput == INVALID_HANDLE_VALUE) {
                    printf("Cannot create output file for writing!\r\n");
                } else {
                    // Write the SWF data from the heap memory to the output file
                    WriteFile(hfOutput, hMemHeap, *(DWORD*)&dataEnd[4], &lpNumberOfBytesRW, NULL);
                    
                    // Close the output file handle
                    CloseHandle(hfOutput);
                    
                    // Output success message
                    printf("Done, check your shiny new dumped.swf! =)\r\n");
                }
                
                // Free the allocated heap memory
                HeapFree(hHeap, 0, hMemHeap);
            } else {
                printf("Can't fetch the process heap!\r\n");
            }
        } else {
            // If the signature doesn't match, output an error message
            printf("This doesn't seems to be a projector bundle.\r\n");
        }
        
        // Close the input file handle
        CloseHandle(hfInput);
    }
    
    return 0;
}
