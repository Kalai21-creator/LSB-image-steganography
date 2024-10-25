#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"

#define MAGIC_STRING "#*" // Magic string used as an identifier for encoded data
#define MAX_SECRET_BUF_SIZE 1024
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

// Get image size
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET); // Go to width field

    fread(&width, sizeof(int), 1, fptr_image);  // Read width
    fread(&height, sizeof(int), 1, fptr_image); // Read height

    return width * height * 3; // Return image size (width * height * 3 bytes)
}

// Get image size
uint get_file_size(FILE *fptr)
{
    uint size;
    fseek(fptr, 0, SEEK_END); // Seek to the end of the file
    size = ftell(fptr);       // Get the size (position of file pointer)
    rewind(fptr);             // Rewind the file pointer to the beginning
    return size;
}

/**
 * Funtion: brief Reads and validates command line arguments for encoding.
 *
 * This function processes the command line arguments provided to the program.
 * It checks the number of arguments and validates each argument to ensure they
 * conform to the expected format for encoding operations. If the arguments are
 * valid, it populates the provided EncodeInfo structure with the relevant data.
 * 
 * Parameters:
 * ---------------
 * #param argc The number of command line arguments.
 * #param argv An array of command line argument strings.
 * #param encInfo A pointer to an EncodeInfo structure where valid parameters
 *                will be stored.
 *
 * Return Status:
 * ----------------
 *
 * An enumerated value indicating the success or failure of the
 *                 argument validation process. Possible return values include:
 *                 - SUCCESS: if arguments are valid and processed correctly.
 *                 - FAILURE: if there are issues with the provided arguments.
 */
Status read_and_validate_encode_args(int argc, char *argv[], EncodeInfo *encInfo)
{
    // Validate argument count
    if (argc < 4 || argc > 5)
    {
        printf("Encoding: ./lsb_steg -e <.bmp file> <.txt file> [output file]\n");
        return e_failure;
    }

    // Validate source image file (argv[2])
    char *str = strstr(argv[2], ".bmp");
    if (str != NULL && strcmp(str, ".bmp") == 0)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        printf("ERROR: Source image file must have a .bmp extension.\n");
        return e_failure;
    }

    // Check if the secret file (argv[3]) has a dot in its name
    if (strchr(argv[3], '.') != NULL)
    {
        encInfo->secret_fname = argv[3];
    }
    else
    {
        printf("Encoding: ./lsb_steg -e <.bmp file> <.txt file> [output file]\n");
        return e_failure;
    }

    // Validate and store the stego image file if provided (default is used if not provided)
    if (argc == 5)
    {
        char *str1 = strstr(argv[4], ".bmp");
        if (str1 != NULL && strcmp(str1, ".bmp") == 0)
        {
            encInfo->stego_image_fname = argv[4];
        }
        else
        {
            printf("Encoding: ./lsb_steg -e <.bmp file> <.txt file> [output file]\n");
            return e_failure;
        }
    }
    else
    {
        encInfo->stego_image_fname = "stego_img.bmp";
        printf("No stego image file provided. Using default: %s\n", encInfo->stego_image_fname);
    }

    return e_success;
}

/* 
 * Function: open_files
 * ----------------------
 * Opens the necessary files for reading the source image, the secret file,
 * and writing the stego image. 
 *
 * Parameters:
 * ---------------
 *   - EncodeInfo *encInfo: Structure containing file information.
 *
 * Returns:
 * ---------------
 *   - Status: e_success if all files are opened successfully,
 *             e_failure if any file fails to open.
 */

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }
    // No failure return e_success
    printf("INFO: Opening required files\n");
    printf("INFO: Opened %s\n", encInfo->src_image_fname);
    printf("INFO: Opened %s\n", encInfo->secret_fname);
    printf("INFO: Opened %s\n", encInfo->stego_image_fname);
    printf("INFO: Done\n");
    return e_success;
}


/* 
 * Function: do_encoding
 * -----------------------
 * Executes the encoding process by opening files, checking capacity, 
 * and encoding data into the stego image.
 *
 * Parameters:
 * ---------------
 *   - EncodeInfo *encInfo: Structure containing encoding information.
 *
 * Returns:
 * --------------
 *   - Status: e_success if encoding is successful,
 *             e_failure if an error occurs. */
Status do_encoding(EncodeInfo *encInfo)
{

    // Open the required files
    if (open_files(encInfo) == e_failure)
    {
        return e_failure;
    }
    printf("INFO: ## Encoding Procedure Started ##\n");

    // Check if the image has enough capacity to hold the secret data
    if (check_capacity(encInfo) == e_failure)
    {
        return e_failure;
    }

    // Copy BMP header
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    // Encode magic string
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        return e_failure;
    }

    // Encode secret file extension size
    if (encode_secret_extn_size(encInfo->extn_size, encInfo) == e_failure)
    {
        return e_failure;
    }

    // Encode secret file extension
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
    {
        return e_failure;
    }

    // Encode secret file size
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        return e_failure;
    }

    // Encode secret file data
    if (encode_secret_file_data(encInfo) == e_failure)
    {
        return e_failure;
    }

    // Copy remaining image data to the stego image
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }
    printf("INFO: ## Encoding Done Successfully ##\n");
    return e_success;
}


