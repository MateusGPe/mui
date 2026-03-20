// src/ImFileDialog/ImFileDialog.cpp
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "ImFileDialog.h"
#include "../../core/app.hpp" // Added for mui::App::assertMainThread()
#include <imgui.h>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <lmcons.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

namespace ifd
{
	FileDialog::FileDialog()
	{
		mui::App::assertMainThread();

		m_isOpen = false;
		m_type = 0;
		m_calledOpenPopup = false;
		m_sortColumn = 0;
		m_sortDirection = ImGuiSortDirection_Ascending;
		m_filterSelection = 0;
		m_inputTextbox[0] = 0;
		m_pathBuffer[0] = 0;
		m_searchBuffer[0] = 0;
		m_newEntryBuffer[0] = 0;
		m_selectedFileItem = -1;
		m_zoom = 1.0f;

		m_previewLoader = nullptr;
		m_previewLoaderRunning = false;

		m_setDirectory(std::filesystem::current_path(), false);

		// favorites are available on every OS
		FileTreeNode *quickAccess = new FileTreeNode("Quick Access");
		quickAccess->Read = true;
		m_treeCache.push_back(quickAccess);

#ifdef _WIN32
		wchar_t username[UNLEN + 1] = {0};
		DWORD username_len = UNLEN + 1;
		GetUserNameW(username, &username_len);

		std::wstring userPath = L"C:\\Users\\" + std::wstring(username) + L"\\";

		// Quick Access / Bookmarks
		quickAccess->Children.push_back(new FileTreeNode(userPath + L"Desktop"));
		quickAccess->Children.push_back(new FileTreeNode(userPath + L"Documents"));
		quickAccess->Children.push_back(new FileTreeNode(userPath + L"Downloads"));
		quickAccess->Children.push_back(new FileTreeNode(userPath + L"Pictures"));

		// OneDrive
		FileTreeNode *oneDrive = new FileTreeNode(userPath + L"OneDrive");
		m_treeCache.push_back(oneDrive);

		// This PC
		FileTreeNode *thisPC = new FileTreeNode("This PC");
		thisPC->Read = true;
		if (std::filesystem::exists(userPath + L"3D Objects"))
			thisPC->Children.push_back(new FileTreeNode(userPath + L"3D Objects"));
		thisPC->Children.push_back(new FileTreeNode(userPath + L"Desktop"));
		thisPC->Children.push_back(new FileTreeNode(userPath + L"Documents"));
		thisPC->Children.push_back(new FileTreeNode(userPath + L"Downloads"));
		thisPC->Children.push_back(new FileTreeNode(userPath + L"Music"));
		thisPC->Children.push_back(new FileTreeNode(userPath + L"Pictures"));
		thisPC->Children.push_back(new FileTreeNode(userPath + L"Videos"));
		DWORD d = GetLogicalDrives();
		for (int i = 0; i < 26; i++)
			if (d & (1 << i))
				thisPC->Children.push_back(new FileTreeNode(std::string(1, 'A' + i) + ":"));
		m_treeCache.push_back(thisPC);
#else
		std::error_code ec;

		// Quick Access
		struct passwd *pw;
		uid_t uid;
		uid = geteuid();
		pw = getpwuid(uid);
		if (pw)
		{
			std::string homePath = "/home/" + std::string(pw->pw_name);

			if (std::filesystem::exists(homePath, ec))
				quickAccess->Children.push_back(new FileTreeNode(homePath));
			if (std::filesystem::exists(homePath + "/Desktop", ec))
				quickAccess->Children.push_back(new FileTreeNode(homePath + "/Desktop"));
			if (std::filesystem::exists(homePath + "/Documents", ec))
				quickAccess->Children.push_back(new FileTreeNode(homePath + "/Documents"));
			if (std::filesystem::exists(homePath + "/Downloads", ec))
				quickAccess->Children.push_back(new FileTreeNode(homePath + "/Downloads"));
			if (std::filesystem::exists(homePath + "/Pictures", ec))
				quickAccess->Children.push_back(new FileTreeNode(homePath + "/Pictures"));
		}

		// This PC
		FileTreeNode *thisPC = new FileTreeNode("This PC");
		thisPC->Read = true;
		for (const auto &entry : std::filesystem::directory_iterator("/", ec))
		{
			if (std::filesystem::is_directory(entry, ec))
				thisPC->Children.push_back(new FileTreeNode(entry.path().u8string()));
		}
		m_treeCache.push_back(thisPC);
#endif
	}

