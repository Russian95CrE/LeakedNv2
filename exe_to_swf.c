#include <stdio.h>
#include <windows.h>

/**
 * Main function to extract SWF data from a projector bundle.
 *
 * @return 0 on success, 1 on failure.
 */
int main() {
    // Declare variables for file size, file handle, memory heap, and bytes read/written
    int fileSize;                       // Size of the input file
    char endData[8];                    // Buffer to store the last 8 bytes of the input file
    DWORD bytesReadOrWritten;           // Number of bytes read/written
    HANDLE inputFileHandle,             // Handle for input file
           outputFileHandle,            // Handle for output file
           heapMemoryHandle,            // Handle for allocated heap memory
           processHeapHandle;           // Handle to the process heap
    char fileName[30];                  // Buffer to store the name of the Nv2 file

    // Get the name of the Nv2 file from the user
    printf("Nv2 file name (with .exe extension): ");
    scanf("%s", fileName);

    // Open the input file with read access
    inputFileHandle = CreateFile(fileName, GENERIC_READ, 0, NULL, 
                                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    // Check if the file was successfully opened
    if (inputFileHandle == INVALID_HANDLE_VALUE) {
        printf("Cannot open input file for reading!\r\n");
        return 1;
    }

    // Get the size of the input file
    fileSize = GetFileSize(inputFileHandle, NULL);
    
    // Move the file pointer to 8 bytes before the end of the file
    SetFilePointer(inputFileHandle, fileSize - 8, NULL, FILE_BEGIN);
    
    // Read the last 8 bytes of the input file into endData
    ReadFile(inputFileHandle, endData, 8, &bytesReadOrWritten, NULL);
    
    // Check if the first 4 bytes of endData match the signature 0xFA123456
    if (*(DWORD*)&endData == 0xFA123456) {
        // Get the handle to the process heap
        processHeapHandle = GetProcessHeap();
        
        if (processHeapHandle) {
            // Allocate heap memory based on the last 4 bytes of endData (file size information)
            heapMemoryHandle = (PHANDLE)HeapAlloc(processHeapHandle, HEAP_ZERO_MEMORY, *(DWORD*)&endData[4]);
            
            // Move the file pointer to the location where the SWF data starts
            SetFilePointer(inputFileHandle, fileSize - 8 - *(DWORD*)&endData[4], NULL, FILE_BEGIN);
            
            // Read the SWF data into the allocated memory
            ReadFile(inputFileHandle, heapMemoryHandle, *(DWORD*)&endData[4], &bytesReadOrWritten, NULL);
            
            // Create an output file to write the extracted data
            outputFileHandle = CreateFile("dumped.swf", GENERIC_WRITE, 0, NULL, 
                                         CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            
            // Check if the output file was successfully created
            if (outputFileHandle == INVALID_HANDLE_VALUE) {
                printf("Cannot create output file for writing!\r\n");
            } else {
                // Write the SWF data from the heap memory to the output file
                WriteFile(outputFileHandle, heapMemoryHandle, *(DWORD*)&endData[4], &bytesReadOrWritten, NULL);
                
                // Close the output file handle
                CloseHandle(outputFileHandle);
                
                // Output success message
                printf("Done, check your shiny new dumped.swf! =D\r\n");
            }
            
            // Free the allocated heap memory
            HeapFree(processHeapHandle, 0, heapMemoryHandle);
        } else {
            printf("Can't fetch the process heap!\r\n");
        }
    } else {
        // If the signature doesn't match, output an error message
        printf("This doesn't seem to be a projector bundle.\r\n");
    }
    
    // Close the input file handle
    CloseHandle(inputFileHandle);
    
    return 0;
}
