
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


int count_live_neighbours(int * a,  int size_y, int size_x, int y, int x) {
    int count = 0;
    for (int y_dif = -1 ; y_dif < 2; y_dif ++) {
        for (int x_dif = -1; x_dif < 2; x_dif ++) {
            if (y_dif == 0 && x_dif == 0) {
                continue;
            }
            if ((y + y_dif) < size_y && (y + y_dif) >= 0 && (x + x_dif) < size_x && (x + x_dif) >= 0 &&  a[(y*size_y) + x + (y_dif*size_y) + x_dif] == 1 ) {
                count ++ ;
            }
        }
    }
    return count;

}
void gm_live(int * a,  int size_y, int size_x) {
    int new_game[size_y * size_x];
    for (int y = 0; y < size_y; y ++) {
        for (int x = 0; x < size_x; x ++) {
            int live_neighbours = count_live_neighbours(a, size_y, size_x, y,x);
            if (a[(y* size_y) + x] == 1) {
                if (live_neighbours < 2 || live_neighbours > 3) {
                    new_game[(y * size_y) + x] = 0;
                }
                else {
                    new_game[(y * size_y) + x] = 1;
                }
            }
            else {
                if (live_neighbours == 3){
                    new_game[y * size_y + x] = 1;
                }
                else {
                    new_game[y*size_y + x] = 0;
                }
            }
        }
    }
    for (int y = 0; y < size_y; y ++) {
        for (int x = 0; x < size_x; x ++) {
            a[(y * size_y) + x] = new_game[(y * size_y) + x];
        }
    }
}

void print_matrix(int * a,  int size_y, int size_x) {
    for (int i = 0; i < size_y; i ++ ) {
        for (int j = 0; j < size_x; j++) {
            printf("%d ", a[size_y*i  + j]);
        }
        printf("\n");
    }
}

#pragma pack(push, 1)
union Metadata{
    struct {
        char  naim[2];
        unsigned int size: 32;
        char unsused_data[4];
        int offset;

    } fields;
    unsigned char bytes[14];
};

union Frame_Header {
    struct {
        int size;
        int wight_pixels;
        int height_pixels;
        short int planes;
        short int number_of_bits;
        int compression;
        int size_of_raw;
        int res1;
        int res2;
        int number_colors;
        int important_colors;
    }fields;
    unsigned char bytes[40];
};
#pragma pack(pop)
uint32_t big_to_little_endian(uint32_t num) {
    return ((num>>24)&0xFF) | ((num>>8)&0xFF00) | ((num<<8)&0xFF0000) | ((num<<24)&0xFF000000);
}
int * print_bmp(FILE * file ) {
    fseek(file,  0, SEEK_SET);
    union Metadata x;
    union Frame_Header y;

    fread(x.bytes,sizeof(char), 14, file);
    fread(y.bytes, sizeof(char), 40, file);

    int size = y.fields.height_pixels * y.fields.wight_pixels * (y.fields.number_of_bits / 8);
    int row_size = (y.fields.number_of_bits / 8) * y.fields.wight_pixels;
    int row_padding = (4 - (row_size % 4) ) % 4;

    char * bmp_file = (char *) malloc(sizeof(char ) * (size + row_padding*y.fields.height_pixels));

    fseek(file,  x.fields.offset, SEEK_SET);

    for (int i = 0; i < y.fields.height_pixels; i += 1) {
        for (int j = 0; j < row_size  ; j += 3) {
            char *buf = (char *) malloc(sizeof(char ) * 3);

            fread(buf, 3, sizeof(char), file);

            bmp_file[i*(y.fields.height_pixels*3 + row_padding) + j] = buf[2];
            bmp_file[i*(y.fields.height_pixels*3 + row_padding) +j + 1] = buf[1];
            bmp_file[i*(y.fields.height_pixels*3 + row_padding) + j + 2] = buf[0];
        }
        char * buf = (char *) malloc(sizeof(char ) * row_padding);
        fread(buf, row_padding, sizeof(char ), file);
        for (int j = row_size; j < row_size + row_padding; j ++) {
            bmp_file[i*(y.fields.height_pixels*3 + row_padding) + j] = 0;
        }
    }
    int count = 0;
    int * data = (int *) malloc(sizeof(int) * (size/3));
    int index = 0;
    for (int i = y.fields.height_pixels - 1; i >= 0; i --) {
        for (int j = 0; j < row_size; j += 3){
            char buf[3];
            buf[0] = bmp_file[i*(y.fields.height_pixels*3 + row_padding) + j];
            buf[1] = bmp_file[i*(y.fields.height_pixels*3 + row_padding) + j + 1];
            buf[2] = bmp_file[i*(y.fields.height_pixels*3 + row_padding) + j + 2];
            if (buf[0] == -1 && buf[1] ==  -1 && buf[2] ==   -1) {
                data[index] = 0;
            }
            else {
                count += 1;
                data[index] = 1;

            }
            index ++;
        }
    }
    printf("\n %02x", bmp_file[315]);
    for (int i = 0; i <size/3 ; i ++) {
        if (i % row_size ==  0) {
            printf("\n");
        }
        printf("%d ", data[i]);
    }
    printf("\n");


    printf("\n");
    for (int i = 0; i < size + row_padding * y.fields.height_pixels; i ++ ) {
        if (i % (row_size + row_padding) == 0) {
           printf("\n%04x: ",i );
        }
        printf("%02x ", bmp_file[i]);
    }
    return data;
}


