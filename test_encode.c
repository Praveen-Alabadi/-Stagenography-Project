#include <stdio.h>
#include "encode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;

    /* Check command-line arguments */
    if(argc < 4 || argc > 5)
    {
        printf("Usage: %s -e source.bmp secret.txt [output.bmp]\n", argv[0]);
        return 1;
    }

    /* Check operation type */
    if(check_operation_type(argv[1][1]) == e_encode)
    {
        printf("Encoding selected\n");

        /* Read and validate arguments */
        if(read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            printf("ERROR: Invalid encoding arguments\n");
            return 1;
        }

        /* Perform encoding */
        if(do_encoding(&encInfo) == e_success)
        {
            printf("Encoding is success\n");
        }
        else
        {
            printf("Encoding failed\n");
            return 1;
        }

        /* Close files */
        fclose(encInfo.fptr_src_image);
        fclose(encInfo.fptr_secret);
        fclose(encInfo.fptr_stego_image);
    }
    else
    {
        printf("ERROR: Unsupported operation\n");
        return 1;
    }

    return 0;
}


/* Check operation type */
OperationType check_operation_type(char opt)
{
    if(opt == 'e')
        return e_encode;
    else if(opt == 'd')
        return e_decode;
    else
        return e_unsupported;
}