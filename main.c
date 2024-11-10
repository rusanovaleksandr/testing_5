#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <stdint.h>
#define FILE_READ_ERROR 41
#define OPTION_ERROR 42
#define WRONG_ARGUMENTS_ERROR 43
#define MEMORY_ALLOCATION_ERROR 44

#pragma pack(push, 1)

typedef struct BitmapFileHeader
{
    unsigned short signature;    // определение типа файла
    unsigned int filesize;       // размер файла
    unsigned short reserved1;    // должен быть 0
    unsigned short reserved2;    // должен быть 0
    unsigned int pixelArrOffset; // начальный адрес байта, в котором находятся данные изображения (массив пикселей)
} BitmapFileHeader;

typedef struct BitmapInfoHeader
{
    unsigned int headerSize;          // размер этого заголовка в байтах
    unsigned int width;               // ширина изображения в пикселях
    unsigned int height;              // высота изображения в пикселях
    unsigned short planes;            // кол-во цветовых плоскостей (должно быть 1)
    unsigned short bitsPerPixel;      // глубина цвета изображения
    unsigned int compression;         // тип сжатия; если сжатия не используется, то здесь должен быть 0
    unsigned int imageSize;           // размер изображения
    unsigned int xPixelsPerMeter;     // горизонтальное разрешение (пиксель на метр)
    unsigned int yPixelsPerMeter;     // вертикальное разрешение (пиксель на метр)
    unsigned int colorsInColorTable;  // кол-во цветов в цветовой палитре
    unsigned int importantColorCount; // кол-во важных цветов (или 0, если каждый цвет важен)
} BitmapInfoHeader;

typedef struct Rgb
{
    unsigned char b;
    unsigned char g;
    unsigned char r;
} Rgb;

typedef struct BMP
{
    BitmapInfoHeader bmih;
    BitmapFileHeader bmfh;
    Rgb **img;
} BMP;

#pragma pack(pop)

BMP readBMP(char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        printf("Error: file reading error\n");
        exit(FILE_READ_ERROR);
    }

    BMP bmp;
    fread(&bmp.bmfh, 1, sizeof(bmp.bmfh), f);
    fread(&bmp.bmih, 1, sizeof(bmp.bmih), f);
    if (bmp.bmih.headerSize != 40 || bmp.bmih.bitsPerPixel != 24 || bmp.bmfh.signature != 0x4d42 || bmp.bmih.compression != 0)
    {
        printf("Error: unsupported file format\n");
        exit(FILE_READ_ERROR);
    }

    unsigned int H = bmp.bmih.height;
    unsigned int W = bmp.bmih.width;
    bmp.img = (Rgb **)malloc(sizeof(Rgb *) * H);
    if (bmp.img == NULL)
    {
        printf("Memory allocation error!\n");
        exit(MEMORY_ALLOCATION_ERROR);
    }

    for (int i = 0; i < H; i++)
    {
        bmp.img[i] = (Rgb *)malloc(W * sizeof(Rgb) + (4 - (W * sizeof(Rgb)) % 4) % 4);
        if (bmp.img[i] == NULL)
        {
            printf("Memory allocation error!\n");
            exit(MEMORY_ALLOCATION_ERROR);
        }
        fread(bmp.img[i], 1, W * sizeof(Rgb) + (4 - (W * sizeof(Rgb)) % 4) % 4, f);
    }

    fclose(f);
    return bmp;
}

void writeBMP(char *filename, BMP bmp)
{
    FILE *ff = fopen(filename, "wb");
    size_t H = bmp.bmih.height;
    size_t W = bmp.bmih.width;

    fwrite(&bmp.bmfh, sizeof(BitmapFileHeader), 1, ff);
    fwrite(&bmp.bmih, sizeof(BitmapInfoHeader), 1, ff);

    int padding = (4 - (W * 3) % 4) % 4; // выравнивание
    uint8_t paddingBytes[3] = {0};

    for (size_t i = 0; i < H; i++)
    {
        fwrite(bmp.img[i], sizeof(Rgb), W, ff);
        fwrite(paddingBytes, sizeof(uint8_t), padding, ff);
        free(bmp.img[i]);
    }

    free(bmp.img);
    fclose(ff);
}