	FileDialog::~FileDialog()
	{
		m_clearIconPreview();
		m_clearIcons();

		for (auto fn : m_treeCache)
			m_clearTree(fn);
		m_treeCache.clear();
	}

	bool FileDialog::Save(const std::string &key, const std::string &title, const std::string &filter, const std::string &startingDir)
	{
		mui::App::assertMainThread();

		if (!m_currentKey.empty())
			return false;

		m_currentKey = key;
		m_currentTitle = title + "###" + key;
		m_isOpen = true;
		m_calledOpenPopup = false;
		m_result.clear();
		m_inputTextbox[0] = 0;
		m_selections.clear();
		m_selectedFileItem = -1;
		m_isMultiselect = false;
		m_type = IFD_DIALOG_SAVE;

		m_parseFilter(filter);
		if (!startingDir.empty())
			m_setDirectory(std::filesystem::u8path(startingDir), false);
		else
			m_setDirectory(m_currentDirectory, false); // refresh contents

		return true;
	}

	bool FileDialog::Open(const std::string &key, const std::string &title, const std::string &filter, bool isMultiselect, const std::string &startingDir)
	{
		mui::App::assertMainThread();

		if (!m_currentKey.empty())
			return false;

		m_currentKey = key;
		m_currentTitle = title + "###" + key;
		m_isOpen = true;
		m_calledOpenPopup = false;
		m_result.clear();
		m_inputTextbox[0] = 0;
		m_selections.clear();
		m_selectedFileItem = -1;
		m_isMultiselect = isMultiselect;
		m_type = filter.empty() ? IFD_DIALOG_DIRECTORY : IFD_DIALOG_FILE;

		m_parseFilter(filter);
		if (!startingDir.empty())
			m_setDirectory(std::filesystem::u8path(startingDir), false);
		else
			m_setDirectory(m_currentDirectory, false); // refresh contents

		return true;
	}

	bool FileDialog::IsDone(const std::string &key)
	{
		mui::App::assertMainThread();

		bool isMe = m_currentKey == key;

		if (isMe && m_isOpen)
		{
			if (!m_calledOpenPopup)
			{
				ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
				ImGui::OpenPopup(m_currentTitle.c_str());
				m_calledOpenPopup = true;
			}

			if (ImGui::BeginPopupModal(m_currentTitle.c_str(), &m_isOpen, ImGuiWindowFlags_NoScrollbar))
			{
				m_renderFileDialog();
				ImGui::EndPopup();
			}
			else
				m_isOpen = false;
		}

		return isMe && !m_isOpen;
	}

	void FileDialog::Close()
	{
		mui::App::assertMainThread();

		m_currentKey.clear();
		m_backHistory = std::stack<std::filesystem::path>();
		m_forwardHistory = std::stack<std::filesystem::path>();

		// clear the tree
		for (auto fn : m_treeCache)
		{
			for (auto item : fn->Children)
			{
				for (auto ch : item->Children)
					m_clearTree(ch);
				item->Children.clear();
				item->Read = false;
			}
		}

		// free icon textures
		m_clearIconPreview();
		m_clearIcons();
	}

	void FileDialog::RemoveFavorite(const std::string &path)
	{
		mui::App::assertMainThread();

		auto itr = std::find(m_favorites.begin(), m_favorites.end(), m_currentDirectory.u8string());

		if (itr != m_favorites.end())
			m_favorites.erase(itr);

		// remove from sidebar
		for (auto &p : m_treeCache)
			if (p->Path == "Quick Access")
			{
				for (size_t i = 0; i < p->Children.size(); i++)
					if (p->Children[i]->Path == path)
					{
						p->Children.erase(p->Children.begin() + i);
						break;
					}
				break;
			}
	}

	void FileDialog::AddFavorite(const std::string &path)
	{
		mui::App::assertMainThread();

		if (std::count(m_favorites.begin(), m_favorites.end(), path) > 0)
			return;

		if (!std::filesystem::exists(std::filesystem::u8path(path)))
			return;

		m_favorites.push_back(path);

		// add to sidebar
		for (auto &p : m_treeCache)
			if (p->Path == "Quick Access")
			{
				p->Children.push_back(new FileTreeNode(path));
				break;
			}
	}

