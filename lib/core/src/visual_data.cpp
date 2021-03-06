#include <xsteg/visual_data.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <numeric>

#include <xsteg/availability_map.hpp>

#define SCFLOAT(x) static_cast<float>(x)
#define SCINT(x) static_cast<int>(x)

namespace xsteg
{
	const std::array<uint8_t, 8> truncation_masks
	{
		0xFFu, 0xFEu, 0xFCu, 0xF8u,
        0xF0u, 0xE0u, 0xC0u, 0x80u
	};

    float get_visual_data(
        const uint8_t* px, 
        visual_data_type type, 
        pixel_availability truncate_bits)
    {
        assert(truncate_bits.r < 8 && truncate_bits.r >= -1);
        assert(truncate_bits.g < 8 && truncate_bits.g >= -1);
        assert(truncate_bits.b < 8 && truncate_bits.b >= -1);
        assert(truncate_bits.a < 8 && truncate_bits.a >= -1);

        uint8_t tpx[4];
        std::memcpy(tpx, px, 4);

        tpx[0] &= truncation_masks[std::max(truncate_bits.r, 0)];
        tpx[1] &= truncation_masks[std::max(truncate_bits.g, 0)];
        tpx[2] &= truncation_masks[std::max(truncate_bits.b, 0)];
        tpx[3] &= truncation_masks[std::max(truncate_bits.a, 0)];

        switch (type)
        {
            case visual_data_type::AVERAGE_VALUE_RGB:
            {
                return SCFLOAT(
                    std::accumulate(tpx, tpx + 3, 0)) / 3.0F / 255.0F;
            }
            case visual_data_type::AVERAGE_VALUE_RGBA:
            {
                return SCFLOAT(
                    std::accumulate(tpx, tpx + 4, 0)) / 4.0F / 255.0F;
            }
            case visual_data_type::ALPHA:
            {
                return SCFLOAT(tpx[3]) / 255.0F;
            }
            case visual_data_type::COLOR_BLUE:
            {
                return SCFLOAT(tpx[2]) / 255.0F;
            }
            case visual_data_type::COLOR_GREEN:
            {
                return SCFLOAT(tpx[1]) / 255.0F;
            }
            case visual_data_type::COLOR_RED:
            {
                return SCFLOAT(tpx[0]) / 255.0F;
            }
            case visual_data_type::LUMINANCE:
            {
				uint8_t max_rgb = std::max({ tpx[0], tpx[1], tpx[2] });
				uint8_t min_rgb = std::min({ tpx[0], tpx[1], tpx[2] });

                return std::abs((0.5F * (max_rgb + min_rgb)) / 255.0F);
            }
            case visual_data_type::SATURATION:
            {
				uint8_t max_rgb = std::max({tpx[0], tpx[1], tpx[2]});
				uint8_t min_rgb = std::min({tpx[0], tpx[1], tpx[2]});

                return SCFLOAT(max_rgb - min_rgb) / max_rgb;
            }
            default: return 0;
        }
    }

    std::vector<float> get_visual_data_map(
        const image* img, 
        visual_data_type type,
        pixel_availability truncate_bits)
    {
        std::vector<float> result;
        result.resize(img->pixel_count(), 0);

        for(size_t i = 0; i < img->pixel_count(); ++i)
        {
            const uint8_t* pxptr = img->cdata() + (i * 4);
            result[i] = get_visual_data(pxptr, type, truncate_bits);
        }

        return result;
    }

    image generate_visual_data_image(
        const image* imgptr, 
        visual_data_type type,
        pixel_availability truncate_bits)
    {
        image result(imgptr->width(), imgptr->height());
        for(size_t i = 0; i < imgptr->pixel_count(); ++i)
        {
            const uint8_t* pxptr = imgptr->cdata() + (i * 4);
            float val = get_visual_data(pxptr, type, truncate_bits);
            assert(val <= 1.0F && val >= 0.0F);
            uint8_t val8 = static_cast<uint8_t>(val * 255);
            uint8_t* px = result.pixel_at_idx(i);
            uint8_t new_px[4] = { val8, val8, val8, 255 };
            std::memcpy(px, new_px, 4);
        }
        return result;
    }

    image generate_visual_data_diff_image(
        const image* imgptr, 
        visual_data_type type,
        float val_diff,
        pixel_availability truncate_bits)
    {
        image result(imgptr->width(), imgptr->height());
        for(size_t i = 0; i < imgptr->pixel_count(); ++i)
        {
            const uint8_t* pxptr = imgptr->cdata() + (i * 4);
            float val = get_visual_data(pxptr, type, truncate_bits);
            val = val > val_diff ? 0.0F : 1.0F;
            assert(val <= 1.0F && val >= 0.0F);
            uint8_t val8 = static_cast<uint8_t>(val * 255);
            uint8_t* px = result.pixel_at_idx(i);
            uint8_t new_px[4] = { val8, val8, val8, 255 };
            std::memcpy(px, new_px, 4);
        }
        return result;
    }
}