void printFileHeader(BitmapFileHeader header)
{
    printf("signature:\t%x (%hu)\n", header.signature, header.signature);
    printf("filesize:\t%x (%u)\n", header.filesize, header.filesize);
    printf("reserved1:\t%x (%hu)\n", header.reserved1, header.reserved1);
    printf("reserved2:\t%x (%hu)\n", header.reserved2, header.reserved2);
    printf("pixelArrOffset:\t%x (%u)\n", header.pixelArrOffset, header.pixelArrOffset);
}

void printInfoHeader(BitmapInfoHeader header)
{
    printf("headerSize:\t%x (%u)\n", header.headerSize, header.headerSize);
    printf("width:     \t%x (%u)\n", header.width, header.width);
    printf("height:    \t%x (%u)\n", header.height, header.height);
    printf("planes:    \t%x (%hu)\n", header.planes, header.planes);
    printf("bitsPerPixel:\t%x (%hu)\n", header.bitsPerPixel, header.bitsPerPixel);
    printf("compression:\t%x (%u)\n", header.compression, header.compression);
    printf("imageSize:\t%x (%u)\n", header.imageSize, header.imageSize);
    printf("xPixelsPerMeter:\t%x (%u)\n", header.xPixelsPerMeter, header.xPixelsPerMeter);
    printf("yPixelsPerMeter:\t%x (%u)\n", header.yPixelsPerMeter, header.yPixelsPerMeter);
    printf("colorsInColorTable:\t%x (%u)\n", header.colorsInColorTable, header.colorsInColorTable);
    printf("importantColorCount:\t%x (%u)\n", header.importantColorCount, header.importantColorCount);
}

void printHelp()
{
    printf("Course work for option 4.11, created by Rusanov Aleksandr\n\n");

    printf("***Options:***\n");
    printf("-h, --help: Display this help information\n");
    printf("-i, --info: Display  information about file\n");
    printf("-I, --input <filename>: Specify the input BMP file\n");
    printf("-o, --output <filename>: Specify the output BMP file\n");
    printf("-c, --circle: Draw a circle\n");
    printf("-O, --center <x.y>: Specify the center coordinates of the circle (e.g., --center 100.50)\n");
    printf("-r, --radius <radius>: Set the radius of the circle (positive integer, e.g., --radius 50)\n");
    printf("-T, --thickness <thickness>: Set the thickness of the circle line (positive integer, e.g., --thickness 2)\n");
    printf("-C, --color <rrr.ggg.bbb>: Specify the color of the circle line (RGB values, e.g., --color 255.0.0 for red)\n");
    printf("-F, --fill: Fill the circle with the specified color (optional)\n");
    printf("-P, --fill_color <rrr.ggg.bbb>: Set the fill color of the circle (RGB values, e.g., --fill_color 0.0.255 for blue)\n");
    printf("-f, --rgbfilter: Apply an RGB component filter to the entire image\n");
    printf("-N, --component_name <red|green|blue>: Select the RGB component to modify\n");
    printf("-V, --component_value <value>: Set the value of the selected component (0-255)\n");
    printf("-s, --split: Divide the image into N*M parts\n");
    printf("-x, --number_x <number>: Set the number of horizontal divisions (positive integer, e.g., --number_x 3)\n");
    printf("-y, --number_y <number>: Set the number of vertical divisions (positive integer, e.g., --number_y 2)\n");
    printf("-T, --thickness <thickness>: Set the thickness of the dividing lines (positive integer, e.g., --thickness 10)\n");
    printf("-C --color <rrr.ggg.bbb>: Specify the color of the dividing lines (RGB values, e.g., --color 0.5.0.0 for gray)\n");
    printf("\n");

    printf("***Example Usage:***\n");
    printf("1. Draw a red circle with radius 50 and thickness 3 at coordinates (100, 50):\n");
    printf("./cw -i input.bmp -o output.bmp -c --center 100.50 --radius 50 --thickness 3 --color 255.0.0\n");
    printf("\n");

    printf("2. Apply a green filter to the entire image, setting all green values to 128:\n");
    printf("./cw -i input.bmp -o output.bmp -f --component_name green --component_value 128\n");
    printf("\n");

    printf("3. Divide the image into 4x3 parts with black dividing lines of thickness 10:\n");
    printf("./cw -i input.bmp -o output.bmp -s --number_x 4 --number_y 3 --thickness 10 --color 0.0.0\n");
    printf("\n");
}

void drawPixel(BMP *bmp, int x, int y, Rgb *color)
{
    bmp->img[y][x].r = color->r;
    bmp->img[y][x].g = color->g;
    bmp->img[y][x].b = color->b;
}