	void FileDialog::m_select(const std::filesystem::path &path, bool isCtrlDown)
	{
		bool multiselect = isCtrlDown && m_isMultiselect;

		if (!multiselect)
		{
			m_selections.clear();
			m_selections.push_back(path);
		}
		else
		{
			auto it = std::find(m_selections.begin(), m_selections.end(), path);
			if (it != m_selections.end())
				m_selections.erase(it);
			else
				m_selections.push_back(path);
		}

		if (m_selections.size() == 1)
		{
			std::string filename = m_selections[0].filename().u8string();
			if (filename.size() == 0)
				filename = m_selections[0].u8string(); // drive

			strcpy(m_inputTextbox, filename.c_str());
		}
		else
		{
			std::string textboxVal = "";
			for (const auto &sel : m_selections)
			{
				std::string filename = sel.filename().u8string();
				if (filename.size() == 0)
					filename = sel.u8string();

				textboxVal += "\"" + filename + "\", ";
			}
			strcpy(m_inputTextbox, textboxVal.substr(0, textboxVal.size() - 2).c_str());
		}
	}

	bool FileDialog::m_finalize(const std::string &filename)
	{
		bool hasResult = (!filename.empty() && m_type != IFD_DIALOG_DIRECTORY) || m_type == IFD_DIALOG_DIRECTORY;

		if (hasResult)
		{
			if (!m_isMultiselect || m_selections.size() <= 1)
			{
				std::filesystem::path path = std::filesystem::u8path(filename);
				if (path.is_absolute())
					m_result.push_back(path);
				else
					m_result.push_back(m_currentDirectory / path);
				if (m_type == IFD_DIALOG_DIRECTORY || m_type == IFD_DIALOG_FILE)
				{
					if (!std::filesystem::exists(m_result.back()))
					{
						m_result.clear();
						return false;
					}
				}
			}
			else
			{
				for (const auto &sel : m_selections)
				{
					if (sel.is_absolute())
						m_result.push_back(sel);
					else
						m_result.push_back(m_currentDirectory / sel);
					if (m_type == IFD_DIALOG_DIRECTORY || m_type == IFD_DIALOG_FILE)
					{
						if (!std::filesystem::exists(m_result.back()))
						{
							m_result.clear();
							return false;
						}
					}
				}
			}

			if (m_type == IFD_DIALOG_SAVE)
			{
				// add the extension
				if (m_filterSelection < m_filterExtensions.size() && m_filterExtensions[m_filterSelection].size() > 0)
				{
					if (!m_result.back().has_extension())
					{
						std::string extAdd = m_filterExtensions[m_filterSelection][0];
						m_result.back().replace_extension(extAdd);
					}
				}
			}
		}

		m_isOpen = false;

		return true;
	}

	void FileDialog::m_parseFilter(const std::string &filter)
	{
		m_filter = "";
		m_filterExtensions.clear();
		m_filterSelection = 0;

		if (filter.empty())
			return;

		std::vector<std::string> exts;

		size_t lastSplit = 0, lastExt = 0;
		bool inExtList = false;
		for (size_t i = 0; i < filter.size(); i++)
		{
			if (filter[i] == ',')
			{
				if (!inExtList)
					lastSplit = i + 1;
				else
				{
					exts.push_back(filter.substr(lastExt, i - lastExt));
					lastExt = i + 1;
				}
			}
			else if (filter[i] == '{')
			{
				std::string filterName = filter.substr(lastSplit, i - lastSplit);
				if (filterName == ".*")
				{
					m_filter += std::string(std::string("All Files (*.*)\0").c_str(), 16);
					m_filterExtensions.push_back(std::vector<std::string>());
				}
				else
					m_filter += std::string((filterName + "\0").c_str(), filterName.size() + 1);
				inExtList = true;
				lastExt = i + 1;
			}
			else if (filter[i] == '}')
			{
				exts.push_back(filter.substr(lastExt, i - lastExt));
				m_filterExtensions.push_back(exts);
				exts.clear();

				inExtList = false;
			}
		}
		if (lastSplit != 0)
		{
			std::string filterName = filter.substr(lastSplit);
			if (filterName == ".*")
			{
				m_filter += std::string(std::string("All Files (*.*)\0").c_str(), 16);
				m_filterExtensions.push_back(std::vector<std::string>());
			}
			else
				m_filter += std::string((filterName + "\0").c_str(), filterName.size() + 1);
		}
	}

	void FileDialog::m_clearTree(FileTreeNode *node)
	{
		if (node == nullptr)
			return;

		for (auto n : node->Children)
			m_clearTree(n);

		delete node;
		node = nullptr;
	}
}
