#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"
#include "types.h"
#include "common.h"


/* Function Definitions */


/*
 * Get image size
 *
 * Input  : Image file pointer
 * Output : Image capacity
 *
 * In a 24-bit BMP image:
 *     width  = stored at offset 18
 *     height = stored after width
 *
 * Each pixel uses 3 bytes.
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    /* Move file pointer to BMP width location */
    fseek(fptr_image, 18, SEEK_SET);

    /* Read width */
    fread(&width, sizeof(int), 1, fptr_image);

    /* Read height */
    fread(&height, sizeof(int), 1, fptr_image);

    printf("width = %u\n", width);
    printf("height = %u\n", height);

    /* Image capacity = width × height × 3 */
    return width * height * 3;
}


/*
 * Read and validate command-line arguments
 *
 * argv[2] -> source BMP file
 * argv[3] -> secret file
 * argv[4] -> output BMP file (optional)
 */
Status read_and_validate_encode_args(char *argv[],EncodeInfo *encInfo)
{
    /* Find the last '.' in source filename */
    char *dot = strrchr(argv[2], '.');

    /* Source file must be .bmp */
    if(dot == NULL || strcmp(dot, ".bmp") != 0)
    {
        printf("ERROR: Source image file must be .bmp file\n");
        return e_failure;
    }

    /* Store source image filename */
    encInfo->src_image_fname = argv[2];

    /* Store secret filename */
    encInfo->secret_fname = argv[3];


    /*
     * Get secret file extension.
     *
     * Example:
     * secret.txt
     *       ^
     *       dot points to ".txt"
     */
    char *secret_dot = strrchr(encInfo->secret_fname, '.');

    if(secret_dot == NULL)
    {
        printf("ERROR: Secret file extension not found\n");
        return e_failure;
    }

    /* Store ".txt" in extn_secret_file */
    strcpy(encInfo->extn_secret_file, secret_dot);
    
    /*
     * Check whether output filename is given.
     *
     * If not given:
     *     output.bmp is used.
     */
    if(argv[4] == NULL)
    {
        encInfo->stego_image_fname = "output.bmp";
    }
    else
    {
        /* Check output file extension */
        char *dot_stego = strrchr(argv[4], '.');

        if(dot_stego == NULL || strcmp(dot_stego, ".bmp") != 0)
        {
            printf("ERROR: Output file must be .bmp file\n");
            return e_failure;
        }

        /* Store output filename */
        encInfo->stego_image_fname = argv[4];
    }


    /* Open source, secret and output files */
    if(open_files(encInfo) == e_failure)
    {
        printf("File not opened\n");
        return e_failure;
    }

    return e_success;
}


/*
 * Open all required files
 *
 * Source image -> read binary mode
 * Secret file  -> read binary mode
 * Stego image  -> write binary mode
 */
Status open_files(EncodeInfo *encInfo)
{
    /* Open source BMP file */
    encInfo->fptr_src_image =fopen(encInfo->src_image_fname, "rb");

    if(encInfo->fptr_src_image == NULL)
    {
        printf("ERROR: Source file not opened\n");
        return e_failure;
    }


    /* Open secret file */
    encInfo->fptr_secret =fopen(encInfo->secret_fname, "rb");

    if(encInfo->fptr_secret == NULL)
    {
        printf("ERROR: Secret file not opened\n");
        return e_failure;
    }


    /* Open output/stego file */
    encInfo->fptr_stego_image =
        fopen(encInfo->stego_image_fname, "wb");

    if(encInfo->fptr_stego_image == NULL)
    {
        printf("ERROR: Output file not opened\n");
        return e_failure;
    }

    return e_success;
}


/*
 * Perform complete encoding process
 *
 * Order:
 *
 * 1. Check capacity
 * 2. Copy BMP header
 * 3. Encode magic string
 * 4. Encode extension size
 * 5. Encode extension
 * 6. Encode secret file size
 * 7. Encode secret file data
 * 8. Copy remaining image data
 */
