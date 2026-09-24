#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

#define MAX_FILE_SUFFIX 5

typedef struct _DecodeInfo
{
    /* Output secret file information */
    char decode_fname[50];
    FILE *fptr_decode;

    /* Secret file information */
    int secret_file_extn_size;
    char extn_secret_file[MAX_FILE_SUFFIX];
    long secret_file_size;

    /* Stego image information */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} DecodeInfo;


/* Check operation type */
OperationType check_operation_type(char opt);

/* Read and validate decode arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Open input and output files */
Status open_decfiles(DecodeInfo *decInfo);

/* Decode magic string */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Decode secret file extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo);

#endif