/* 
 * Function: check_capacity
 * ---------------------------
 * Checks if the stego image has enough capacity to hold the secret data 
 * being encoded.
 *
 * Parameters:
 * -----------
 *   - EncodeInfo *encInfo: Structure containing encoding information.
 *
 * Returns:
 * ---------
 *   - Status: e_success if there is sufficient capacity,
 *             e_failure if capacity is insufficient.
 */
Status check_capacity(EncodeInfo *encInfo)
{
    // Get the size of the source image
    uint image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    // Get the size of the secret file in bytes
    uint size_secret_file = get_file_size(encInfo->fptr_secret);
    // Get the length of the magic string in bytes
    uint magic_string_length = strlen(MAGIC_STRING);
    // Get the secret file extension
    char *file_extension = strchr(encInfo->secret_fname, '.');
    uint extension_size = 0;
    if (file_extension != NULL)
    {
        encInfo->size_secret_file = strlen(file_extension);
        extension_size = strlen(file_extension);
        encInfo->extn_size = extension_size;
        strcpy(encInfo->extn_secret_file, file_extension);
    }
    // Include BMP header size in calculations
    unsigned int header_size = 54;
    // Calculate total size correctly
    unsigned int total_size = header_size + ((magic_string_length + extension_size + size_secret_file + sizeof(size_secret_file) + sizeof(extension_size)) * 8);
    if (image_capacity >= total_size)
    {
        printf("INFO: Checking for %s capacity to handle %s\n", encInfo->src_image_fname, encInfo->secret_fname);
        printf("INFO: Done. Found OK\n");
        return e_success;
    }
    else
    {
        return e_failure;
    }
}



/*
 * Function: copy_bmp_header
 * ---------------------------
 * Copies the BMP header from the source image to the destination image.
 *
 * Parameters:
 * -------------------
 *   - FILE *fptr_src_image: File pointer for the source image.
 *   - FILE *fptr_dest_image: File pointer for the destination (stego) image.
 *
 * Returns:
 * ---------------------
 *   - Status: e_success if header copying is successful,
 *             e_failure if an error occurs.
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];         // Buffer to hold the 54-byte BMP header
    rewind(fptr_src_image);  // Rewind the source image file pointer to the beginning
    rewind(fptr_dest_image); // Rewind the destination image file pointer to the beginning

    if (fread(buffer, 54, 1, fptr_src_image) != 1) // Read the BMP header into the buffer
    {
        printf("ERROR: Unable to read the BMP header from the source image.\n");
        return e_failure;
    }

    if (fwrite(buffer, 54, 1, fptr_dest_image) != 1) // Write the header to the destination file
    {
        printf("ERROR: Unable to write the BMP header from the destination image.\n");
        return e_failure;
    }
    printf("INFO: Copying Image Header\n");
    printf("INFO: Done\n");
    return e_success;
}


/* 
 * Function: encode_magic_string
 * -------------------------------
 * Encodes a predefined magic string into the least significant bits of 
 * the stego image.
 *
 * Parameters:
 * ---------------------
 *   - const char *magic_string: The magic string to be encoded.
 *   - EncodeInfo *encInfo: Structure containing encoding information.
 *
 * Returns:
 * -----------------
 *   - Status: e_success if encoding is successful,
 *             e_failure if an error occurs.
 */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{

    FILE *src_file = encInfo->fptr_src_image;     // Source image file
    FILE *stego_file = encInfo->fptr_stego_image; // Destination stego image file
    char image_buffer[8] = {0};                   // Buffer to hold 8 bytes of image data

    int length = strlen(MAGIC_STRING); // Get the length of the magic string
    for (int i = 0; i < length; i++)
    {
        if (fread(image_buffer, sizeof(char), 8, src_file) != 8) // Read 8 bytes from the source image
        {
            return e_failure;
        }

        char ch = magic_string[i]; // Get the current character from the magic string

        encode_byte_to_lsb(ch, image_buffer); // Encode the character into the LSB of the image buffer

        if (fwrite(image_buffer, sizeof(char), 8, stego_file) != 8) // Write the modified image buffer to the stego image
        {
            return e_failure;
        }
    }
    printf("INFO: Encoding Magic String Signature\n");
    printf("INFO: Done\n");
    return e_success;
}
/**
 * FUNCTION:Status encode_secret_extn_size
 *--------------------------------------------
 *  
 *
 * This function calculates the size of the secret file and updates the 
 * provided EncodeInfo structure to include this size. This information is 
 * crucial for ensuring that the encoding process can accommodate the secret 
 * data without exceeding the available capacity in the cover file.
 *
 * Parameters:
 * ------------
 *
 *   @param file_size The size of the secret file in bytes.
 * @param encInfo A pointer to an EncodeInfo structure that will be updated 
 *                with the secret file size.
 * 
 * Return status:
 * ----------------
 * @return Status An enumerated value indicating the outcome of the operation.
 *                 Possible return values include:
 *                 - SUCCESS: if the secret file size was encoded successfully.
 *                 - FAILURE: if there was an error in processing the size.
 */