void checkDataDrawCircle(BMP *bmp, int coord_x, int coord_y, int radius, int thickness, Rgb *line_color, int fill, Rgb *fill_color)
{
    if (bmp->img == NULL)
    {
        printf("Error: can not find image data\n");
        exit(WRONG_ARGUMENTS_ERROR);
    }
    if (radius <= 0)
    {
        printf("Error: circle radius must be positive\n");
        exit(WRONG_ARGUMENTS_ERROR);
    }
    if (thickness <= 0)
    {
        printf("Error: thickness must be positive\n");
        exit(WRONG_ARGUMENTS_ERROR);
    }
    if (fill == 1 && fill_color == NULL)
    {
        printf("Error: no fill color given\n");
        exit(WRONG_ARGUMENTS_ERROR);
    }
}

void drawCircle(BMP *bmp, int coord_x, int coord_y, int radius, int thickness, Rgb *line_color, int fill, Rgb *fill_color)
{

    int inner_radius = radius - thickness / 2;
    if (inner_radius < 0)
    {
        inner_radius = 0;
    }

    int outer_radius = radius + thickness / 2;
    int min_x = 0;
    int end_iteration_x = bmp->bmih.width;
    int min_y = 0;
    int end_iteration_y = bmp->bmih.height;
    if (coord_x - outer_radius - 1 > 0)
    {
        min_x = coord_x - outer_radius - 1;
    }
    if (coord_y - outer_radius - 1 > 0)
    {
        min_y = coord_y - outer_radius - 1;
    }
    if (coord_x + outer_radius + 1 < bmp->bmih.width)
    {
        end_iteration_x = coord_x + outer_radius + 1;
    }
    if (coord_y + outer_radius + 1 < bmp->bmih.height)
    {
        end_iteration_y = coord_y + outer_radius + 1;
    }

    for (int y = min_y; y < end_iteration_y; y++)
    {
        for (int x = min_x; x < end_iteration_x; x++)
        {
            if (y >= 0 && y < bmp->bmih.height && x >= 0 && x < bmp->bmih.width)
            {
                int squared_dist = (x - coord_x) * (x - coord_x) + (y - coord_y) * (y - coord_y);
                if ((squared_dist <= (outer_radius) * (outer_radius)) && (squared_dist >= (inner_radius) * (inner_radius)))
                {

                    drawPixel(bmp, x, y, line_color);
                }
            }
        }
    }
    if (fill)
    {
        for (int y = coord_y - inner_radius; y <= coord_y + inner_radius; y++)
        {
            for (int x = coord_x - inner_radius; x <= coord_x + inner_radius; x++)
            {
                if (y >= 0 && y < bmp->bmih.height && x >= 0 && x < bmp->bmih.width &&
                    sqrt((x - coord_x) * (x - coord_x) + (y - coord_y) * (y - coord_y)) < inner_radius)
                {
                    drawPixel(bmp, x, y, fill_color);
                }
            }
        }
    }
}

void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

void drawLine(BMP *bmp, int x0, int y0, int x1, int y1, int thickness, Rgb *color)
{
    if (x0 < 0 || y0 < 0 || x1 < 0 || y1 < 0 || thickness <= 0)
    {
        return;
    }

    unsigned int H = bmp->bmih.height;
    unsigned int W = bmp->bmih.width;

    // vertical line
    if (x0 == x1)
    {
        if (y0 > y1)
        {
            swap(&y0, &y1);
        }
        for (int y = y0; y <= y1; y++)
        {
            for (int j = 0; j <= thickness; j++)
            {
                if (H - y >= 0 && x0 - j >= 0 && x0 - j < W && H - y < H)
                {
                    bmp->img[H - y][x0 - j].r = color->r;
                    bmp->img[H - y][x0 - j].g = color->g;
                    bmp->img[H - y][x0 - j].b = color->b;
                }
            }
        }
    }
    else if (y0 == y1)
    {
        if (x0 > x1)
        {
            swap(&x0, &x1);
        }
        for (int x = x0; x <= x1; x++)
        {
            for (int j = 0; j <= thickness; j++)
            {
                if (H - y0 + j >= 0 && x >= 0 && x < W && H - y0 + j < H)
                {
                    bmp->img[H - y0 + j][x].r = color->r;
                    bmp->img[H - y0 + j][x].g = color->g;
                    bmp->img[H - y0 + j][x].b = color->b;
                }
            }
        }
    }
}

