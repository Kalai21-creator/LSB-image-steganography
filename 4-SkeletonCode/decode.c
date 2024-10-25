#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"

/**
 *
 * Function : 
 *------------------
 * brief Reads and validates command line arguments for decoding.
 *
 * This function processes the command line arguments provided to the program 
 * for the decoding operation. It checks the number of arguments and validates 
 * each argument to ensure they meet the expected format. If the arguments 
 * are valid, it populates the provided DecodeInfo structure with the relevant 
 * data necessary for decoding.
 *
 *
 * parameters:
 *-----------------
 * @param argc The number of command line arguments.
 * @param argv An array of command line argument strings.
 * @param decInfo A pointer to a DecodeInfo structure where validated parameters 
 *                will be stored.
 * 
 * Return:
 *----------------
 * return Status An enumerated value indicating the success or failure of the 
 *                 argument validation process. Possible return values include:
 *                 - SUCCESS: if arguments are valid and processed correctly.
 *                 - FAILURE: if there are issues with the provided arguments.
 */
Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo);

Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo)
{
    if (argc < 3 || argc > 4) // Check if the argument count is correct (either 3 or 4)
    {
        printf("Decoding: ./lsb_steg -d <.bmp file> [output file]\n");
        return e_failure;
    }

    // Validate source image file (should end with .bmp)
    char *str = strstr(argv[2], ".bmp");
    if (str != NULL && strcmp(str, ".bmp") == 0)
    {
        decInfo->stego_image_fname1 = argv[2]; // Store the name of the stego image
    }
    else
    {
        printf("Decoding: ./lsb_steg -d <.bmp file> [output file]\n");
        return e_failure;
    }

    // Handle optional output file name
    if (argc == 4)
    {
        strcpy(decInfo->output_fname, argv[3]); // Store the name as provided, without extension manipulation
    }
    else
    {
        strcpy(decInfo->output_fname, "output"); // Default name without extension
        printf("No output file provided. Using default: %s\n", decInfo->output_fname);
    }

    printf("Output file name: %s\n", decInfo->output_fname);
    return e_success;
}

/* 
 * Function: do_decoding
 * ----------------------
 * This function orchestrates the decoding process for extracting secret data
 * from a stego image. It sequentially calls functions to open files, decode 
 * the magic string, file extension size, file extension, secret file size,
 * and secret file data.
 *
 * Parameters:
 * ---------------------
 *   - DecodeInfo *decInfo: A pointer to a DecodeInfo structure containing
 *     information about the stego image and output file.
 *
 * Returns:
 * --------------
 *   - Status: e_success if all decoding operations were successful,
 *             e_failure if any step fails (with appropriate error messages).
 *
 * This function is essential for retrieving and reconstructing the hidden 
 * data from the stego image.
 */

Status do_decoding(DecodeInfo *decInfo)
{
    printf("INFO: ## Decoding Procedure Started ##\n");
    if (open_files_for_decode(decInfo) == e_failure) // Open files needed for decoding
    {
        return e_failure;
    }

    if (decode_magic_string(decInfo) == e_failure)
    {
        return e_failure;
    }

    if (decode_file_extn_size(decInfo) == e_failure)
    {
        return e_failure;
    }
    if (decode_secret_file_extn(decInfo) == e_failure)
    {
        return e_failure;
    }
    if (decode_secret_file_size(decInfo) == e_failure)
    {
        return e_failure;
    }
    if (decode_secret_file_data(decInfo) == e_failure)
    {
        return e_failure;
    }
    printf("INFO: ## Decoding Done Successfully ##\n");
    return e_success;
}

/* 
 * Function: open_files_for_decode
 * ---------------------------------
 * Opens the stego image file for reading. It also skips the BMP header
 * to position the file pointer for decoding operations.
 *
 * Parameters:
 * --------------------
 *   - DecodeInfo *decInfo: A pointer to a DecodeInfo structure containing
 *     the name of the stego image file.
 *
 * Returns:
 * ----------------
 *   - Status: e_success if the file is opened successfully,
 *             e_failure if the file cannot be opened.
 *
 * This function is crucial for ensuring that the stego image is accessible
 * for the subsequent decoding steps.
 */
Status open_files_for_decode(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname1, "r"); // Open the stego image file in read mode
    if (decInfo->fptr_stego_image == NULL)
    {
        printf("ERROR : unable to open the stego image\n");
        return e_failure;
    }
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET); // Skip BMP header
    printf("INFO: Opening required files\n");
    printf("INFO: Opened %s\n", decInfo->stego_image_fname1);
    return e_success;
}