Status encode_secret_extn_size(long file_size, EncodeInfo *encInfo)
{
    FILE *src_file = encInfo->fptr_src_image;     // Source image file
    FILE *stego_file = encInfo->fptr_stego_image; // Destination stego image file

    char image_buffer[32] = {0}; // Buffer to hold 32 bytes of image data

    // Read 32 bytes from the source image file
    if (fread(image_buffer, sizeof(char), 32, src_file) != 32)
    {
        printf("ERROR: Unable to read 32 bytes from source image\n");
        return e_failure;
    }

    encode_int_to_lsb(file_size, image_buffer); // Encode the file size into the LSB of the image buffer

    // Write the modified buffer to the stego image file
    if (fwrite(image_buffer, sizeof(char), 32, stego_file) != 32)
    {
        printf("ERROR: Unable to write 32 bytes to stego image\n");
        return e_failure;
    }

    return e_success;
}


/* 
 * Function: encode_byte_to_lsb
 * -------------------------------
 * Encodes a single byte into the least significant bits of the image data array.
 *
 * Parameters:
 * -----------------
 *   - char data: The byte to be encoded.
 *   - char *image_buffer: The image data buffer where the byte will be encoded.
 *
 * Returns:
 * --------------
 *   - Status: e_success after encoding the byte.
 */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE);    // Clear the least significant bit
        char bit = (data & (1 << (7 - i))) >> (7 - i); // Extract the bit from the data
        image_buffer[i] |= bit;                        // Set the least significant bit with the bit from data
    }
    return e_success;
}

/**
 * Function :Status encode_int_to_lsb
 *------------------------------------------
 * brief Encodes an integer into the least significant bits (LSBs) of an image buffer.
 *
 * This function takes a 32-bit integer and embeds it into the first 32 bytes 
 * of the provided image buffer by modifying the least significant bit of each byte. 
 * This technique is commonly used in steganography to hide data within an image.
 *
 * parameters:
 * --------------------
 * -param data The integer data to be encoded (32 bits).
 * -param image_buffer A pointer to the image buffer where the integer will be encoded. 
 *                     It must be at least 32 bytes in size.
 * Return:
 * ------------
 *   -return Status An enumerated value indicating the success or failure of the operation.
 *                 Possible return values include:
 *                 - e_success: if the integer was successfully encoded into the image buffer.
 */

Status encode_int_to_lsb(int data, char *image_buffer)
{
    for (int i = 0; i < 32; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE);     // Clear the LSB
        int bit = (data & (1 << (31 - i))) >> (31 - i); // Extract the bit from data
        image_buffer[i] |= bit;                         // Set the LSB with the extracted bit
    }
    return e_success;
}




/*
 * Function: encode_secret_file_extn
 * ------------------------------------
 * Encodes the file extension of the secret file into the stego image.
 *
 * Parameters:
 * ---------------
 *
 *   - const char *file_extn: The file extension to be encoded.
 *   - EncodeInfo *encInfo: Structure containing encoding information.
 *
 * Returns:
 * -------------
 *
 *   - Status: e_success if encoding is successful,
 *             e_failure if an error occurs.
 */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    FILE *src_file = encInfo->fptr_src_image;     // Source image file
    FILE *stego_file = encInfo->fptr_stego_image; // Destination stego image file
    char image_buffer[8] = {0};                   // Buffer to hold 8 bytes of image data

    int length = strlen(file_extn);

    for (int i = 0; i < length; i++)
    {
        if (fread(image_buffer, sizeof(char), 8, src_file) != 8) // Read 8 bytes from the source image
        {
            printf("Failed to read 8 bytes from source file\n");
            return e_failure;
        }

        char ch = file_extn[i];               // Get the current character from the file extension
        encode_byte_to_lsb(ch, image_buffer); // Encode the character into the LSB of the image buffer

        if (fwrite(image_buffer, sizeof(char), 8, stego_file) != 8) // Write the modified image buffer to the stego image
        {
            printf("Failed to write 8 bytes to stego file\n");
            return e_failure;
        }
    }
    printf("INFO: Encoding %s File Extension\n", encInfo->secret_fname);
    printf("INFO: Done\n");
    return e_success;
}


