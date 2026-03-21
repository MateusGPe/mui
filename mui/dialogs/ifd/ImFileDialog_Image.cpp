// src/ImFileDialog/ImFileDialog_Image.cpp
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "ImFileDialog.h"
#include "stb_image.h"

namespace ifd
{
	void FileDialog::m_refreshIconPreview()
	{
		if (m_zoom >= 5.0f)
		{
			if (m_previewLoader == nullptr)
			{
				m_previewLoaderRunning = true;
				m_previewLoader = new std::thread(&FileDialog::m_loadPreview, this);
			}
		}
		else
			m_clearIconPreview();
	}

	void FileDialog::m_clearIconPreview()
	{
		m_stopPreviewLoader();

		std::lock_guard<std::recursive_mutex> lock(m_contentMutex);
		for (auto &data : m_content)
		{
			if (!data.HasIconPreview)
				continue;

			data.HasIconPreview = false;
			if (this->DeleteTexture)
				this->DeleteTexture(data.IconPreview);

			if (data.IconPreviewData != nullptr)
			{
				stbi_image_free(data.IconPreviewData);
				data.IconPreviewData = nullptr;
			}
		}
	}

	void FileDialog::m_stopPreviewLoader()
	{
		if (m_previewLoader != nullptr)
		{
			m_previewLoaderRunning = false;

			if (m_previewLoader && m_previewLoader->joinable())
				m_previewLoader->join();

			delete m_previewLoader;
			m_previewLoader = nullptr;
		}
	}

	void FileDialog::m_loadPreview()
	{
		size_t i = 0;
		while (m_previewLoaderRunning)
		{
			FileData dataCopy(std::filesystem::path(""));
			bool found = false;

			// Scoped lock to safely read from m_content
			{
				std::lock_guard<std::recursive_mutex> lock(m_contentMutex);
				if (i < m_content.size())
				{
					dataCopy = m_content[i];
					found = true;
				}
				else
				{
					break; // Reached end of content
				}
			}

			if (found && !dataCopy.HasIconPreview && dataCopy.Path.has_extension())
			{
				std::string ext = dataCopy.Path.extension().string();
				if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
				{
					int width, height, nrChannels;
					unsigned char *image = stbi_load(dataCopy.Path.string().c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);

					if (image != nullptr && width > 0 && height > 0)
					{

						// Safely write the preview back
						std::lock_guard<std::recursive_mutex> lock(m_contentMutex);
						if (i < m_content.size() && m_content[i].Path == dataCopy.Path)
						{
							m_content[i].HasIconPreview = true;
							m_content[i].IconPreviewData = image;
							m_content[i].IconPreviewWidth = width;
							m_content[i].IconPreviewHeight = height;
						}
						else
						{
							stbi_image_free(image); // Content changed in main thread, discard
						}
					}
				}
			}
			i++;
		}
		m_previewLoaderRunning = false;
	}
}