/* 
 * Function: decode_magic_string
 * -------------------------------
 * Reads and decodes the magic string from the stego image, comparing it
 * to the user-provided string to verify that decoding is valid.
 *
 * Parameters:
 *--------------
 *
 *   - DecodeInfo *decInfo: A pointer to a DecodeInfo structure containing
 *     the stego image file pointer.
 *
 * Returns:
 * -----------
 *   - Status: e_success if the decoded magic string matches the input,
 *             e_failure if the strings do not match or reading fails.
 *
 * This function is vital for validating the decoding process and ensuring
 * that the expected data is retrieved.
 */

Status decode_magic_string(DecodeInfo *decInfo)
{
    char magic_string[3];         // Buffer for the magic string to be decoded
    char image_buffer[8] = {0};   // Buffer to hold image data
    char decoded_magic_string[3]; // Buffer for the decoded magic string

    printf("Enter the magic string to decode:\n");
    scanf("%s", magic_string);

    // Decode the magic string from the stego image
    for (int i = 0; i < strlen(magic_string); i++)
    {
        if (fread(image_buffer, sizeof(char), 8, decInfo->fptr_stego_image) != 8)
        {
            printf("ERROR : Unable to read 8 bytes from the stego image\n");
            return e_failure;
        }
        decode_lsb_to_byte(&decoded_magic_string[i], image_buffer); // Decode each byte
    }

    decoded_magic_string[strlen(magic_string)] = '\0'; // Null-terminate the decoded string

    if (strcmp(magic_string, decoded_magic_string) == 0) // Check if the decoded magic string matches the input
    {
        printf("INFO: Decoding Magic String Signature\n");
        printf("INFO: Done\n");
        return e_success;
    }
    else
    {
        printf("The decoded magic string does not match the input magic string.\n");
        return e_failure;
    }
}


/* 
 * Function: decode_lsb_to_byte
 * ------------------------------
 * Decodes a byte from the least significant bits (LSB) of an image buffer.
 *
 * Parameters:
 * -------------
 *   - char *data: A pointer to a variable where the decoded byte will be stored.
 *   - char *image_buffer: A pointer to the image buffer containing LSB data.
 *
 * Returns:
 * ------------
 *   - Status: e_success after successfully decoding the byte.
 *
 * This function is essential for extracting individual bytes from the stego image.
 */

Status decode_lsb_to_byte(char *data, char *image_buffer)
{
    *data = 0; // Initialize the decoded byte to zero
    for (int i = 0; i < 8; i++)
    {
        int lsb = image_buffer[i] & 1; // Extract the least significant bit
        *data = (*data << 1) | lsb;    // Shift and store the LSB
    }
    return e_success;
}


/* 
 * Function: decode_lsb_to_int
 * -----------------------------
 * This function decodes a 32-bit integer from the least significant bits (LSB)
 * of a 32-byte image buffer. It constructs the integer by extracting the LSBs
 * from each byte in the buffer and assembling them.
 *
 * Parameters:
 *   - int *data: A pointer to the integer where the decoded value will be stored.
 *   - char *image_buffer: A pointer to the image buffer containing 32 bytes of data.
 *
 * Returns:
 *   - Status: e_success after successfully decoding the integer.
 */
Status decode_lsb_to_int(int *data, char *image_buffer)
{
    *data = 0; // Initialize the decoded integer to zero
    for (int i = 0; i < 32; i++)
    {
        int lsb = image_buffer[i] & 1; // Extract the least significant bit
        *data = (*data << 1) | lsb;    // Shift and store the LSB
    }
    return e_success;
}



/*
 * Function: decode_file_extn_size
 * ---------------------------------
 * This function decodes the size of the file extension from the stego image.
 * It reads 32 bytes from the image, which encode the size of the file extension,
 * and then decodes it into an integer value.
 *
 * Parameters:
 * ---------------
 *   - DecodeInfo *decInfo: A pointer to a DecodeInfo structure that contains
 *     file pointers and relevant information for decoding.
 *
 * Returns:
 * -----------
 *   - Status: e_success if the size is successfully decoded,
 *             e_failure if there is an error reading from the image.
 */
Status decode_file_extn_size(DecodeInfo *decInfo)
{
    char image_buffer[32] = {0}; // Buffer to hold 32 bytes of image data
    int extn_size = 0;           // Variable to store the decoded extension size

    // Read 32 bytes from the stego image
    if (fread(image_buffer, sizeof(char), 32, decInfo->fptr_stego_image) != 32)
    {
        printf("ERROR: Unable to read 32 bytes from Stego image\n");
        return e_failure;
    }
    decode_lsb_to_int(&extn_size, image_buffer); // Decode the extension size

    decInfo->length = extn_size; // Store the decoded size in DecodeInfo structure
    return e_success;
}


