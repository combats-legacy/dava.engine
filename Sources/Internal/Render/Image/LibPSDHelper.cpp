/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Render/Image/LibPSDHelper.h"
#include "Render/Image/Image.h"
#include "FileSystem/File.h"

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
#include "libpsd/libpsd.h"
#endif

namespace DAVA
{
LibPSDHelper::LibPSDHelper()
{
    name.assign("PSD");
    supportedExtensions.emplace_back(".psd");
    supportedFormats = { FORMAT_RGBA8888 };
}

bool LibPSDHelper::CanProcessFile(File* infile) const
{
    return GetImageInfo(infile).dataSize != 0;
}

eErrorCode LibPSDHelper::ReadFile(File* infile, Vector<Image*>& imageSet, const ImageSystem::LoadingParams& loadingParams) const
{
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
    auto fileName = infile->GetFilename().GetAbsolutePathname();
    psd_context* psd = nullptr;
    auto status = psd_image_load(&psd, const_cast<psd_char*>(fileName.c_str()));
    if ((psd == nullptr) || (status != psd_status_done))
    {
        DAVA::Logger::Error("============================ ERROR ============================");
        DAVA::Logger::Error("| Unable to load PSD from file (result code = %d):", static_cast<int>(status));
        DAVA::Logger::Error("| %s", fileName.c_str());
        DAVA::Logger::Error("| Try to re-save this file by using `Save as...` in Photoshop");
        DAVA::Logger::Error("===============================================================");
        return eErrorCode::ERROR_READ_FAIL;
    }

    SCOPE_EXIT
    {
        psd_image_free(psd);
    };

    imageSet.reserve(psd->layer_count);
    for (psd_short l = 0; l < psd->layer_count; ++l)
    {
        auto& layer = psd->layer_records[l];

        // swap R and B channels
        DAVA::uint32* p = reinterpret_cast<DAVA::uint32*>(layer.image_data);
        for (psd_int i = 0, e = layer.width * layer.height; i < e; ++i)
        {
            p[i] = (p[i] & 0xff00ff00) | (p[i] & 0x000000ff) << 16 | (p[i] & 0xff0000) >> 16;
        }
        ScopedPtr<Image> layerImage(Image::CreateFromData(layer.width, layer.height, DAVA::PixelFormat::FORMAT_RGBA8888,
                                                          reinterpret_cast<DAVA::uint8*>(layer.image_data)));

        Vector<uint8> emptyData(psd->width * psd->height * PixelFormatDescriptor::GetPixelFormatSizeInBits(DAVA::PixelFormat::FORMAT_RGBA8888) / 8, 0);
        Image* resultImage = Image::CreateFromData(psd->width, psd->height, DAVA::PixelFormat::FORMAT_RGBA8888, emptyData.data());
        resultImage->InsertImage(layerImage, layer.left, layer.top);
        imageSet.push_back(resultImage);
    }

    return eErrorCode::SUCCESS;
#else
    Logger::Error("[LibPSDHelper::ReadFile] PSD reading is disabled for this platform");
    return eErrorCode::ERROR_READ_FAIL;
#endif
}

eErrorCode LibPSDHelper::WriteFile(const FilePath& fileName, const Vector<Image*>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    Logger::Error("[LibPSDHelper::WriteFile] PSD writing is not supported");
    return eErrorCode::ERROR_WRITE_FAIL;
}

eErrorCode LibPSDHelper::WriteFileAsCubeMap(const FilePath& fileName, const Vector<Vector<Image*>>& imageSet, PixelFormat compressionFormat, ImageQuality quality) const
{
    Logger::Error("[LibPSDHelper::WriteFileAsCubeMap] PSD writing is not supported");
    return eErrorCode::ERROR_WRITE_FAIL;
}

DAVA::ImageInfo LibPSDHelper::GetImageInfo(File* infile) const
{
    ImageInfo info;

    return info;
}
};
