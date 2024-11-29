/*

$ gcc -O3 -march=native -o mwc mwc_png.c -lpng -lz
$ ./mwc test.png


*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include <zlib.h>

#define OK 0
#define ERROR -1

png_voidp error_ptr = NULL;
png_error_ptr error_fn = NULL; 
png_error_ptr warn_fn  = NULL;
/* Write a png file */
int write_png(char *file_name, png_byte* image, int width, int height)
{
	int png_transforms = PNG_TRANSFORM_IDENTITY;
	
	FILE *fp;
	png_structp png_ptr;
	png_infop   info_ptr;

	/* Open the file */
	fp = fopen(file_name, "wb");
	if (fp == NULL)
		return ERROR;

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also check that
    * the library version is compatible with the one used at compile time,
    * in case we are using dynamically linked libraries.  REQUIRED.
    */
   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		error_ptr, error_fn, warn_fn);
   if (png_ptr == NULL)
   {
      fclose(fp);
      return ERROR;
   }

   /* Allocate/initialize the image information data.  REQUIRED. */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      png_destroy_write_struct(&png_ptr,  NULL);
      return ERROR;
   }

   /* Set up error handling.  REQUIRED if you aren't supplying your own
    * error handling functions in the png_create_write_struct() call.
    */
   if (setjmp(png_jmpbuf(png_ptr)))
   {
      /* If we get here, we had a problem writing the file. */
      fclose(fp);
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return ERROR;
   }
	png_init_io(png_ptr, fp);

int bit_depth = 8;
const int bytes_per_pixel = 1;
	png_set_IHDR  (png_ptr, info_ptr, width, height, bit_depth,
       PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
       PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info_before_PLTE(png_ptr, info_ptr);
	png_write_info(png_ptr, info_ptr);

	for (int y = 0; y < height; y++)
         png_write_row(png_ptr, image + y * width * bytes_per_pixel);

	png_write_end  (png_ptr, info_ptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	return OK;
}
#include <stdint.h>
uint32_t mwc32x( uint32_t* state, const uint32_t A)
{
	uint32_t x = *state;
	*state = A*(uint16_t)(x) + (x>>16);
    return x;//((x>>16) ^ (x&0xFFFFU));
}
/* преобразование чисел в формат float32 дает распрделение [0,1) */
static inline float u64_float(uint64_t x) {
	return ((uint32_t)((x>>32) ^ (x&0xFFFFFFFFU)) >> 8) * 0x1.0p-24;
}
float uniform (uint32_t *state, uint32_t A1) {
	return (mwc32x(state, A1)&0xFFFFFFU)*0x1.0p-24;
}
int main(int argc, char *argv[]){

	int width = 1024;
	int height= 1024;
	// 0xFE94, 0xFEA0;//0xFE30;
	const uint32_t A1 =0xFEE4;

	if (argc<2) return 1;
	char *file_name = argv[1];

	png_byte *image = malloc(width*height);
	memset (image, 0, width*height);

	uint32_t s = 1;
	uint32_t MASK = (width*height -1);
	for (uint64_t i=0; i<=0x7FFFFFF; i++){
		uint32_t x = uniform(&s, A1)*width;
		uint32_t y = uniform(&s, A1)*height;
		//if (v<width*height) 
		uint8_t col = image[x + y*width];
		col += 1;
		if (col<1) col = 255;
		image[x + y*width] = col;
	}

	
	write_png(file_name, image, width, height);
	return 0;
}