#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>
#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"


using u8 = uint8_t;
using u32 = uint32_t;
using i32 = int32_t;

#define GRAY_COLOR_STEP (10)
#define GRAY_COLOR_PALETTE_SIZE
#define RESET ("\u001b[0m")
#define MOVE_CURSOR_UP(lines) ("\033["#lines"A")
#define MOVE_CURSOR_DOWN(lines) ("\033["#lines"B")
#define MOVE_CURSOR_LEFT(spaces) ("\033["#spaces"D")
#define MOVE_CURSOR_RIGHT(spaces) ("\033["#spaces"C")


struct Image {
    i32 width;
    i32 height;
    i32 components;
    u8* data;
};

struct Color {
    union {
        struct {
            u8 r;
            u8 g;
            u8 b;
            u8 a;
        };
        u8 c[4]; // NOTE(Sarmis) from components
        u32 identifier;
    };
};

struct Palette {
    std::map<u32, std::string> colors;
};

Palette generateGrayPalette() {
    Palette result = {};

    // std::cout << "Generating Gray Palette" << std::endl;

    int row = 14;
    int col = 8;
    int color = row * 16 + col;
    int currentColor = 0;

    std::string code = RESET;
    Color newColor = {};
    result.colors[newColor.identifier] = code;
    currentColor += GRAY_COLOR_STEP;

    while(true){
        code = "\u001b[48;5;";
        code += std::to_string(color++);
        code += "m";
        // std::cout << code << " " << RESET;

        newColor.c[0] = currentColor;
        newColor.c[1] = currentColor;
        newColor.c[2] = currentColor;
        currentColor += GRAY_COLOR_STEP;

        result.colors[newColor.identifier] = code;
        
        if(color >= 16 * 16){
            break;    
        }
    }

    // std::cout << "\n";
    // std::cout << RESET << std::endl;

    return result;
}

Palette generateColorPalette() {
    Palette result = {};

    std::cout << "Generating Gray Palette" << std::endl;

    for(int y = 0; y < 16; ++y){
        for(int x = 0; x < 16; ++x){
            std::string code = "\u001b[48;5;";
            code += std::to_string(x + y * 16);
            code += "m";

            std::cout << code << " " << RESET ;
        }
        std::cout << "\n";
    }
    std::cout << RESET << std::endl;

    return result;
}

void freeImage(Image* image){
    if(image == NULL){
        return; // throw
    }
    stbi_image_free(image->data);
}

Image readImage(const char* filename){
    Image result = {};

    i32 tmp = 0;
    result.data = stbi_load(filename, &result.width, &result.height, &tmp, 1);
    result.components = 1; // force components for now

    return result;
}

Image resizeImage(Image* image, i32 newWidth, i32 newHeight){
    Image result = {};
    
    result.width = newWidth;
    result.height = newHeight;

    result.data = (u8*)malloc(sizeof(u8) * newWidth * newHeight);

    stbir_resize_uint8(image->data,      image->width,      image->height, 0,
                       result.data, (u32)result.width, (u32)result.height, 0, 1);

    result.components = image->components;

    return result;
}

void printNewline(){
    std::cout << "\n";
}

void printPixel(Palette* palette, Color color){
    std::cout << palette->colors[color.identifier] << " " << RESET;
}

void printImageWithPalette(Image* image, Palette* palette){
    switch(image->components){
        case 1: { // gray scale
                for(int y = 0; y < image->height; ++y){
                    i32 row = y * image->width;

                    for(int x = 0; x < image->width; ++x){
                        u8 color = (image->data[x + row] / GRAY_COLOR_STEP) * GRAY_COLOR_STEP;
                        if(color > 230){
                            color = 230;
                        }

                        Color c = {};
                        c.c[0] = color;
                        c.c[1] = color;
                        c.c[2] = color;

                        printPixel(palette, c);
                    }
                    printNewline();
                }
            }
            break;
    }
    std::cout << std::endl;
}

void printUsage(const char* name){
    std::cout << name << " [options]\n";
    std::cout << "    -h, --help             this message\n";
    std::cout << "    --width WIDTH          tells the width to be displayed\n";
    std::cout << "    --height HEIGHT        tells the height to be displayed\n";
    std::cout << "    -i PATH                the path to the image(lower resolution the better)\n";
    std::cout << "    --specs                display the specs\n";
    std::cout << std::endl;
}

void writeSpecs(i32 width, i32 height){
    std::cout << MOVE_CURSOR_UP((height / 2));
    std::cout << MOVE_CURSOR_RIGHT((width + 4));
    std::cout << "Something in here...................";
    std::cout << MOVE_CURSOR_DOWN((height / 2));
}
 

int main(int argumentCount, char* arguments[]){

    if(argumentCount < 2) {
        printUsage(arguments[0]);
        return 1;
    }

    i32 displayWidth = 0;
    i32 displayHeight = 0;
    char* inputImage = NULL;

    // no error handling for now
    for(int i = 0; i < argumentCount; ++i){
        if(!strcmp(arguments[i], "-h") || !strcmp(arguments[i], "--help")){
            printUsage(arguments[0]);
            return 0;
        } else if(!strcmp(arguments[i], "--width")){
            char* dw = arguments[++i];
            displayWidth = atoi(dw);
        } else if(!strcmp(arguments[i], "--height")){
            char* dh = arguments[++i];
            displayHeight = atoi(dh);
        } else if(!strcmp(arguments[i], "-i")){
            inputImage = arguments[++i];
        } else if(!strcmp(arguments[i], "--specs")){
            //
        }
    }

    if(!displayWidth || !displayHeight || !inputImage){
        std::cout << "No width or no height or no image path was specified, please specify them all, thank you." << std::endl;
        printUsage(arguments[0]);
        return 1;
    }

    Image input = readImage(inputImage);

    if(!input.data){
        std::cout << "The image " << inputImage << " could not be read, please check the path is correct or if the file exists." << std::endl; 
        return 1;
    }

    Image resizedImage = resizeImage(&input, displayWidth, displayHeight);
    
    Palette grayPalette = generateGrayPalette();

    printImageWithPalette(&resizedImage, &grayPalette);

    freeImage(&input);
    freeImage(&resizedImage);

    return 0;
} 

 