void checkDataDividePicture(BMP *bmp, int thickness, int countY, int countX, Rgb *line_color)
{
    if (bmp->img == NULL)
    {
        printf("Error: can not find image data\n");
        exit(WRONG_ARGUMENTS_ERROR);
    }
    if (countY <= 1)
    {
        printf("Error: --number_x argument must be greater than 1\n");
        exit(WRONG_ARGUMENTS_ERROR);
    }
    if (countX <= 1)
    {
        printf("Error: --number_y argument must be greater than 1\n");
        exit(WRONG_ARGUMENTS_ERROR);
    }
    if (thickness <= 0)
    {
        printf("Error: thickness must be positive\n");
        exit(WRONG_ARGUMENTS_ERROR);
    }
}

void dividePicture(BMP *bmp, int thickness, int countY, int countX, Rgb *line_color)
{
    int W = bmp->bmih.width;
    int H = bmp->bmih.height;

    int countlinesY = countY - 1;
    int countlinesX = countX - 1;

    int x0 = W / countX;
    int y0 = H;
    int y1 = 0;
    int cnt_x = 0;
    while (cnt_x != countlinesX)
    {
        drawLine(bmp, x0, y0, x0, y1, thickness, line_color);
        x0 += W / countX;
        cnt_x++;
    }

    x0 = 0;
    int x1 = W;
    y0 = H / countY;
    int cnt_y = 0;
    while (cnt_y != countlinesY)
    {
        drawLine(bmp, x0, y0, x1, y0, thickness, line_color);
        y0 += H / countY;
        cnt_y++;
    }
}

void checkDataRgbFilter(BMP *bmp, char *component_name, int component_value)
{
    if (!(bmp->img != NULL && 0 <= component_value && component_value <= 255))
    {
        printf("Error: wrong data passed to function --rgbfilter\n");
        exit(WRONG_ARGUMENTS_ERROR);
    }
    if (!(strcmp(component_name, "red") == 0 || strcmp(component_name, "green") == 0 || strcmp(component_name, "blue") == 0))
    {
        printf("Error: Invalid component name (red, green, or blue expected)\n");
        exit(WRONG_ARGUMENTS_ERROR);
    }
    if (!(0 <= component_value && component_value <= 255))
    {
        printf("Error: Color values must be between 0 and 255\n");
        exit(WRONG_ARGUMENTS_ERROR);
    }
}

void rgbFilter(BMP *bmp, char *component_name, int value)
{
    int H = bmp->bmih.height;
    int W = bmp->bmih.width;
    unsigned char c;
    if (strcmp(component_name, "red") == 0)
        c = 'r';
    else if (strcmp(component_name, "green") == 0)
        c = 'g';
    else if (strcmp(component_name, "blue") == 0)
        c = 'b';

    for (size_t i = 0; i < H; i++)
    {
        for (size_t j = 0; j < W; j++)
        {
            if (c == 'r')
                (bmp->img)[i][j].r = value;
            else if (c == 'g')
                (bmp->img)[i][j].g = value;
            else if (c == 'b')
                (bmp->img)[i][j].b = value;
        }
    }
}

Rgb *getColor(char *color_str)
{
    if (color_str == NULL)
    {
        printf("Error: wrong argument");
        exit(WRONG_ARGUMENTS_ERROR);
    }

    int check;
    int r, g, b;
    check = sscanf(color_str, "%d.%d.%d", &r, &g, &b);
    if (check != 3)
    {
        printf("Error: invalid color format (expected \"RRR.GGG.BBB\")\n");
        exit(WRONG_ARGUMENTS_ERROR);
    }

    if ((r < 0) || (r > 255) || (g < 0) || (g > 255) || (b < 0) || (b > 255))
    {
        printf("Error: Color values must be between 0 and 255\n");
        exit(WRONG_ARGUMENTS_ERROR);
    }

    Rgb *color = (Rgb *)malloc(sizeof(Rgb));
    if (color == NULL)
    {
        printf("Memory allocation error!\n");
        exit(MEMORY_ALLOCATION_ERROR);
    }

    color->r = (unsigned char)r;
    color->g = (unsigned char)g;
    color->b = (unsigned char)b;
    return color;
}

void getCoordinates(char* center_coords, int* coord_x, int* coord_y)
{
    int check_coords;
    check_coords = sscanf(center_coords, "%d.%d", coord_x, coord_y);
    if (check_coords < 2)
    {
        printf("Error: wrong center coordinates\n");
        exit(WRONG_ARGUMENTS_ERROR);
    }
}

