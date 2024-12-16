#include "raylib.h"
#include <stdio.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define NANOSVG_IMPLEMENTATION // Expands implementation
#include "nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

static Image LoadImageSVG(const char* fileName, int width, int height);
void nsvgRasterize2(NSVGrasterizer* r, NSVGimage* image, float tx, float ty, float scale, unsigned char* dst, int w, int h, int stride);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 400;
    const int screenHeight = 400;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - svg loading");

    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context
    // is required)

    // NOTE: Setting this to 400,450 somehow breaks nanosvg.
    Image image = LoadImageSVG("blocks.svg", 21, 21); // Loaded in CPU memory (RAM)
    Texture2D texture = LoadTextureFromImage(image);    // Image converted to texture, GPU memory (VRAM)
    UnloadImage(image);                                 // Once image has been converted to texture and uploaded
                                                        // to VRAM, it can be unloaded from RAM

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawTexture(texture, screenWidth / 2 - texture.width / 2, screenHeight / 2 - texture.height / 2, WHITE);

        // Red border to illustrate how the SVG is centered within the specified
        // dimensions
        DrawRectangleLines((screenWidth / 2 - texture.width / 2) - 1, (screenHeight / 2 - texture.height / 2) - 1, texture.width + 2, texture.height + 2, RED);

        DrawText("this IS a texture loaded from an SVG file!", 300, 410, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture); // Texture unloading

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Load SVG image, rasteraizing it at desired width and height
// NOTE: If width/height are 0, using internal default width/height
static Image LoadImageSVG(const char* fileName, int width, int height)
{
    printf("%s\n", "Start LoadImageSVG");

    Image image = {0};

    if ((strcmp(GetFileExtension(fileName), ".svg") == 0) || (strcmp(GetFileExtension(fileName), ".SVG") == 0))
    {

        printf("%s\n", "Is .svg");

        int charCount = 0;
        FILE* file_ptr;
        int arraySize = 1000;
        char* string_ptr = malloc(arraySize);

        file_ptr = fopen(fileName, "r");
        if (file_ptr == NULL)
        {
            printf("Could not open %s\n", fileName);
            exit(8);
        }

        printf("%s\n", "File was opened.");

        while (1)
        {
            if (charCount == arraySize)
            {

                printf("%s\n", "Array resize");
                char* temp_ptr = malloc(arraySize * 2);

                for (int i = 0; i < arraySize; i++)
                {
                    temp_ptr[i] = string_ptr[i];
                }

                free(string_ptr);
                string_ptr = NULL;

                string_ptr = temp_ptr;
                arraySize = arraySize * 2;

                printf("New arraySize is: %i\n", arraySize);
            }

            char c = fgetc(file_ptr);

            if (c == EOF)
            {
                printf("%s\n", "EOF reached");

                string_ptr[charCount] = '\0';
                charCount++;
                break;
            }

            string_ptr[charCount] = c;
            charCount++;
        }

        fclose(file_ptr);

        printf("%s\n", "Finished reading file");

        // Validate fileData as valid SVG string data
        //<svg xmlns="http://www.w3.org/2000/svg" width="2500" height="2484"
        // viewBox="0 0 192.756 191.488">
        if ((string_ptr != NULL) && (string_ptr[0] == '<') && (string_ptr[1] == 's') && (string_ptr[2] == 'v') && (string_ptr[3] == 'g'))
        {
            struct NSVGimage* svgImage = nsvgParse(string_ptr, "px", 96.0f);

            printf("imgData malloc size: %f\n", svgImage->width * svgImage->height * 4);
            /*
            The 4 is typically there because the image data is
            being stored in a format that uses four bytes per pixel.

            This format is usually RGBA, where:
            R stands for Red,
            G stands for Green,
            B stands for Blue, and
            A stands for Alpha (which represents transparency).
            */
            unsigned char* imgData = RL_MALLOC(svgImage->width * svgImage->height * 4);

            // NOTE: If required width or height is 0, using default SVG internal value
            if (width == 0)
            {
                width = svgImage->width;
            }

            if (height == 0)
            {
                height = svgImage->height;
            }

            // Calculate scales for both the width and the height
            float scaleWidth = width / svgImage->width;
            float scaleHeight = height / svgImage->height;

            // Set the largest of the 2 scales to be the scale to use
            float scale = (scaleHeight > scaleWidth) ? scaleWidth : scaleHeight;

            int offsetX = 0;
            int offsetY = 0;

            if (scaleHeight > scaleWidth)
            {
                offsetY = (height - svgImage->height * scale) / 2;
            }
            else
            {
                offsetX = (width - svgImage->width * scale) / 2;
            }

            printf("%s\n", "Before Rasterize");

            // Rasterize
            struct NSVGrasterizer* rast = nsvgCreateRasterizer();

            printf("%s\n", "Created Rasterizer");

            printf("offsetX: %i\toffsetY: %i\tscale: %f\n", offsetX, offsetY, scale);

            nsvgRasterize2(rast, svgImage, offsetX, offsetY, scale, imgData, width, height, width * 4);

            printf("%s\n", "After Rasterize");

            // Populate image struct with all data
            image.data = imgData;
            image.width = width;
            image.height = height;
            image.mipmaps = 1;
            image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

            printf("%s\n", "Before delete");

            nsvgDelete(svgImage);
            nsvgDeleteRasterizer(rast);

            printf("%s\n", "after delete");

            free(string_ptr);
        }
    }

    return image;
}

