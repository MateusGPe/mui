#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _WIN32

#include "mui_dialog.h"
#include <unistd.h>
#include <pwd.h>

namespace ifd
{
	void FileDialog::m_initSystemFavorites()
	{
		FileTreeNode *quickAccess = new FileTreeNode("Quick Access");
		quickAccess->Read = true;
		m_treeCache.push_back(quickAccess);

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
				thisPC->Children.push_back(new FileTreeNode(entry.path().string()));
		}
		m_treeCache.push_back(thisPC);
	}

	void FileDialog::m_fixDrivePath(std::filesystem::path& p)
	{
		// Not needed on POSIX
	}

	void *FileDialog::m_getIcon(const std::filesystem::path &path, bool isDirectory)
	{
		// Naturally fall back to FontAwesome
		return nullptr;
	}
}
#endif