int main(int argc, char *argv[])
{
    char *input_file = argv[argc - 1];
    char *output_file = "output.bmp";
    const char *short_options = "hio:I:fN:V:sx:y:T:C:cO:r:FP:";

    const struct option long_options[] =
        {

            {"help", no_argument, 0, 'h'},
            {"info", no_argument, 0, 'i'},
            {"output", required_argument, 0, 'o'},
            {"input", required_argument, 0, 'I'},
            {"circle", no_argument, 0, 'c'},
            {"center", required_argument, 0, 'O'},
            {"radius", required_argument, 0, 'r'},
            {"fill", no_argument, 0, 'F'},
            {"fill_color", required_argument, 0, 'P'},
            {"rgbfilter", no_argument, 0, 'f'},
            {"component_name", required_argument, 0, 'N'},
            {"component_value", required_argument, 0, 'V'},
            {"split", no_argument, 0, 's'},
            {"number_x", required_argument, 0, 'x'},
            {"number_y", required_argument, 0, 'y'},
            {"thickness", required_argument, 0, 'T'},
            {"color", required_argument, 0, 'C'},
            {0, 0, 0, 0}};

    int opt;
    int option_index;
    int make_info_about_file = 0;
    int option = 0;

    char *center_coords = NULL;
    int coord_x, coord_y;
    int radius = -1;
    int fill = 0;
    char *color_f = NULL;

    char *component_name;
    int component_value = -1;
    int number_x = -1;
    int number_y = -1;
    int thickness = -1;
    char *color = NULL;

    while ((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'h':
        {
            printHelp();
            exit(EXIT_SUCCESS);
            break;
        };
        case 'c':
        {
            option = 1;
            break;
        };
        case 'f':
        {
            option = 2;
            break;
        };
        case 's':
        {
            option = 3;
            break;
        };
        case 'o':
        {
            output_file = optarg;
            break;
        };
        case 'i':
        {
            make_info_about_file = 1;
            break;
        };
        case 'r':
        {
            radius = atoi(optarg);
            break;
        };
        case 'O':
        {
            center_coords = optarg;
            break;
        };
        case 'F':
        {
            fill = 1;
            break;
        };
        case 'P':
        {
            color_f = optarg;
            break;
        };
        case 'N':
        {
            component_name = optarg;
            break;
        };
        case 'V':
        {
            component_value = atoi(optarg);
            break;
        };
        case 'x':
        {
            number_x = atoi(optarg);
            break;
        };
        case 'y':
        {
            number_y = atoi(optarg);
            break;
        };
        case 'T':
        {
            thickness = atoi(optarg);
            break;
        };
        case 'C':
        {
            color = optarg;
            break;
        };
        case 'I':
        {
            input_file = optarg;
            break;
        };
        case '?':
        {
            printf("Error: unknown option\n");
            exit(OPTION_ERROR);
            break;
        }
        }
    }

    BMP bmp = readBMP(input_file);

    if (make_info_about_file == 1)
    {
        printFileHeader(bmp.bmfh);
        printInfoHeader(bmp.bmih);
        exit(EXIT_SUCCESS);
    }

    switch (option)
    {
    case 1:
    {

        Rgb *color_line = getColor(color);
        Rgb *fill_color = NULL;
        if (fill == 1)
        {
            fill_color = getColor(color_f);
        }
        getCoordinates(center_coords, &coord_x, &coord_y);
        coord_y = bmp.bmih.height - coord_y;
        checkDataDrawCircle(&bmp, coord_x, coord_y, radius, thickness, color_line, fill, fill_color);
        drawCircle(&bmp, coord_x, coord_y, radius, thickness, color_line, fill, fill_color);
        free(color_line);
        if (fill_color != NULL)
        {
            free(fill_color);
        }
        break;
    };

    case 2:
    {
        checkDataRgbFilter(&bmp, component_name, component_value);
        rgbFilter(&bmp, component_name, component_value);
        break;
    };

    case 3:
    {
        Rgb *color_line = getColor(color);
        checkDataDividePicture(&bmp, thickness, number_x, number_y, color_line);
        dividePicture(&bmp, thickness, number_x, number_y, color_line);
        free(color_line);
        break;
    };

    default:
    {
        printf("Error: no option selected\n");
        exit(OPTION_ERROR);
        break;
    }
    }

    writeBMP(output_file, bmp);

    return 0;
}