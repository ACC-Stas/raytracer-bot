#pragma once

#include <png.h>
#include <jpeglib.h>
#include <iostream>

struct RGB {
    int r, g, b;
    bool operator==(const RGB& rhs) const {
        return r == rhs.r && g == rhs.g && b == rhs.b;
    }
};

class Image {
public:
    Image(int width, int height) {
        PrepareImage(width, height);
    }

    void PrepareImage(int width, int height) {
        height_ = height;
        width_ = width;
        bytes_ = static_cast<png_bytep*>(malloc(sizeof(png_bytep) * height_));
        for (int y = 0; y < height_; y++) {
            bytes_[y] = static_cast<png_byte*>(malloc(width_ * sizeof(png_byte) * 4));
            for (int x = 0; x < width_; ++x) {
                bytes_[y][x * 4] = bytes_[y][x * 4 + 1] = bytes_[y][x * 4 + 2] = 0;
                bytes_[y][x * 4 + 3] = 255;
            }
        }
    }

    explicit Image(const std::string& filename) {
        if (filename.find(".png") != std::string::npos) {
            ReadPng(filename);
        } else {
            ReadJpg(filename);
        }
    }

    void ReadPng(const std::string& filename) {
        FILE* fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            throw std::runtime_error("Can't open file " + filename);
        }

        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png) {
            throw std::runtime_error("Can't create png read struct");
        }

        png_infop info = png_create_info_struct(png);
        if (!info) {
            throw std::runtime_error("Can't create png info struct");
        }
        if (setjmp(png_jmpbuf(png))) {
            abort();
        }

        png_init_io(png, fp);

        png_read_info(png, info);

        width_ = png_get_image_width(png, info);
        height_ = png_get_image_height(png, info);
        png_byte color_type = png_get_color_type(png, info);
        png_byte bit_depth = png_get_bit_depth(png, info);

        // Read any color_type into 8bit depth, RGBA format.
        // See http://www.libpng.org/pub/png/libpng-manual.txt

        if (bit_depth == 16) {
            png_set_strip_16(png);
        }

        if (color_type == PNG_COLOR_TYPE_PALETTE) {
            png_set_palette_to_rgb(png);
        }

        // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
            png_set_expand_gray_1_2_4_to_8(png);
        }

        if (png_get_valid(png, info, PNG_INFO_tRNS)) {
            png_set_tRNS_to_alpha(png);
        }

        // These color_type don't have an alpha channel then fill it with 0xff.
        if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY ||
            color_type == PNG_COLOR_TYPE_PALETTE) {
            png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
        }

        if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
            png_set_gray_to_rgb(png);
        }

        png_read_update_info(png, info);

        bytes_ = static_cast<png_bytep*>(malloc(sizeof(png_bytep) * height_));
        for (int y = 0; y < height_; y++) {
            bytes_[y] = static_cast<png_byte*>(malloc(png_get_rowbytes(png, info)));
        }

        png_read_image(png, bytes_);
        png_destroy_read_struct(&png, &info, nullptr);
        fclose(fp);
    }

    void ReadJpg(const std::string& filename) {
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr err;
        FILE* infile = fopen(filename.c_str(), "rb");

        if (!infile) {
            throw std::runtime_error("Can't open " + filename);
        }

        cinfo.err = jpeg_std_error(&err);
        jpeg_create_decompress(&cinfo);
        jpeg_stdio_src(&cinfo, infile);

        (void)jpeg_read_header(&cinfo, true);
        (void)jpeg_start_decompress(&cinfo);

        int row_stride = cinfo.output_width * cinfo.output_components;
        JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)(reinterpret_cast<j_common_ptr>(&cinfo),
                                                       JPOOL_IMAGE, row_stride, 1);

        PrepareImage(cinfo.output_width, cinfo.output_height);
        size_t y = 0;

        while (cinfo.output_scanline < cinfo.output_height) {
            (void)jpeg_read_scanlines(&cinfo, buffer, 1);
            for (int x = 0; x < Width(); ++x) {
                RGB pixel;
                if (cinfo.output_components == 3) {
                    pixel.r = buffer[0][x * 3];
                    pixel.g = buffer[0][x * 3 + 1];
                    pixel.b = buffer[0][x * 3 + 2];
                } else {
                    pixel.r = pixel.g = pixel.b = buffer[0][x];
                }
                SetPixel(pixel, y, x);
            }
            ++y;
        }

        (void)jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
    }

    void Write(const std::string& filename) {
        FILE* fp = fopen(filename.c_str(), "wb");
        if (!fp) {
            throw std::runtime_error("Can't open file " + filename);
        }

        png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png) {
            throw std::runtime_error("Can't create png write struct");
        }

        png_infop info = png_create_info_struct(png);
        if (!info) {
            throw std::runtime_error("Can't create png info struct");
        }

        if (setjmp(png_jmpbuf(png))) {
            abort();
        }

        png_init_io(png, fp);

        // Output is 8bit depth, RGBA format.
        png_set_IHDR(png, info, width_, height_, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png, info);

        // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
        // Use png_set_filler().
        // png_set_filler(png, 0, PNG_FILLER_AFTER);

        png_write_image(png, bytes_);
        png_write_end(png, nullptr);

        fclose(fp);
        png_destroy_write_struct(&png, &info);
    }

    RGB GetPixel(int y, int x) const {
        auto row = bytes_[y];
        auto px = &row[x * 4];
        return RGB{px[0], px[1], px[2]};
    }

    void SetPixel(const RGB& pixel, int y, int x) {
        auto row = bytes_[y];
        auto px = &row[x * 4];
        px[0] = pixel.r;
        px[1] = pixel.g;
        px[2] = pixel.b;
    }

    int Height() const {
        return height_;
    }

    int Width() const {
        return width_;
    }

    ~Image() {
        for (int i = 0; i < height_; ++i) {
            free(bytes_[i]);
        }
        free(bytes_);
    }

private:
    int width_, height_;
    png_bytep* bytes_;
};
