

#include "png_common.h"

#include <png.h>
#include <pnginfo.h>

#define png_infopp_NULL NULL

namespace image {

ImageInfo* FromPngFile(LPCTSTR lpszPngFile)
{
	ImageInfo* imageInfo = NULL;

    char png_header[8];
    png_structp png_ptr;
    png_infop info_ptr;
    int width, height, rowBytes;
    png_byte color_type; 
    png_byte bit_depth;
    png_colorp palette; 

    /* open file and test for it being a png */
    FILE *file = _tfopen(lpszPngFile, L"rb");
    fread(png_header, 1, 8, file);
    if(png_sig_cmp((png_bytep)png_header, 0, 8))
    {
        //TFC_DEBUG("Not a PNG file...");
        fclose(file);
    }
    /* initialise structures for reading a png file */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
        //TFC_DEBUG("ReadPngFile: Failed to read the PNG file");
        fclose(file);
    }
    //I/O initialisation methods
    png_init_io(png_ptr, file);
    png_set_sig_bytes(png_ptr, 8);  //Required!!!


    /* **************************************************
     * The high-level read interface in libpng (http://www.libpng.org/pub/png/libpng-1.2.5-manual.html)
     * **************************************************
     */

       png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
       width = info_ptr->width;
       height = info_ptr->height;
       unsigned char* rgba = (unsigned char *)malloc(width * height * 4);  //each pixel(RGBA) has 4 bytes
       png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

       //Original PNG pixel data stored from top-left corner, BUT OGLES Texture drawing is from bottom-left corner
       //int pos = 0;
       //for(int row = 0; row < height; row++)
       //{
           //for(int col = 0; col < (4 * width); col += 4)
           //{
              //rgba[pos++] = row_pointers[row][col];     // red
              //rgba[pos++] = row_pointers[row][col + 1]; // green
              //rgba[pos++] = row_pointers[row][col + 2]; // blue
              //rgba[pos++] = row_pointers[row][col + 3]; // alpha
           //}
       //}


       //unlike store the pixel data from top-left corner, store them from bottom-left corner for OGLES Texture drawing...
       int pos = (width * height * 4) - (4 * width);
       for(int row = 0; row < height; row++)
       {
		  int line = height - row - 1;
          for(int col = 0; col < (4 * width); col += 4)
          {
              rgba[pos++] = row_pointers[line][col];        // red
              rgba[pos++] = row_pointers[line][col + 1]; // green
              rgba[pos++] = row_pointers[line][col + 2]; // blue
              rgba[pos++] = row_pointers[line][col + 3]; // alpha
          }
          pos = (pos - (width * 4) *2); //move the pointer back two rows
       }

    imageInfo = (ImageInfo*)malloc(sizeof(ImageInfo));
    imageInfo->pixelData = rgba;
    imageInfo->imageHeight = height;
    imageInfo->imageWidth = width;

    //clean up after the read, and free any memory allocated
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(file);

	return imageInfo;
}

}