/* 
 * Function: decode_secret_file_extn
 * ------------------------------------
 * This function decodes the secret file extension from the stego image.
 * It reads the specified number of bytes from the image, decodes them 
 * into a string representing the file extension, and prepares to create 
 * an output file with that extension.
 *
 * Parameters:
 * --------------------
 *   - DecodeInfo *decInfo: A pointer to a DecodeInfo structure that contains
 *     file pointers and relevant information for decoding.
 *
 * Returns:
 * ---------------
 *   - Status: e_success if the file extension is successfully decoded and
 *             the output file is created,
 *             e_failure if there is an error reading from the image or 
 *             opening the output file.
 */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    int size = decInfo->length; // Get the size of the extension to decode
    char file_exten[20] = {0};  // Buffer for the decoded file extension
    char image_buffer[8] = {0}; // Buffer to hold image data

    // Read the extension from the stego image
    for (int i = 0; i < size; i++)
    {
        if (fread(image_buffer, sizeof(char), 8, decInfo->fptr_stego_image) != 8)
        {
            printf("ERROR: Unable to read 8 bytes from the stego image\n");
            return e_failure;
        }
        decode_lsb_to_byte(&file_exten[i], image_buffer);// Decode each byte of the extension
    }
    file_exten[size] = '\0';// Null-terminate the decoded extension
    strcpy(decInfo->extension, file_exten); // Store the extension in the DecodeInfo structure

    // Update the output file name with the decoded extension
    char *dot = strchr(decInfo->output_fname, '.'); // Find the last dot in the output file name
    if (dot != NULL)
    {
        *dot = '\0'; // Remove the existing extension
    }
    strcat(decInfo->output_fname, file_exten); // Append the decoded extension
    printf("Output file with decoded extension: %s\n", decInfo->output_fname);

    decInfo->fptr_output_file = fopen(decInfo->output_fname, "w"); // Open the output file for writing
    if (decInfo->fptr_output_file == NULL)
    {
        printf("ERROR: Unable to open the output file\n");
        return e_failure;
    }

    printf("INFO: Opened %s\n", decInfo->output_fname); // For the output file
    printf("INFO: Done. Opened all required files\n");
    return e_success;
}


/* 
 * Function: decode_secret_file_size
 * -----------------------------------
 * Reads the size of the secret file from the stego image.
 *
 * Parameters:
 * --------------
 *   - DecodeInfo *decInfo: A pointer to a DecodeInfo structure containing
 *     the stego image file pointer.
 *
 * Returns:
 * ------------
 *   - Status: e_success if the secret file size was read successfully,
 *             e_failure if reading fails.
 *
 * This function is critical for knowing how many bytes need to be extracted
 * from the stego image for the secret file.
 */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char image_buffer[32] = {0}; // Buffer to hold 32 bytes of image data
    int size = 0; // Variable to store the decoded file size

    // Read 32 bytes from the stego image
    if (fread(image_buffer, sizeof(char), 32, decInfo->fptr_stego_image) != 32)
    {
        printf("ERROR: Unable to read 32 bytes from Stego image\n");
        return e_failure;
    }

    decode_lsb_to_int(&size, image_buffer);// Decode the file size
    decInfo->file_size = size; // Store the decoded size in the DecodeInfo structure
    printf("INFO: Decoding %s File Size\n", decInfo->output_fname); // Reference to the output file
    printf("INFO: Done\n");
    return e_success;
}



/*
 * Function: decode_secret_file_data
 * -----------------------------------
 * Extracts the actual secret data from the stego image and writes it to
 * the output file.
 *
 * Parameters:
 *-----------------
 *
 *   - DecodeInfo *decInfo: A pointer to a DecodeInfo structure containing
 *     the stego image and output file pointers.
 *
 * Returns:
 *------------
 *
 *   - Status: e_success if all secret data was extracted and written
 *             successfully, e_failure if any reading or writing operation fails.
 *
 * This function is essential for reconstructing the secret file from the
 * data hidden in the stego image.
 */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    rewind(decInfo->fptr_output_file);// Reset the output file pointer to the beginning

    char buffer[8]; // Buffer to hold 8 bytes of image data
    char ch;// Variable to hold each decoded character
    for (int i = 0; i < decInfo->file_size; i++)// Read the secret file data from the stego image
    {
        if (fread(buffer, sizeof(char), 8, decInfo->fptr_stego_image) != 8)
        {
            printf("ERROR: Unable to read 8 bytes from stego image\n");
            return e_failure;
        }
        if (decode_lsb_to_byte(&ch, buffer) != e_success)// Decode each character
        {
            printf("ERROR: Decoding from LSB failed\n");
            return e_failure;
        }
        if (fwrite(&ch, sizeof(char), 1, decInfo->fptr_output_file) != 1) // Write the decoded character to the output file
        {
            printf("ERROR: Unable to write to output file\n");
            return e_failure;
        }
    }
    printf("INFO: Decoding %s File Data\n", decInfo->output_fname);
    printf("INFO: Done\n");
    return e_success;
}
