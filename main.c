
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
//

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
    union Metadata x;
    union Frame_Header y;

    fread(x.bytes,sizeof(char), 14, file);
    fread(y.bytes, sizeof(char), 40, file);
    //printf("%d %d", x.fields.offset, y.fields.number_of_bits);

    int size = y.fields.height_pixels * y.fields.wight_pixels * (y.fields.number_of_bits / 8);
    int row_size = (y.fields.number_of_bits / 8) * y.fields.wight_pixels;
    int row_padding = (4 - (row_size % 4) ) % 4;

    char * bmp_file = (char *) malloc(sizeof(char ) * (size + row_padding*y.fields.height_pixels));

    fseek(file,  x.fields.offset, SEEK_SET);

    //printf(" %d  %d %d", row_padding, size, row_size);
    //int count = 0; 000000

    for (int i = 0; i < y.fields.height_pixels; i += 1) {
        for (int j = 0; j < row_size  ; j += 3) {
            char *buf = (char *) malloc(sizeof(char ) * 3);
            //count += 3;
            fread(buf, 3, sizeof(char), file);

            bmp_file[i*(y.fields.height_pixels*3 + row_padding) + j] = buf[2];
            bmp_file[i*(y.fields.height_pixels*3 + row_padding) +j + 1] = buf[1];
            bmp_file[i*(y.fields.height_pixels*3 + row_padding) + j + 2] = buf[0];
            //printf("%d \n%d \n %d \n ", i*(y.fields.height_pixels*3 + row_padding) + j, i*(y.fields.height_pixels*3 + row_padding) + j + 1, i*(y.fields.height_pixels*3 + row_padding) + j + 2 );

            //printf("\n a %d \n", i*(y.fields.height_pixels*3 + row_padding) + j + 2); color="#ffffff" color="#ffffff" e60202 color="#ffffff"
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
            //printf("%02x %02x %02x ",buf[0], buf[1],buf[2]);
            if (buf[0] == -1 && buf[1] ==  -1 && buf[2] ==   -1) {
                data[index] = 0;
            }
            else {
                count += 1;
                data[index] = 1;

            }
            //printf(" -- %d - %d --", data[index], index);

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


void * save_bmp (FILE *file, int * data) {
    fseek(file,  0, SEEK_SET);

    union Metadata x;
    union Frame_Header y;

    fread(x.bytes,sizeof(char), 14, file);
    fread(y.bytes, sizeof(char), 40, file);

    int size = y.fields.height_pixels * y.fields.wight_pixels * (y.fields.number_of_bits / 8);
    int row_size = (y.fields.number_of_bits / 8) * y.fields.wight_pixels;
    int row_padding = (4 - (row_size % 4) ) % 4;

    fseek(file,  x.fields.offset, SEEK_SET);
    //ffff00 ff0000 ff1919
    printf("\n %d \n", row_size);
    int m = 0;
    int i = 0;
    int count = 0;
    int index = 0;
    for (int i = 0 ; i < y.fields.height_pixels ; i ++) { //размерность ?
        for (int j = 0; j < row_size; j ++) {
            char * buf;
            *buf = data[((y.fields.height_pixels - i - 1) * row_size + j) / 3] == 1 ? 0 : -1;
            fwrite(buf, sizeof(char), 1, file);
        }
        char * buf2 = (char * ) malloc(sizeof(char ) * row_padding);
        for (int f = 0; f < row_padding; f ++) {
            buf2[f] = 0;
        }
        fwrite(buf2, sizeof(char ), row_padding, file);
    }
}
int main() {
    // insert code here...
    //int x = readBMP();
    FILE* file = fopen("/Users/kirilllesniak/Desktop/Test_Image_for_lab_14/Test_for_lab_14 — копия 2 3.bmp" , "r+");// открытие файла в двоичном режиме
    if (file == NULL) {
        printf("Ошибка открытия файла");
        return 1;
    }
    int *x  = (int *)malloc(sizeof(int) * 100);
    x = print_bmp(file);
    gm_live(x, 10, 10);
    print_matrix(x, 10, 10);
    save_bmp(file, x);
    fclose(file);
    return 0;
}