/* 
 * Function: encode_secret_file_size
 * ------------------------------------
 * Encodes the size of the secret file into the stego image.
 *
 * Parameters:
 * -----------------
 *   - long file_size: The size of the secret file to be encoded.
 *   - EncodeInfo *encInfo: Structure containing encoding information.
 *
 * Returns:
 *---------------
 *
 *   - Status: e_success if encoding is successful,
 *             e_failure if an error occurs.
 */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    FILE *src_file = encInfo->fptr_src_image;     // Source image file
    FILE *stego_file = encInfo->fptr_stego_image; // Destination stego image file
    char image_buffer[32] = {0};                  // Buffer to hold 32 bytes of image data

    // Seek to the end of the secret file to get its size
    fseek(encInfo->fptr_secret, 0, SEEK_END);
    file_size = ftell(encInfo->fptr_secret); // Get the size of the secret file

    if (fread(image_buffer, sizeof(char), 32, src_file) != 32) // Read 32 bytes from the source image
    {
        printf("ERROR: Unable to read 32 bytes from source image\n");
        return e_failure;
    }

    encode_int_to_lsb(file_size, image_buffer); // Encode the file size into the LSB of the image buffer

    if (fwrite(image_buffer, sizeof(char), 32, stego_file) != 32) // Write the modified image buffer to the stego image
    {
        printf("ERROR: Unable to write 32 bytes to stego image\n");
        return e_failure;
    }
    printf("INFO: Encoding %s File Size\n", encInfo->secret_fname);
    printf("INFO: Done\n");
    return e_success;
}



/*
 * Function: encode_secret_file_size
 * ------------------------------------
 * Encodes the size of the secret file into the stego image.
 *
 * Parameters:
 * ------------------
 *   - long file_size: The size of the secret file to be encoded.
 *   - EncodeInfo *encInfo: Structure containing encoding information.
 *
 * Returns:
 * ------------
 *   - Status: e_success if encoding is successful,
 *             e_failure if an error occurs.
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    FILE *src_file = encInfo->fptr_src_image;     // Source image file
    FILE *stego_file = encInfo->fptr_stego_image; // Destination stego image file
    FILE *secret_file = encInfo->fptr_secret;     // Secret file containing the data to be encoded
    char secret_data;                             // Variable to hold each character of the secret file data
    char image_buffer[8] = {0};     // Buffer to hold 8 bytes of image data
    rewind(secret_file);
    // Get the actual size of the secret file
    fseek(secret_file, 0, SEEK_END);
    long size = ftell(secret_file);   // Get the size of the secret file
    encInfo->size_secret_file = size; // Set the correct file size in EncodeInfo structure
    rewind(secret_file);
    // Read and encode the secret file data
    for (int i = 0; i < size; i++)
    {
        if (fread(&secret_data, sizeof(char), 1, secret_file) != 1) // Read a single character from the secret file
        {
            printf("ERROR: Unable to read secret file data\n");
            return e_failure;
        }
        // Read 8 bytes from the source image
        if (fread(image_buffer, sizeof(char), 8, src_file) != 8)
        {
            printf("ERROR: Unable to read 8 bytes from source image\n");
            return e_failure;
        }

        encode_byte_to_lsb(secret_data, image_buffer); // Encode the secret data into the LSB of the image buffer

        if (fwrite(image_buffer, sizeof(char), 8, stego_file) != 8) // Write the modified image buffer to the stego image
        {
            printf("ERROR: Unable to write encoded data to stego image\n");
            return e_failure;
        }
    }
    printf("INFO: Encoding %s File Data\n", encInfo->secret_fname);
    printf("INFO: Done\n");

    return e_success;
}


/* 
 * Function: copy_remaining_img_data
 * ------------------------------------
 * Copies any remaining data from the source image to the stego image after 
 * the encoding process is completed.
 *
 * Parameters:
 * --------------
 *   - FILE *fptr_src: File pointer for the source image.
 *   - FILE *fptr_dest: File pointer for the destination (stego) image.
 *
 * Returns:
 * -----------
 *   - Status: e_success if copying is successful,
 *             e_failure if an error occurs during writing.
 */

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    int ch;// Variable to hold each character read from the source file

    // Loop until the end of the source file
    while ((ch = fgetc(fptr_src)) != EOF)
    {
        // Write the character to the destination file
        if (fputc(ch, fptr_dest) == EOF)
        {
            printf("ERROR : unable to write the remaining data to stego image\n");
            return e_failure;
        }
    }
    printf("INFO: Copying Left Over Data\n");
    printf("INFO: Done\n");
    return e_success;
}
