
#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
char decode_byte_from_lsb(char *image_buffer);
int decode_size_from_lsb(char *image_buffer);
#include "common.h"

/* Function Definitions */


/*
 * Check operation type
 * Input: Command line option
 * Output: Operation type
 * Return Value: e_decode or e_unsupported
 */


/*
 * Read and validate Decode args from argv
 * Input: Command line arguments
 * Output: DecodeInfo structure
 * Return Value: e_success or e_failure
 */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *dot;

    /* Check source stego image extension */
    dot = strchr(argv[2], '.');

    if(dot == NULL || strcmp(dot, ".bmp") != 0)
    {
        printf("Error : Stego image file must be .bmp file\n");
        return e_failure;
    }

    /* Store stego image file name */
    decInfo->stego_image_fname = argv[2];

    /* Store output decoded file name */
    if(argv[3] == NULL)
    {
        strcpy(decInfo->decode_fname, "output");
    }
    else
    {
        strcpy(decInfo->decode_fname, argv[3]);
    }

    /* Open files */
    if(open_decfiles(decInfo) == e_failure)
    {
        printf("File not opened\n");
        return e_failure;
    }

    return e_success;
}


/*
 * Get File pointers for i/p and o/p files
 * Inputs: Stego Image file and decoded output file
 * Output: FILE pointers for above files
 * Return Value: e_success or e_failure
 */
Status open_decfiles(DecodeInfo *decInfo)
{
    /* Stego image file open */
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");

    if(decInfo->fptr_stego_image == NULL)
    {
        printf("Error : Stego image file not opened\n");
        return e_failure;
    }

    return e_success;
}


/*
 * Perform the decoding
 * Input: DecodeInfo structure
 * Output: Extracted secret file
 * Return Value: e_success or e_failure
 */
Status do_decoding(DecodeInfo *decInfo)
{
    char magic_string[3];

    /* Skip BMP header */
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    /* Decode magic string */
    if(decode_magic_string(magic_string, decInfo) == e_failure)
    {
        printf("Error : Unable to decode magic string\n");
        return e_failure;
    }

    /* Compare decoded magic string */
    if(strcmp(magic_string, MAGIC_STRING) != 0)
    {
        printf("Error : Magic string does not match\n");
        return e_failure;
    }

    /* Decode secret file extension size */
    if(decode_secret_file_extn_size(decInfo) == e_failure)
    {
        printf("Error : Unable to decode secret file extension size\n");
        return e_failure;
    }

    /* Decode secret file extension */
    if(decode_secret_file_extn(decInfo) == e_failure)
    {
        printf("Error : Unable to decode secret file extension\n");
        return e_failure;
    }

    /* Decode secret file size */
    if(decode_secret_file_size(decInfo) == e_failure)
    {
        printf("Error : Unable to decode secret file size\n");
        return e_failure;
    }

    /* Create output file name */
    strcat(decInfo->decode_fname, decInfo->extn_secret_file);

    /* Open decoded output file */
    decInfo->fptr_decode = fopen(decInfo->decode_fname, "w");

    if(decInfo->fptr_decode == NULL)
    {
        printf("Error : Output file not opened\n");
        return e_failure;
    }

    /* Decode secret file data */
    if(decode_secret_file_data(decInfo) == e_failure)
    {
        printf("Error : Unable to decode secret file data\n");
        return e_failure;
    }

    return e_success;
}


/*
 * Decode Magic String
 * Input: Stego image file
 * Output: Magic string
 * Return Value: e_success or e_failure
 */
Status decode_magic_string(char *magic_string, DecodeInfo *decInfo)
{
    char buffer[8];

    for(int i = 0; i < 2; i++)
    {
        if(fread(buffer, 8, 1, decInfo->fptr_stego_image) == 0)
        {
            return e_failure;
        }

        magic_string[i] = decode_byte_from_lsb(buffer);
    }

    magic_string[2] = '\0';

    return e_success;
}


/*
 * Decode secret file extension size
 * Input: Stego image file
 * Output: Secret file extension size
 * Return Value: e_success or e_failure
 */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];

    if(fread(buffer, 32, 1, decInfo->fptr_stego_image) == 0)
    {
        return e_failure;
    }

    decInfo->secret_file_extn_size = decode_size_from_lsb(buffer);

    return e_success;
}


/*
 * Decode secret file extension
 * Input: Stego image file
 * Output: Secret file extension
 * Return Value: e_success or e_failure
 */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];

    for(int i = 0; i < decInfo->secret_file_extn_size; i++)
    {
        if(fread(buffer, 8, 1, decInfo->fptr_stego_image) == 0)
        {
            return e_failure;
        }

        decInfo->extn_secret_file[i] = decode_byte_from_lsb(buffer);
    }

    decInfo->extn_secret_file[decInfo->secret_file_extn_size] = '\0';

    return e_success;
}


/*
 * Decode secret file size
 * Input: Stego image file
 * Output: Secret file size
 * Return Value: e_success or e_failure
 */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];

    if(fread(buffer, 32, 1, decInfo->fptr_stego_image) == 0)
    {
        return e_failure;
    }

    decInfo->secret_file_size = decode_size_from_lsb(buffer);

    return e_success;
}


/*
 * Decode secret file data
 * Input: Stego image file
 * Output: Decoded secret file
 * Return Value: e_success or e_failure
 */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;

    for(long i = 0; i < decInfo->secret_file_size; i++)
    {
        if(fread(buffer, 8, 1, decInfo->fptr_stego_image) == 0)
        {
            return e_failure;
        }

        ch = decode_byte_from_lsb(buffer);

        if(fwrite(&ch, 1, 1, decInfo->fptr_decode) == 0)
        {
            return e_failure;
        }
    }

    return e_success;
}


/*
 * Decode one byte from LSB
 * Input: 8 bytes of image data
 * Output: One decoded byte
 * Return Value: Decoded character
 */
char decode_byte_from_lsb(char *image_buffer)
{
    char data = 0;

    for(int i = 0; i < 8; i++)
    {
        data = data << 1;
        data = data | (image_buffer[i] & 1);
    }

    return data;
}


/*
 * Decode size from LSB
 * Input: 32 bytes of image data
 * Output: Decoded size
 * Return Value: Decoded integer
 */
int decode_size_from_lsb(char *image_buffer)
{
    int data = 0;

    for(int i = 0; i < 32; i++)
    {
        data = data << 1;
        data = data | (image_buffer[i] & 1);
    }

    return data;
}
