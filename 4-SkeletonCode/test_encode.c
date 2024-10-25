#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
     // Structs for encoding and decoding information
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    if (argc > 1) // Check if command line arguments are provided
    {
         // Determine the operation type based on the first argument
        if (check_operation_type(argv[1]) == e_encode)
        {
            // Validate encoding arguments
            if (read_and_validate_encode_args(argc, argv, &encInfo) == e_failure)
            {
                return e_failure;
            }
             // Perform the encoding operation
            do_encoding(&encInfo);
        }
        else if (check_operation_type(argv[1]) == e_decode)
        {
             // Validate decoding arguments
            if (read_and_validate_decode_args(argc, argv, &decInfo) == e_failure)
            {
                return e_failure;
            }
            // Perform the decoding operation
            do_decoding(&decInfo);
        }
        else
        {
            printf("Invalid input\n");
        }
    }
    else
    {
        printf("Encoding: ./lsb_steg -e <.bmp file> <.txt file> [output file]\n");
        printf("Decoding: ./lsb_steg -d <.bmp file> [output file]\n");
    }

    return 0;
}

/* 
 * Function:
 * ------------
 *           check_operation_type
 *           it  Determines the operation type based on the command line argument
 * Parameters:
 * -------------
 *           command line argument as input for encode '-e' and for decode '-d'
 * Returns:
 * ------------
 *          returns e_encode for '-e', e_decode for '-d', and e_unsupported for invalid input
 */
OperationType check_operation_type(char *argv)
{
    // Compare the input argument with the expected operation types
    if (strcmp(argv, "-e") == 0)
    {
        return e_encode;// Return e_encode for encoding operation
    }
    else if (strcmp(argv, "-d") == 0)
    {
        return e_decode;// Return e_decode for decoding operation
    }
    else
    {
        return e_unsupported;// Return e_unsupported for invalid input
    }
}
