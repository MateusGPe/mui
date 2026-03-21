#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _WIN32
#define NOMINMAX
#include "mui_dialog.h"
#include <windows.h>
#include <shellapi.h>
#include <lmcons.h>
#include <algorithm>
#pragma comment(lib, "Shell32.lib")

namespace mui_dlg
{
	void FileDialog::m_initSystemFavorites()
	{
		FileTreeNode *quickAccess = new FileTreeNode("Quick Access");
		quickAccess->Read = true;
		m_treeCache.push_back(quickAccess);

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
	}

	void FileDialog::m_fixDrivePath(std::filesystem::path& p)
	{
		// drives don't work well without the backslash symbol
		if (p.string().size() == 2 && p.string()[1] == ':')
			p = std::filesystem::path(p.string() + "\\");
	}

	void *FileDialog::m_getIcon(const std::filesystem::path &path, bool isDirectory)
	{
		if (m_icons.count(path.string()) > 0)
			return m_icons[path.string()];

		std::string pathU8 = path.string();

		std::error_code ec;
		m_icons[pathU8] = nullptr;

		DWORD attrs = 0;
		UINT flags = SHGFI_ICON | SHGFI_LARGEICON;
		if (!std::filesystem::exists(path, ec))
		{
			flags |= SHGFI_USEFILEATTRIBUTES;
			attrs = FILE_ATTRIBUTE_DIRECTORY;
		}

		SHFILEINFOW fileInfo = {0};
		std::wstring pathW = path.wstring();
		for (size_t i = 0; i < pathW.size(); i++)
			if (pathW[i] == '/')
				pathW[i] = '\\';
		SHGetFileInfoW(pathW.c_str(), attrs, &fileInfo, sizeof(SHFILEINFOW), flags);

		if (fileInfo.hIcon == nullptr)
			return nullptr;

		// check if icon is already loaded
		auto itr = std::find(m_iconIndices.begin(), m_iconIndices.end(), fileInfo.iIcon);
		if (itr != m_iconIndices.end())
		{
			const std::string &existingIconFilepath = m_iconFilepaths[itr - m_iconIndices.begin()];
			m_icons[pathU8] = m_icons[existingIconFilepath];
			return m_icons[pathU8];
		}

		m_iconIndices.push_back(fileInfo.iIcon);
		m_iconFilepaths.push_back(pathU8);

		ICONINFO iconInfo = {0};
		GetIconInfo(fileInfo.hIcon, &iconInfo);

		if (iconInfo.hbmColor == nullptr)
			return nullptr;

		DIBSECTION ds;
		GetObject(iconInfo.hbmColor, sizeof(ds), &ds);
		int byteSize = ds.dsBm.bmWidth * ds.dsBm.bmHeight * (ds.dsBm.bmBitsPixel / 8);

		if (byteSize == 0)
			return nullptr;

		uint8_t *data = (uint8_t *)malloc(byteSize);
		GetBitmapBits(iconInfo.hbmColor, byteSize, data);

		m_icons[pathU8] = this->CreateTexture(data, ds.dsBm.bmWidth, ds.dsBm.bmHeight, 0);

		free(data);

		return m_icons[pathU8];
	}
}
#endif