void * save_bmp (FILE *file, FILE * file_write, int * data) {
    fseek(file,  0, SEEK_SET);

    union Metadata x;
    union Frame_Header y;

    fread(x.bytes,sizeof(char), 14, file);
    fread(y.bytes, sizeof(char), 40, file);

    int row_size = (y.fields.number_of_bits / 8) * y.fields.wight_pixels;
    int row_padding = (4 - (row_size % 4) ) % 4;
    int count = 0;
    fseek(file_write,  0, SEEK_SET);
    count += fwrite(&(x.bytes), sizeof(char), 14, file_write);
    count += fwrite(&(y.bytes), sizeof(char), 40, file_write);

    for (int i = 0 ; i < y.fields.height_pixels ; i ++) { //размерность ?
        for (int j = 0; j < row_size; j ++) {
            char * buf = (char *) malloc(sizeof(char ));
            *buf = data[((y.fields.height_pixels - i - 1) * row_size + j) / 3] == 1 ? 0 : -1;
            fseek(file_write,  0, SEEK_CUR);
            fwrite(buf, sizeof(char), 1, file_write);
        }
        char * buf2 = (char *)malloc(sizeof(char ) * row_padding);
        for (int f = 0; f < row_padding; f ++) {
            buf2[f] = 0;
        }
        fseek(file_write,  0, SEEK_CUR);
        fwrite(buf2, sizeof(char ), row_padding, file_write);
    }
}
int main(int argc, char  ** argv) {
    /*printf("%s \n", argv[2]);
    printf("%s \n", argv[4]);
    printf ("%s \n", argv[6]);
    printf("%s \n", argv[8]);*/
    FILE* file = fopen(argv[2] , "r+");// открытие файла в двоичном режиме
    FILE * file_write = fopen(argv[4], "r+");
    int max_iter = strtol(argv[6], NULL, 10) ;
    int freq = strtol(argv[8], NULL, 10 );
    if (file == NULL) {
        printf("Ошибка открытия файла 1");
        return 1;
    }
    else if (file_write == NULL) {
        printf("Ошибка открытия файла 2");
        return 1;
    }
    union Metadata z;
    union Frame_Header y;
    fread(z.bytes,sizeof(char), 14, file);
    fread(y.bytes, sizeof(char), 40, file);
    int row_size =  (y.fields.number_of_bits / 8) * y.fields.wight_pixels / 3;
    int * x  = (int *)malloc(sizeof(int) *y.fields.height_pixels * row_size);
    x = print_bmp(file);
    for (int i = 0 ; i < max_iter;  i ++) {
        if (i % freq == 0) {
            save_bmp(file, file_write, x);
        }
        gm_live(x, y.fields.height_pixels, row_size);
    }
    fclose(file);
    fclose(file_write);
    return 0;
}