Status do_encoding(EncodeInfo *encInfo)
{
    /* Check whether BMP has enough space */
    if(check_capacity(encInfo) == e_failure)
    {
        printf("ERROR: Insufficient image capacity\n");
        return e_failure;
    }


    /* Copy original BMP header to output */
    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: BMP Header not copied\n");
        return e_failure;
    }


    /* Encode magic string "#*" */
    if(encode_magic_string(MAGIC_STRING,encInfo) == e_failure)
    {
        printf("ERROR: Unable to encode magic string\n");
        return e_failure;
    }


    /* Encode size of secret file extension */
    if(encode_secret_file_extn_size(encInfo) == e_failure)
    {
        printf("ERROR: Failed to encode extension size\n");
        return e_failure;
    }


    /* Encode secret file extension ".txt" */
    if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo) == e_failure)
    {
        printf("ERROR: Unable to encode secret file extension\n");
        return e_failure;
    }


    /* Encode secret file size */
    if(encode_secret_file_size(encInfo->size_secret_file,encInfo) == e_failure)
    {
        printf("ERROR: Unable to encode secret file size\n");
        return e_failure;
    }


    /* Encode actual secret file data */
    if(encode_secret_file_data(encInfo) == e_failure)
    {
        printf("ERROR: Unable to encode secret file data\n");
        return e_failure;
    }


    /* Copy remaining BMP data without modification */
    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Unable to copy remaining image\n");
        return e_failure;
    }

    return e_success;
}


/*
 * Check whether source image has enough capacity
 * to store all secret information.
 */
Status check_capacity(EncodeInfo *encInfo)
{
    /* Get BMP image capacity */
    encInfo->image_capacity =get_image_size_for_bmp(encInfo->fptr_src_image);

    /* Get secret file size */
    encInfo->size_secret_file =get_file_size(encInfo->fptr_secret);

    /*
     * Calculate extension size.
     *
     * Example:
     * ".txt" -> 4 bytes
     */
    int extn_size =strlen(encInfo->extn_secret_file);


    /*
     * Required image bytes:
     *
     * Magic string
     * + 4 bytes for extension size
     * + extension
     * + 4 bytes for secret file size
     * + secret data
     *
     * Every secret byte needs 8 image bytes.
     */
    uint required_size =(strlen(MAGIC_STRING)+ 4 + extn_size + 4 + encInfo->size_secret_file) * 8;


    printf("Image capacity: %u bytes\n",encInfo->image_capacity);

    printf("Required capacity: %u bytes\n",required_size);


    /* Check whether image is large enough */
    if(required_size > encInfo->image_capacity)
    {
        printf("ERROR: Secret file is too large\n");
        return e_failure;
    }

    return e_success;
}


/*
 * Get size of secret file.
 *
 * ftell() gives current file position.
 * By moving to SEEK_END, current position
 * becomes the file size.
 */
uint get_file_size(FILE *fptr)
{
    uint size;

    /* Move to end of file */
    fseek(fptr, 0, SEEK_END);

    /* Get file size */
    size = ftell(fptr);

    /* Move back to beginning */
    fseek(fptr, 0, SEEK_SET);

    return size;
}


/*
 * Copy the 54-byte BMP header.
 *
 * BMP header must remain unchanged.
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];

    /* Start from beginning of BMP */
    rewind(fptr_src_image);

    /* Read BMP header */
    if(fread(buffer, 54, 1, fptr_src_image) == 0)
    {
        return e_failure;
    }

    /* Write BMP header */
    if(fwrite(buffer, 54, 1, fptr_dest_image) == 0)
    {
        return e_failure;
    }

    return e_success;
}


/*
 * Encode magic string.
 *
 * Example:
 * MAGIC_STRING = "#*"
 *
 * '#' is stored using 8 image bytes.
 * '*' is stored using next 8 image bytes.
 */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];

    for(int i = 0; i < strlen(magic_string); i++)
    {
        /* Read 8 image bytes */
        if(fread(buffer, 8, 1, encInfo->fptr_src_image) == 0)
        {
            return e_failure;
        }

        /* Store one character in 8 LSBs */
        encode_byte_to_lsb(magic_string[i], buffer);

        /* Write encoded bytes */
        if(fwrite(buffer, 8, 1, encInfo->fptr_stego_image) == 0)
        {
            return e_failure;
        }
    }

    return e_success;
}