// Rasterizes SVG image, returns RGBA image (non-premultiplied alpha)
//   r - pointer to rasterizer context
//   image - pointer to image to rasterize
//   tx,ty - image offset (applied after scaling)
//   scale - image scale
//   dst - pointer to destination image data, 4 bytes per pixel (RGBA)
//   w - width of the image to render
//   h - height of the image to render
//   stride - number of bytes per scaleline in the destination buffer
//
//   In image processing, a scaleline typically refers to a line or a
//   set of lines used as a reference to measure or compare the scale
//   of features within an image
void nsvgRasterize2(NSVGrasterizer* r, NSVGimage* image, float tx, float ty, float scale, unsigned char* dst, int w, int h, int stride)
{
    NSVGshape* shape = NULL;
    NSVGedge* e = NULL;
    NSVGcachedPaint cache;
    int i;

    r->bitmap = dst;
    r->width = w;
    r->height = h;
    r->stride = stride;

    if (w > r->cscanline)
    {
        r->cscanline = w;
        r->scanline = (unsigned char*)realloc(r->scanline, w);

        if (r->scanline == NULL)
        {
            return;
        }
    }
    /*
    i*stride: 0      Size: 1600
    i*stride: 1600   Size: 1600
    i*stride: 3200   Size: 1600
    i*stride: 4800   Size: 1600
    i*stride: 6400   Size: 1600
    i*stride: 8000   Size: 1600
    i*stride: 9600   Size: 1600
    i*stride: 11200  Size: 1600
    i*stride: 12800  Size: 1600
    */
    printf("%s\n", "Before memset");

    /*
    // FIXME: Crashes here:
    for (i = 0; i < h; i++)
    {
        printf("i*stride: %i\t Size: %i\n", i * stride, w * 4);
        // &dst is a memory address for char*, each i*stride must be a row in the bitmap. Size is the width?
        // Why not just do rows * colums and do one memset?
        // Why w * 4
        // Stride is w * 4 wtf?
        memset(&dst[i * stride], 0, w * 4);
    }
    */

    int memsetSize = (h * (w * 4));

    printf("memset size: %i\n", memsetSize);

    // memset(&dst, 0, memsetSize);

    printf("%s\n", "After memset");

    for (shape = image->shapes; shape != NULL; shape = shape->next)
    {
        if (!(shape->flags & NSVG_FLAGS_VISIBLE))
        {
            continue;
        }

        if (shape->fill.type != NSVG_PAINT_NONE)
        {
            nsvg__resetPool(r);
            r->freelist = NULL;
            r->nedges = 0;

            nsvg__flattenShape(r, shape, scale);

            // Scale and translate edges
            for (i = 0; i < r->nedges; i++)
            {
                e = &r->edges[i];
                e->x0 = tx + e->x0;
                e->y0 = (ty + e->y0) * NSVG__SUBSAMPLES;
                e->x1 = tx + e->x1;
                e->y1 = (ty + e->y1) * NSVG__SUBSAMPLES;
            }

            // Rasterize edges
            if (r->nedges != 0)
            {
                qsort(r->edges, r->nedges, sizeof(NSVGedge), nsvg__cmpEdge);
            }

            // now, traverse the scanlines and find the intersections on each scanline, use non-zero rule
            nsvg__initPaint(&cache, &shape->fill, shape->opacity);

            nsvg__rasterizeSortedEdges(r, tx, ty, scale, &cache, shape->fillRule);
        }

        if (shape->stroke.type != NSVG_PAINT_NONE && (shape->strokeWidth * scale) > 0.01f)
        {
            nsvg__resetPool(r);
            r->freelist = NULL;
            r->nedges = 0;

            nsvg__flattenShapeStroke(r, shape, scale);

            //			dumpEdges(r, "edge.svg");

            // Scale and translate edges
            for (i = 0; i < r->nedges; i++)
            {
                e = &r->edges[i];
                e->x0 = tx + e->x0;
                e->y0 = (ty + e->y0) * NSVG__SUBSAMPLES;
                e->x1 = tx + e->x1;
                e->y1 = (ty + e->y1) * NSVG__SUBSAMPLES;
            }

            // Rasterize edges
            if (r->nedges != 0)
            {
                qsort(r->edges, r->nedges, sizeof(NSVGedge), nsvg__cmpEdge);
            }

            // now, traverse the scanlines and find the intersections on each scanline, use non-zero rule
            nsvg__initPaint(&cache, &shape->stroke, shape->opacity);

            nsvg__rasterizeSortedEdges(r, tx, ty, scale, &cache, NSVG_FILLRULE_NONZERO);
        }
    }

    nsvg__unpremultiplyAlpha(dst, w, h, stride);

    r->bitmap = NULL;
    r->width = 0;
    r->height = 0;
    r->stride = 0;
}
