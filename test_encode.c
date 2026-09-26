#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;

    if(argc < 3 || argc > 5)
    {
        printf("Usage:\n");
        printf("Encode: %s -e source.bmp secret.txt [output.bmp]\n", argv[0]);
        printf("Decode: %s -d stego.bmp [output_file]\n", argv[0]);
        return 1;
    }

    OperationType operation = check_operation_type(argv[1][1]);

    if(operation == e_encode)
    {
        printf("Encoding selected\n");

        if(argc < 4 || argc > 5)
        {
            printf("Usage: %s -e source.bmp secret.txt [output.bmp]\n", argv[0]);
            return 1;
        }

        if(read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            printf("ERROR: Invalid encoding arguments\n");
            return 1;
        }

        if(do_encoding(&encInfo) == e_success)
        {
            printf("Encoding is success\n");
        }
        else
        {
            printf("Encoding failed\n");
            return 1;
        }

        fclose(encInfo.fptr_src_image);
        fclose(encInfo.fptr_secret);
        fclose(encInfo.fptr_stego_image);
    }
    else if(operation == e_decode)
    {
        printf("Decoding selected\n");

        if(argc < 3 || argc > 4)
        {
            printf("Usage: %s -d stego.bmp [output_file]\n", argv[0]);
            return 1;
        }

        if(read_and_validate_decode_args(argv, &decInfo) == e_failure)
        {
            printf("ERROR: Invalid decoding arguments\n");
            return 1;
        }

        if(do_decoding(&decInfo) == e_success)
        {
            printf("Decoding is success\n");
        }
        else
        {
            printf("Decoding failed\n");
            return 1;
        }

        fclose(decInfo.fptr_stego_image);
        fclose(decInfo.fptr_decode);
    }
    else
    {
        printf("ERROR: Unsupported operation\n");
        return 1;
    }

    return 0;
}
	OperationType check_operation_type(char opt)
{
    if(opt == 'e')
        return e_encode;
    else if(opt == 'd')
        return e_decode;
    else
        return e_unsupported;
}