/*
 * Encode one character into 8 image bytes.
 *
 * One character = 8 bits.
 *
 * Example:
 * 'A' = 01000001
 *
 * Each bit is stored in the LSB
 * of one image byte.
 */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i = 7; i >= 0; i--)
    {
        /*
         * If current data bit is 1:
         *     set image byte LSB
         *
         * If current data bit is 0:
         *     clear image byte LSB
         */
        if(data & (1 << i))
        {
            image_buffer[7 - i] = image_buffer[7 - i] | 1;
        }
        else
        {
            image_buffer[7 - i] = image_buffer[7 - i] & ~1;
        }
    }

    return e_success;
}


/*
 * Encode a 32-bit integer into 32 image bytes.
 *
 * Used for:
 *     extension size
 *     secret file size
 */
Status encode_size_to_lsb(long data, char *image_buffer)
{
    /* We need only 32 bits */
    unsigned int value = (unsigned int)data;

    for(int i = 31; i >= 0; i--)
    {
        /*
         * Store each bit of the integer
         * in the LSB of one image byte.
         */
        if(value & (1U << i))
        {
            image_buffer[31 - i] = image_buffer[31 - i] | 1;
        }
        else
        {
            image_buffer[31 - i] = image_buffer[31 - i] & ~1;
        }
    }

    return e_success;
}


/*
 * Encode secret file extension.
 *
 * Example:
 * ".txt"
 *
 * Each character requires 8 image bytes.
 */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];

    /* Process each extension character */
    for(int i = 0; file_extn[i] != '\0'; i++)
    {
        /* Read 8 image bytes */
        if(fread(buffer, 8, 1, encInfo->fptr_src_image) == 0)
        {
            return e_failure;
        }

        /* Encode one extension character */
        encode_byte_to_lsb(file_extn[i], buffer);

        /* Write encoded bytes */
        if(fwrite(buffer, 8, 1, encInfo->fptr_stego_image) == 0)
        {
            return e_failure;
        }
    }

    return e_success;
}


/*
 * Encode size of secret file extension.
 *
 * Example:
 * ".txt" -> 4
 *
 * The value 4 is stored in 32 image bytes.
 */
Status encode_secret_file_extn_size(EncodeInfo *encInfo)
{
    char buffer[32];

    /* Get extension size */
    int size = strlen(encInfo->extn_secret_file);

    /* Read 32 image bytes */
    if(fread(buffer, 32, 1, encInfo->fptr_src_image) == 0)
    {
        return e_failure;
    }

    /* Store extension size */
    encode_size_to_lsb(size, buffer);

    /* Write encoded bytes */
    if(fwrite(buffer, 32, 1, encInfo->fptr_stego_image) == 0)
    {
        return e_failure;
    }

    return e_success;
}


/*
 * Encode secret file size.
 *
 * Example:
 * secret.txt = 25 bytes
 *
 * The value 25 is stored in 32 image bytes.
 */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];

    /* Read 32 image bytes */
    if(fread(buffer, 32, 1, encInfo->fptr_src_image) == 0)
    {
        return e_failure;
    }

    /* Store secret file size */
    encode_size_to_lsb(file_size,buffer);

    /* Write encoded bytes */
    if(fwrite(buffer, 32, 1, encInfo->fptr_stego_image) == 0)
    {
        return e_failure;
    }

    return e_success;
}


/*
 * Encode actual secret file data.
 *
 * Read one character from secret file.
 * Then use 8 image bytes to store that character.
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char buffer[8];

    /*
     * Start reading secret file
     * from the beginning.
     */
    rewind(encInfo->fptr_secret);

    int ch;

    /* Read secret file character by character */
    while((ch = fgetc(encInfo->fptr_secret)) != EOF)
    {
        /* Read 8 image bytes */
        if(fread(buffer, 8, 1, encInfo->fptr_src_image) == 0)
        {
            return e_failure;
        }

        /* Store secret character in image */
        encode_byte_to_lsb(ch, buffer);

        /* Write encoded image bytes */
        if(fwrite(buffer, 8, 1, encInfo->fptr_stego_image) == 0)
        {
            return e_failure;
        }
    }

    return e_success;
}


/*
 * Copy remaining image data.
 *
 * The part after the encoded secret data
 * does not need modification.
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    int ch;

    /* Copy one byte at a time */
    while((ch = fgetc(fptr_src)) != EOF)
    {
        if(fwrite(&ch, 1, 1, fptr_dest) == 0)
        {
            return e_failure;
        }
    }

    return e_success;
}