// src/ImFileDialog/ImFileDialog_Image.cpp
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "mui_dialog.h"
#include "stb_image.h"

namespace ifd
{
	void FileDialog::m_processPreviewResults()
	{
		// This function is called on the main thread, so it can create textures.
		while (true)
		{
			m_previewResultsMutex.lock();
			if (m_previewResults.empty())
			{
				m_previewResultsMutex.unlock();
				break;
			}
			PreviewResult result = m_previewResults.front();
			m_previewResults.pop();
			m_previewResultsMutex.unlock();

			if (result.IconPreviewData)
			{
				void *texture = this->CreateTexture(result.IconPreviewData, result.IconPreviewWidth, result.IconPreviewHeight, 1u);
				stbi_image_free(result.IconPreviewData);

				if (texture)
				{
					bool found = false;
					std::lock_guard<std::recursive_mutex> contentLock(m_contentMutex);
					for (auto &fileData : m_content)
					{
						if (fileData.Path == result.Path)
						{
							fileData.HasIconPreview = true;
							fileData.IconPreview = texture;
							fileData.IconPreviewWidth = result.IconPreviewWidth;
							fileData.IconPreviewHeight = result.IconPreviewHeight;
							found = true;
							break;
						}
					}
					if (!found && this->DeleteTexture)
						this->DeleteTexture(texture); // Clean up if file is no longer in view
				}
			}
		}
	}

	void FileDialog::m_refreshIconPreview()
	{
		m_stopPreviewLoader(); // Stop any existing loader

		if (m_zoom >= 5.0f)
		{
			if (m_previewLoader == nullptr)
			{
				// Populate the queue of files to load previews for
				{
					std::lock_guard<std::recursive_mutex> lock(m_contentMutex);
					m_previewQueue.clear();
					for (const auto &data : m_content)
					{
						if (!data.IsDirectory && !data.HasIconPreview)
						{
							std::string ext = data.Path.extension().string();
							if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
							{
								m_previewQueue.push_back(data.Path);
							}
						}
					}
				}

				if (!m_previewQueue.empty())
				{
					m_previewLoaderRunning = true;
					m_previewLoader = new std::thread(&FileDialog::m_loadPreview, this);
				}
			}
		}
		else
		{
			m_clearIconPreview(); // This also stops the loader
		}
	}

	void FileDialog::m_clearIconPreview()
	{
		m_stopPreviewLoader();

		std::lock_guard<std::recursive_mutex> lock(m_contentMutex);
		// Clear any pending results that haven't been processed
		{
			std::lock_guard<std::mutex> resultsLock(m_previewResultsMutex);
			while (!m_previewResults.empty())
			{
				stbi_image_free(m_previewResults.front().IconPreviewData);
				m_previewResults.pop();
			}
		}

		for (auto &data : m_content)
		{
			if (data.HasIconPreview && this->DeleteTexture)
				this->DeleteTexture(data.IconPreview);
			data.HasIconPreview = false;
			data.IconPreview = nullptr;
		}
	}

	void FileDialog::m_stopPreviewLoader()
	{
		if (m_previewLoader != nullptr)
		{
			m_previewLoaderRunning = false;

			if (m_previewLoader->joinable())
				m_previewLoader->join();

			delete m_previewLoader;
			m_previewLoader = nullptr;
		}
	}

	void FileDialog::m_loadPreview()
	{
		for (const auto &path : m_previewQueue)
		{
			if (!m_previewLoaderRunning)
				break;

			std::string ext = path.extension().string();
			if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
			{
				int width, height, nrChannels;
				unsigned char *image = stbi_load(path.string().c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);

				if (image != nullptr && width > 0 && height > 0)
				{
					PreviewResult result;
					result.Path = path;
					result.IconPreviewData = image;
					result.IconPreviewWidth = width;
					result.IconPreviewHeight = height;

					std::lock_guard<std::mutex> lock(m_previewResultsMutex);
					m_previewResults.push(result);
				}
			}
		}
		m_previewLoaderRunning = false;
	}
}
