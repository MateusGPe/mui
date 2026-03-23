// src/ImFileDialog/ImFileDialog_Image.cpp
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "mui_dialog.h"
#include "stb_image.h"

namespace mui_dlg
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

	void FileDialog::m_startPreviewLoaderIfNeeded()
	{
		if (m_previewLoader == nullptr)
		{
			m_previewThreadExit = false;
			m_previewLoaderRunning = true;
			m_previewLoader = new std::thread(&FileDialog::m_loadPreview, this);
		}
	}

	void FileDialog::m_refreshIconPreview()
	{
		if (m_zoom >= 5.0f)
		{
			m_startPreviewLoaderIfNeeded();

			// Populate the queue of files to load previews for
			{
				std::lock_guard<std::recursive_mutex> lock(m_contentMutex);
				std::lock_guard<std::mutex> qLock(m_previewQueueMutex);

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

			// Wake up the worker thread
			m_previewCv.notify_one();
		}
		else
		{
			m_clearIconPreview();
		}
	}

	void FileDialog::m_clearIconPreview()
	{
		// Stop fetching new items immediately
		{
			std::lock_guard<std::mutex> qLock(m_previewQueueMutex);
			m_previewQueue.clear();
		}

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
			// PROPER FIX: Lock the mutex before signaling the exit condition
			// so the thread doesn't miss the notify_all() while sleeping.
			{
				std::lock_guard<std::mutex> lock(m_previewQueueMutex);
				m_previewThreadExit = true;
			}
			m_previewCv.notify_all();

			if (m_previewLoader->joinable())
				m_previewLoader->join();

			delete m_previewLoader;
			m_previewLoader = nullptr;
			m_previewLoaderRunning = false;
		}
	}

	void FileDialog::m_loadPreview()
	{
		while (!m_previewThreadExit)
		{
			std::filesystem::path path;

			{
				std::unique_lock<std::mutex> lock(m_previewQueueMutex);
				m_previewCv.wait(lock, [this]()
								 { return m_previewThreadExit || !m_previewQueue.empty(); });

				if (m_previewThreadExit)
					break;

				if (!m_previewQueue.empty())
				{
					path = m_previewQueue.back(); // LIFO for faster apparent responsiveness
					m_previewQueue.pop_back();
				}
			}

			if (!path.empty())
			{
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
		}
	}
}