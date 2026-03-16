#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImFileDialog.h"
#include "../core/app.hpp" // Added for mui::App::assertMainThread()
#include "IconsFontAwesome6.h"
#include "stb_image.h"

#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <imgui.h>
#include <imgui_internal.h>



#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <lmcons.h>
#pragma comment(lib, "Shell32.lib")
#else
#include <unistd.h>
#include <pwd.h>
#endif

#ifndef ICON_FA_FOLDER
#define ICON_FA_FOLDER "\xef\x81\xbb"
#endif
#ifndef ICON_FA_FILE
#define ICON_FA_FILE "\xef\x85\x9b"
#endif

#define ICON_SIZE ImGui::GetFontSize() + 3
#define GUI_ELEMENT_SIZE std::max(ImGui::GetFontSize() + 10.f, 24.f)
#define DEFAULT_ICON_SIZE 32
#define PI 3.141592f

namespace ifd
{
	/* UI CONTROLS */
	bool FolderNode(const char *label, ImTextureID icon, bool &clicked)
	{
		ImGuiContext &g = *GImGui;
		ImGuiWindow *window = ImGui::GetCurrentWindow();

		clicked = false;

		ImU32 id = window->GetID(label);
		int opened = window->StateStorage.GetInt(id, 0);
		ImVec2 pos = ImGui::GetCursorScreenPos();
		const bool is_mouse_x_over_arrow = (ImGui::GetIO().MousePos.x >= pos.x && ImGui::GetIO().MousePos.x < pos.x + ImGui::GetFontSize());
		if (ImGui::InvisibleButton(label, ImVec2(-FLT_MIN, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2)))
		{
			if (is_mouse_x_over_arrow)
			{
				int *p_opened = window->StateStorage.GetIntRef(id, 0);
				opened = *p_opened = !*p_opened;
			}
			else
			{
				clicked = true;
			}
		}
		bool hovered = ImGui::IsItemHovered();
		bool active = ImGui::IsItemActive();
		bool doubleClick = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
		if (doubleClick && hovered)
		{
			int *p_opened = window->StateStorage.GetIntRef(id, 0);
			opened = *p_opened = !*p_opened;
			clicked = false;
		}
		if (hovered || active)
			window->DrawList->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]));

		// Icon, text
		float icon_posX = pos.x + ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y;
		float text_posX = icon_posX + ImGui::GetStyle().FramePadding.y + ICON_SIZE;
		ImGui::RenderArrow(window->DrawList, ImVec2(pos.x, pos.y + ImGui::GetStyle().FramePadding.y), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[((hovered && is_mouse_x_over_arrow) || opened) ? ImGuiCol_Text : ImGuiCol_TextDisabled]), opened ? ImGuiDir_Down : ImGuiDir_Right);

		if (icon)
		{
			window->DrawList->AddImage(icon, ImVec2(icon_posX, pos.y), ImVec2(icon_posX + ICON_SIZE, pos.y + ICON_SIZE));
		}
		else
		{
			window->DrawList->AddText(ImGui::GetFont(), ICON_SIZE, ImVec2(icon_posX, pos.y + ImGui::GetStyle().FramePadding.y), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), ICON_FA_FOLDER);
		}

		ImGui::RenderText(ImVec2(text_posX, pos.y + ImGui::GetStyle().FramePadding.y), label);
		if (opened)
			ImGui::TreePush(label);
		return opened != 0;
	}
	bool FileNode(const char *label, ImTextureID icon)
	{
		ImGuiWindow *window = ImGui::GetCurrentWindow();

		// ImU32 id = window->GetID(label);
		ImVec2 pos = ImGui::GetCursorScreenPos();
		bool ret = ImGui::InvisibleButton(label, ImVec2(-FLT_MIN, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2));

		bool hovered = ImGui::IsItemHovered();
		bool active = ImGui::IsItemActive();
		if (hovered || active)
			window->DrawList->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]));

		// Icon, text
		if (icon)
		{
			window->DrawList->AddImage(icon, ImVec2(pos.x, pos.y), ImVec2(pos.x + ICON_SIZE, pos.y + ICON_SIZE));
		}
		else
		{
			window->DrawList->AddText(ImGui::GetFont(), ICON_SIZE, ImVec2(pos.x, pos.y + ImGui::GetStyle().FramePadding.y), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), ICON_FA_FILE);
		}

		ImGui::RenderText(ImVec2(pos.x + ImGui::GetStyle().FramePadding.y + ICON_SIZE, pos.y + ImGui::GetStyle().FramePadding.y), label);

		return ret;
	}
	bool PathBox(const char *label, std::filesystem::path &path, char *pathBuffer, ImVec2 size_arg)
	{
		ImGuiWindow *window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		bool ret = false;
		const ImGuiID id = window->GetID(label);
		int *state = window->StateStorage.GetIntRef(id, 0);

		ImGui::SameLine();

		const ImGuiStyle &style = ImGui::GetStyle();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 uiPos = ImGui::GetCursorPos();
		ImVec2 size = ImGui::CalcItemSize(size_arg, 200, GUI_ELEMENT_SIZE);
		const ImRect bb(pos, pos + size);

		// buttons
		if (!(*state & 0b001))
		{
			ImGui::PushClipRect(bb.Min, bb.Max, false);

			// background
			bool hovered = ImGui::IsMouseHoveringRect(bb.Min, bb.Max);
			bool clicked = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);
			bool anyOtherHC = false; // are any other items hovered or clicked?
			window->DrawList->AddRectFilled(pos, pos + size, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[(*state & 0b10) ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg]));

			// fetch the buttons (so that we can throw some away if needed)
			std::vector<std::pair<std::string, float>> btnList;
			float totalWidth = 0.0f;
			for (auto comp : path)
			{
				std::string section = comp.u8string();
				if (section.size() == 1 && (section[0] == '\\' || section[0] == '/'))
					continue;

				float elSize = ImGui::CalcTextSize(section.c_str()).x + style.FramePadding.x * 2.0f + GUI_ELEMENT_SIZE;
				totalWidth += elSize;
				btnList.push_back({section, elSize});
			}
			totalWidth -= GUI_ELEMENT_SIZE;

			// UI buttons
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
			bool isFirstElement = true;
			for (size_t i = 0; i < btnList.size(); i++)
			{
				if (totalWidth > size.x - 30 && i != btnList.size() - 1)
				{ // trim some buttons if there's not enough space
					totalWidth -= btnList[i].second;
					continue;
				}

				ImGui::PushID(static_cast<int>(i));
				if (!isFirstElement)
				{
					ImGui::ArrowButtonEx("##dir_dropdown", ImGuiDir_Right, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE));
					anyOtherHC |= ImGui::IsItemHovered() | ImGui::IsItemClicked();
					ImGui::SameLine();
				}
				if (ImGui::Button(btnList[i].first.c_str(), ImVec2(0, GUI_ELEMENT_SIZE)))
				{
#ifdef _WIN32
					std::string newPath = "";
#else
					std::string newPath = "/";
#endif
					for (size_t j = 0; j <= i; j++)
					{
						newPath += btnList[j].first;
#ifdef _WIN32
						if (j != i)
							newPath += "\\";
#else
						if (j != i)
							newPath += "/";
#endif
					}
					path = std::filesystem::u8path(newPath);
					ret = true;
				}
				anyOtherHC |= ImGui::IsItemHovered() | ImGui::IsItemClicked();
				ImGui::SameLine();
				ImGui::PopID();

				isFirstElement = false;
			}
			ImGui::PopStyleVar(2);

			// click state
			if (!anyOtherHC && clicked)
			{
				strcpy(pathBuffer, path.u8string().c_str());
				*state |= 0b001;
				*state &= 0b011; // remove SetKeyboardFocus flag
			}
			else
				*state &= 0b110;

			// hover state
			if (!anyOtherHC && hovered && !clicked)
				*state |= 0b010;
			else
				*state &= 0b101;

			ImGui::PopClipRect();

			// allocate space
			ImGui::SetCursorPos(uiPos);
			ImGui::ItemSize(size);
		}
		// input box
		else
		{
			bool skipActiveCheck = false;
			if (!(*state & 0b100))
			{
				skipActiveCheck = true;
				ImGui::SetKeyboardFocusHere();
				if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					*state |= 0b100;
			}
			if (ImGui::InputTextEx("##pathbox_input", "", pathBuffer, 1024, size_arg, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				std::string tempStr(pathBuffer);
				if (std::filesystem::exists(tempStr))
					path = std::filesystem::u8path(tempStr);
				ret = true;
			}
			if (!skipActiveCheck && !ImGui::IsItemActive())
				*state &= 0b010;
		}

		return ret;
	}
	bool FavoriteButton(const char *label, bool isFavorite)
	{
		ImGuiWindow *window = ImGui::GetCurrentWindow();

		ImVec2 pos = ImGui::GetCursorScreenPos();
		bool ret = ImGui::InvisibleButton(label, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE));

		bool hovered = ImGui::IsItemHovered();
		bool active = ImGui::IsItemActive();

		ImU32 color;
		if (isFavorite)
		{
			color = 0xEE0E89BB; // Yellowish
		}
		else if (hovered || active)
		{
			color = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]);
		}
		else
		{
			color = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
		}

		ImFont* font = ImGui::GetFont();
		float fontSize = ImGui::GetFontSize();

		static ImVec2 textSize = {0,0};
		static ImFont* lastFont = nullptr;
		static float lastFontSize = 0.0f;
		if (lastFont != font || lastFontSize != fontSize) {
			textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, ICON_FA_STAR);
			lastFont = font;
			lastFontSize = fontSize;
		}
		ImVec2 textPos = ImVec2(pos.x + (GUI_ELEMENT_SIZE - textSize.x) / 2.0f, pos.y + (GUI_ELEMENT_SIZE - textSize.y) / 2.0f);
		
		window->DrawList->AddText(font, fontSize, textPos, color, ICON_FA_STAR);

		return ret;
	}
	bool FileIcon(const char *label, bool isSelected, ImTextureID icon, bool isDirectory, ImVec2 size, bool hasPreview, int previewWidth, int previewHeight)
	{
		ImGuiStyle &style = ImGui::GetStyle();
		ImGuiWindow *window = ImGui::GetCurrentWindow();

		float windowSpace = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
		ImVec2 pos = ImGui::GetCursorScreenPos();
		bool ret = false;

		if (ImGui::InvisibleButton(label, size))
			ret = true;

		bool hovered = ImGui::IsItemHovered();
		bool active = ImGui::IsItemActive();
		bool doubleClick = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
		if (doubleClick && hovered)
			ret = true;

		float iconSize = size.y - ImGui::GetFontSize() * 2;
		float iconPosX = pos.x + (size.x - iconSize) / 2.0f;
		ImVec2 textSize = ImGui::CalcTextSize(label, 0, true, size.x);

		if (hovered || active || isSelected)
			window->DrawList->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : (isSelected ? ImGuiCol_Header : ImGuiCol_HeaderHovered)]));

		if (hasPreview && icon)
		{
			ImVec2 availSize = ImVec2(size.x, iconSize);
			const float padx(4), pady(4);
			float scale = std::min<float>(availSize.x / previewWidth, availSize.y / previewHeight);
			availSize.x = previewWidth * scale;
			availSize.y = previewHeight * scale;

			float previewPosX = pos.x + (size.x - availSize.x) / 2.0f + padx;
			float previewPosY = pos.y + (iconSize - availSize.y) / 2.0f + padx;
			availSize.x -= padx * 2;
			availSize.y -= pady * 2;
			window->DrawList->AddImage(icon, ImVec2(previewPosX, previewPosY), ImVec2(previewPosX + availSize.x, previewPosY + availSize.y));
		}
		else if (icon)
		{
			window->DrawList->AddImage(icon, ImVec2(iconPosX, pos.y), ImVec2(iconPosX + iconSize, pos.y + iconSize));
		}
		else
		{
			// Fallback to font awesome
			static struct {
				float lastIconSize = -1.0f;
				float lastSizeX = -1.0f;
				ImFont* lastFont = nullptr;
				float finalIconSize = 0.0f;
				ImVec2 textSize = ImVec2(0, 0);
			} iconCache[2];

			int cacheIdx = isDirectory ? 1 : 0;
			const char* icon_text = isDirectory ? ICON_FA_FOLDER : ICON_FA_FILE;
			ImFont* currentFont = ImGui::GetFont();

			if (iconCache[cacheIdx].lastIconSize != iconSize || iconCache[cacheIdx].lastSizeX != size.x || iconCache[cacheIdx].lastFont != currentFont) {
				ImVec2 text_size = currentFont->CalcTextSizeA(iconSize, FLT_MAX, 0.0f, icon_text);
				float scale = 1.0f;
				if (text_size.x > 0.0f) scale = std::min<float>(scale, size.x / text_size.x);
				if (text_size.y > 0.0f) scale = std::min<float>(scale, iconSize / text_size.y);
				iconCache[cacheIdx].finalIconSize = iconSize * scale * 0.8f;
				iconCache[cacheIdx].textSize = currentFont->CalcTextSizeA(iconCache[cacheIdx].finalIconSize, FLT_MAX, 0.0f, icon_text);
				iconCache[cacheIdx].lastIconSize = iconSize;
				iconCache[cacheIdx].lastSizeX = size.x;
				iconCache[cacheIdx].lastFont = currentFont;
			}
			
			float textPosX = pos.x + (size.x - iconCache[cacheIdx].textSize.x) / 2.0f;
			float textPosY = pos.y + (iconSize - iconCache[cacheIdx].textSize.y) / 2.0f;
			window->DrawList->AddText(currentFont, iconCache[cacheIdx].finalIconSize, ImVec2(textPosX, textPosY), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), icon_text);
		}

		window->DrawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(pos.x + (size.x - textSize.x) / 2.0f, pos.y + iconSize), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), label, 0, size.x);

		float lastButtomPos = ImGui::GetItemRectMax().x;
		float thisButtonPos = lastButtomPos + style.ItemSpacing.x + size.x; // Expected position if next button was on same line
		if (thisButtonPos < windowSpace)
			ImGui::SameLine();

		return ret;
	}

	FileDialog::FileData::FileData(const std::filesystem::path &path)
	{
		Path = path;

		struct stat attr;
		if (stat(path.u8string().c_str(), &attr) == 0)
		{
			IsDirectory = S_ISDIR(attr.st_mode);
			Size = attr.st_size;
			DateModified = attr.st_mtime; // Use mtime for modification time
		}
		else
		{
			std::error_code ec;
			IsDirectory = std::filesystem::is_directory(path, ec);
			Size = std::filesystem::file_size(path, ec);
			DateModified = 0;
		}

		HasIconPreview = false;
		IconPreview = nullptr;
		IconPreviewData = nullptr;
		IconPreviewHeight = 0;
		IconPreviewWidth = 0;
	}

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

	void *FileDialog::m_getIcon(const std::filesystem::path &path, bool isDirectory)
	{
#ifdef _WIN32
		if (m_icons.count(path.u8string()) > 0)
			return m_icons[path.u8string()];

		std::string pathU8 = path.u8string();

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
		for (int i = 0; i < pathW.size(); i++)
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
#else
		// Naturally fall back to FontAwesome
		return nullptr;
#endif
	}
	void FileDialog::m_clearIcons()
	{
		std::vector<unsigned int> deletedIcons;

		// delete textures
		for (auto &icon : m_icons)
		{
			unsigned int ptr = (unsigned int)((uintptr_t)icon.second);
			if (std::count(deletedIcons.begin(), deletedIcons.end(), ptr)) // skip duplicates
				continue;

			deletedIcons.push_back(ptr);
			DeleteTexture(icon.second);
		}
		m_iconFilepaths.clear();
		m_iconIndices.clear();
		m_icons.clear();
	}
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
			FileData dataCopy(std::filesystem::u8path(""));
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
				std::string ext = dataCopy.Path.extension().u8string();
				if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
				{
					int width, height, nrChannels;
					unsigned char *image = stbi_load(dataCopy.Path.u8string().c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);

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
	void FileDialog::m_clearTree(FileTreeNode *node)
	{
		if (node == nullptr)
			return;

		for (auto n : node->Children)
			m_clearTree(n);

		delete node;
		node = nullptr;
	}
	void FileDialog::m_setDirectory(const std::filesystem::path &p, bool addHistory)
	{
		std::lock_guard<std::recursive_mutex> lock(m_contentMutex);

		bool isSameDir = m_currentDirectory == p;

		if (addHistory && !isSameDir)
			m_backHistory.push(m_currentDirectory);

		m_currentDirectory = p;
#ifdef _WIN32
		// drives don't work well without the backslash symbol
		if (p.u8string().size() == 2 && p.u8string()[1] == ':')
			m_currentDirectory = std::filesystem::u8path(p.u8string() + "\\");
#endif

		m_clearIconPreview();
		m_content.clear(); // p == "" after this line, due to reference
		m_selectedFileItem = -1;

		if (m_type == IFD_DIALOG_DIRECTORY || m_type == IFD_DIALOG_FILE)
			m_inputTextbox[0] = 0;
		m_selections.clear();

		if (!isSameDir)
		{
			m_searchBuffer[0] = 0;
			m_clearIcons();
		}

		if (m_currentDirectory.u8string() == "Quick Access")
		{
			for (auto &node : m_treeCache)
			{
				if (node->Path == m_currentDirectory)
					for (auto &c : node->Children)
						m_content.push_back(FileData(c->Path));
			}
		}
		else if (m_currentDirectory.u8string() == "This PC")
		{
			for (auto &node : m_treeCache)
			{
				if (node->Path == m_currentDirectory)
					for (auto &c : node->Children)
						m_content.push_back(FileData(c->Path));
			}
		}
		else
		{
			std::error_code ec;
			if (std::filesystem::exists(m_currentDirectory, ec))
				for (const auto &entry : std::filesystem::directory_iterator(m_currentDirectory, ec))
				{
					FileData info(entry.path());

					// skip files when IFD_DIALOG_DIRECTORY
					if (!info.IsDirectory && m_type == IFD_DIALOG_DIRECTORY)
						continue;

					// check if filename matches search query
					if (m_searchBuffer[0])
					{
						std::string filename = info.Path.u8string();

						std::string filenameSearch = filename;
						std::string query(m_searchBuffer);
						std::transform(filenameSearch.begin(), filenameSearch.end(), filenameSearch.begin(), ::tolower);
						std::transform(query.begin(), query.end(), query.begin(), ::tolower);

						if (filenameSearch.find(query, 0) == std::string::npos)
							continue;
					}

					// check if extension matches
					if (!info.IsDirectory && m_type != IFD_DIALOG_DIRECTORY)
					{
						if (m_filterSelection < m_filterExtensions.size())
						{
							const auto &exts = m_filterExtensions[m_filterSelection];
							if (exts.size() > 0)
							{
								std::string extension = info.Path.extension().u8string();
								bool isWildcard = (std::count(exts.begin(), exts.end(), "*.*") > 0);

								// extension not found? skip
								// if it's not a wildcard filter and the extension doesn't match, skip it
								if (!isWildcard && std::count(exts.begin(), exts.end(), extension) == 0)
									continue;
							}
						}
					}

					m_content.push_back(info);
				}
		}

		m_sortContent(m_sortColumn, m_sortDirection);
		m_refreshIconPreview();
	}
	void FileDialog::m_sortContent(unsigned int column, unsigned int sortDirection)
	{
		std::lock_guard<std::recursive_mutex> lock(m_contentMutex);

		// 0 -> name, 1 -> date, 2 -> size
		m_sortColumn = column;
		m_sortDirection = sortDirection;

		// split into directories and files
		std::partition(m_content.begin(), m_content.end(), [](const FileData &data)
					   { return data.IsDirectory; });

		if (m_content.size() > 0)
		{
			// find where the file list starts
			size_t fileIndex = 0;
			for (; fileIndex < m_content.size(); fileIndex++)
				if (!m_content[fileIndex].IsDirectory)
					break;

			// compare function
			auto compareFn = [column, sortDirection](const FileData &left, const FileData &right) -> bool
			{
				// name
				if (column == 0)
				{
					std::string lName = left.Path.u8string();
					std::string rName = right.Path.u8string();

					std::transform(lName.begin(), lName.end(), lName.begin(), ::tolower);
					std::transform(rName.begin(), rName.end(), rName.begin(), ::tolower);

					int comp = lName.compare(rName);

					if (sortDirection == ImGuiSortDirection_Ascending)
						return comp < 0;
					return comp > 0;
				}
				// date
				else if (column == 1)
				{
					if (sortDirection == ImGuiSortDirection_Ascending)
						return left.DateModified < right.DateModified;
					else
						return left.DateModified > right.DateModified;
				}
				// size
				else if (column == 2)
				{
					if (sortDirection == ImGuiSortDirection_Ascending)
						return left.Size < right.Size;
					else
						return left.Size > right.Size;
				}

				return false;
			};

			// sort the directories
			std::sort(m_content.begin(), m_content.begin() + fileIndex, compareFn);

			// sort the files
			std::sort(m_content.begin() + fileIndex, m_content.end(), compareFn);
		}
	}

	void FileDialog::m_renderTree(FileTreeNode *node)
	{
		if (node == nullptr)
			return;

		// directory
		std::error_code ec;
		ImGui::PushID(node);
		bool isClicked = false;
		std::string displayName = node->Path.stem().u8string();
		if (displayName.size() == 0)
			displayName = node->Path.u8string();
		if (FolderNode(displayName.c_str(), (ImTextureID)(uintptr_t)m_getIcon(node->Path, true), isClicked))
		{
			if (!node->Read)
			{
				// cache children if it's not already cached
				if (std::filesystem::exists(node->Path, ec))
					for (const auto &entry : std::filesystem::directory_iterator(node->Path, ec))
					{
						if (std::filesystem::is_directory(entry, ec))
							node->Children.push_back(new FileTreeNode(entry.path().u8string()));
					}
				node->Read = true;
			}

			// display children
			for (auto c : node->Children)
				m_renderTree(c);

			ImGui::TreePop();
		}
		if (isClicked)
			m_setDirectory(node->Path);
		ImGui::PopID();
	}
	void FileDialog::m_renderContent()
	{
		std::lock_guard<std::recursive_mutex> lock(m_contentMutex);

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			m_selectedFileItem = -1;

		// table view
		if (m_zoom == 1.0f)
		{
			if (ImGui::BeginTable("##contentTable", 3, /*ImGuiTableFlags_Resizable |*/ ImGuiTableFlags_Sortable, ImVec2(0, -FLT_MIN)))
			{
				// header
				ImGui::TableSetupColumn("Name##filename", ImGuiTableColumnFlags_WidthStretch, 0.0f - 1.0f, 0);
				ImGui::TableSetupColumn("Date modified##filedate", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 0.0f, 1);
				ImGui::TableSetupColumn("Size##filesize", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 0.0f, 2);
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableHeadersRow();

				// sort
				if (ImGuiTableSortSpecs *sortSpecs = ImGui::TableGetSortSpecs())
				{
					if (sortSpecs->SpecsDirty)
					{
						sortSpecs->SpecsDirty = false;
						m_sortContent(sortSpecs->Specs->ColumnUserID, sortSpecs->Specs->SortDirection);
					}
				}

				// content
				int fileId = 0;
				for (auto &entry : m_content)
				{
					std::string filename = entry.Path.filename().u8string();
					if (filename.size() == 0)
						filename = entry.Path.u8string(); // drive

					bool isSelected = std::count(m_selections.begin(), m_selections.end(), entry.Path);

					ImGui::TableNextRow();

					// file name
					ImGui::TableSetColumnIndex(0);
					void *iconTex = m_getIcon(entry.Path, entry.IsDirectory);
					if (iconTex)
					{
						ImGui::Image((ImTextureID)(uintptr_t)iconTex, ImVec2(ICON_SIZE, ICON_SIZE));
					}
					else
					{
						ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_Text], "%s", entry.IsDirectory ? ICON_FA_FOLDER : ICON_FA_FILE);
					}
					ImGui::SameLine();

					if (ImGui::Selectable(filename.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
					{
						std::error_code ec;
						bool isDir = std::filesystem::is_directory(entry.Path, ec);

						if (ImGui::IsMouseDoubleClicked(0))
						{
							if (isDir)
							{
								m_setDirectory(entry.Path);
								break;
							}
							else
								m_finalize(filename);
						}
						else
						{
							if ((isDir && m_type == IFD_DIALOG_DIRECTORY) || !isDir)
								m_select(entry.Path, ImGui::GetIO().KeyCtrl);
						}
					}
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
						m_selectedFileItem = fileId;
					fileId++;

					// date
					ImGui::TableSetColumnIndex(1);
					auto tm = std::localtime(&entry.DateModified);
					if (tm != nullptr)
						ImGui::Text("%d/%d/%d %02d:%02d", tm->tm_mon + 1, tm->tm_mday, 1900 + tm->tm_year, tm->tm_hour, tm->tm_min);
					else
						ImGui::Text("---");

					// size
					ImGui::TableSetColumnIndex(2);
					ImGui::Text("%.3f KiB", entry.Size / 1024.0f);
				}

				ImGui::EndTable();
			}
		}
		// "icon" view
		else
		{
			// content
			int fileId = 0;
			for (auto &entry : m_content)
			{
				if (entry.HasIconPreview && entry.IconPreviewData != nullptr)
				{
					entry.IconPreview = this->CreateTexture(entry.IconPreviewData, entry.IconPreviewWidth, entry.IconPreviewHeight, 1u);
					stbi_image_free(entry.IconPreviewData);
					entry.IconPreviewData = nullptr;
				}

				std::string filename = entry.Path.filename().u8string();
				if (filename.size() == 0)
					filename = entry.Path.u8string(); // drive

				bool isSelected = std::count(m_selections.begin(), m_selections.end(), entry.Path);

				if (FileIcon(filename.c_str(), isSelected, entry.HasIconPreview ? (ImTextureID)(uintptr_t)entry.IconPreview : (ImTextureID)(uintptr_t)m_getIcon(entry.Path, entry.IsDirectory), entry.IsDirectory, ImVec2(32 + 16 * m_zoom, 32 + 16 * m_zoom), entry.HasIconPreview, entry.IconPreviewWidth, entry.IconPreviewHeight))
				{
					bool isDir = entry.IsDirectory;

					if (ImGui::IsMouseDoubleClicked(0))
					{
						if (isDir)
						{
							m_setDirectory(entry.Path);
							break;
						}
						else
							m_finalize(filename);
					}
					else
					{
						if ((isDir && m_type == IFD_DIALOG_DIRECTORY) || !isDir)
							m_select(entry.Path, ImGui::GetIO().KeyCtrl);
					}
				}
				if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					m_selectedFileItem = fileId;
				fileId++;
			}
		}
	}
	void FileDialog::m_renderPopups()
	{
		std::lock_guard<std::recursive_mutex> lock(m_contentMutex);
		bool openAreYouSureDlg = false, openNewFileDlg = false, openNewDirectoryDlg = false;
		if (ImGui::BeginPopupContextItem("##dir_context"))
		{
			if (ImGui::Selectable("New file"))
				openNewFileDlg = true;
			if (ImGui::Selectable("New directory"))
				openNewDirectoryDlg = true;
			if (m_selectedFileItem != -1 && ImGui::Selectable("Delete"))
				openAreYouSureDlg = true;
			ImGui::EndPopup();
		}
		if (openAreYouSureDlg)
			ImGui::OpenPopup("Are you sure?##delete");
		if (openNewFileDlg)
			ImGui::OpenPopup("Enter file name##newfile");
		if (openNewDirectoryDlg)
			ImGui::OpenPopup("Enter directory name##newdir");

		if (ImGui::BeginPopupModal("Are you sure?##delete"))
		{
			if (m_selectedFileItem >= static_cast<int>(m_content.size()) || m_content.size() == 0)
				ImGui::CloseCurrentPopup();
			else
			{
				const FileData &data = m_content[m_selectedFileItem];
				ImGui::TextWrapped("Are you sure you want to delete %s?", data.Path.filename().u8string().c_str());
				if (ImGui::Button("Yes"))
				{
					std::error_code ec;
					std::filesystem::remove_all(data.Path, ec);
					m_setDirectory(m_currentDirectory, false); // refresh
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("No"))
					ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Enter file name##newfile"))
		{
			ImGui::PushItemWidth(250.0f);
			ImGui::InputText("##newfilename", m_newEntryBuffer, 1024); // TODO: remove hardcoded literals
			ImGui::PopItemWidth();

			if (ImGui::Button("OK"))
			{
				std::ofstream out((m_currentDirectory / std::string(m_newEntryBuffer)).string());
				out << "";
				out.close();

				m_setDirectory(m_currentDirectory, false); // refresh
				m_newEntryBuffer[0] = 0;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				m_newEntryBuffer[0] = 0;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Enter directory name##newdir"))
		{
			ImGui::PushItemWidth(250.0f);
			ImGui::InputText("##newfilename", m_newEntryBuffer, 1024); // TODO: remove hardcoded literals
			ImGui::PopItemWidth();

			if (ImGui::Button("OK"))
			{
				std::error_code ec;
				std::filesystem::create_directory(m_currentDirectory / std::string(m_newEntryBuffer), ec);

				m_setDirectory(m_currentDirectory, false); // refresh
				m_newEntryBuffer[0] = 0;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
				m_newEntryBuffer[0] = 0;
			}
			ImGui::EndPopup();
		}
	}
	void FileDialog::m_renderFileDialog()
	{
		/***** TOP BAR *****/
		bool noBackHistory = m_backHistory.empty(), noForwardHistory = m_forwardHistory.empty();

		ImGui::PushStyleColor(ImGuiCol_Button, 0);
		if (noBackHistory)
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		if (ImGui::ArrowButtonEx("##back", ImGuiDir_Left, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE), noBackHistory ? ImGuiItemFlags_Disabled : 0))
		{
			std::filesystem::path newPath = m_backHistory.top();
			m_backHistory.pop();
			m_forwardHistory.push(m_currentDirectory);

			m_setDirectory(newPath, false);
		}
		if (noBackHistory)
			ImGui::PopStyleVar();
		ImGui::SameLine();

		if (noForwardHistory)
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		if (ImGui::ArrowButtonEx("##forward", ImGuiDir_Right, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE), noForwardHistory ? ImGuiItemFlags_Disabled : 0))
		{
			std::filesystem::path newPath = m_forwardHistory.top();
			m_forwardHistory.pop();
			m_backHistory.push(m_currentDirectory);

			m_setDirectory(newPath, false);
		}
		if (noForwardHistory)
			ImGui::PopStyleVar();
		ImGui::SameLine();

		if (ImGui::ArrowButtonEx("##up", ImGuiDir_Up, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE)))
		{
			if (m_currentDirectory.has_parent_path())
				m_setDirectory(m_currentDirectory.parent_path());
		}

		std::filesystem::path curDirCopy = m_currentDirectory;
		if (PathBox("##pathbox", curDirCopy, m_pathBuffer, ImVec2(-250, GUI_ELEMENT_SIZE)))
			m_setDirectory(curDirCopy);
		ImGui::SameLine();

		if (FavoriteButton("##dirfav", std::count(m_favorites.begin(), m_favorites.end(), m_currentDirectory.u8string())))
		{
			if (std::count(m_favorites.begin(), m_favorites.end(), m_currentDirectory.u8string()))
				RemoveFavorite(m_currentDirectory.u8string());
			else
				AddFavorite(m_currentDirectory.u8string());
		}
		ImGui::SameLine();
		ImGui::PopStyleColor();

		if (ImGui::InputTextEx("##searchTB", "Search", m_searchBuffer, 128, ImVec2(-FLT_MIN, GUI_ELEMENT_SIZE), 0)) // TODO: no hardcoded literals
			m_setDirectory(m_currentDirectory, false);																// refresh

		/***** CONTENT *****/
		float bottomBarHeight = (ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y + ImGui::GetStyle().ItemSpacing.y * 2.0f) * 2;
		if (ImGui::BeginTable("##table", 2, ImGuiTableFlags_Resizable, ImVec2(0, -bottomBarHeight)))
		{
			ImGui::TableSetupColumn("##tree", ImGuiTableColumnFlags_WidthFixed, 125.0f);
			ImGui::TableSetupColumn("##content", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableNextRow();

			// the tree on the left side
			ImGui::TableSetColumnIndex(0);
			ImGui::BeginChild("##treeContainer", ImVec2(0, -bottomBarHeight));
			for (auto node : m_treeCache)
				m_renderTree(node);
			ImGui::EndChild();

			// content on the right side
			ImGui::TableSetColumnIndex(1);
			ImGui::BeginChild("##contentContainer", ImVec2(0, -bottomBarHeight));
			m_renderContent();
			ImGui::EndChild();
			if (ImGui::IsItemHovered() && ImGui::GetIO().KeyCtrl && ImGui::GetIO().MouseWheel != 0.0f)
			{
				m_zoom += ImGui::GetIO().MouseWheel;
				m_zoom = std::min<float>(25.0f, std::max<float>(1.0f, m_zoom));
				m_refreshIconPreview();
			}
			m_renderPopups();

			ImGui::EndTable();
		}

		/***** BOTTOM BAR *****/
		ImGui::Text("File name:");
		ImGui::SameLine();
		if (ImGui::InputTextEx("##file_input", "", m_inputTextbox, 1024, ImVec2(m_type != IFD_DIALOG_DIRECTORY ? -250.0f : -FLT_MIN, 0), ImGuiInputTextFlags_EnterReturnsTrue))
			m_finalize(m_inputTextbox);
		if (m_type != IFD_DIALOG_DIRECTORY)
		{
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-FLT_MIN);

			int sel = m_filterSelection;
			if (ImGui::Combo("##ext_combo", &sel, m_filter.c_str()))
			{
				m_filterSelection = sel;
				m_setDirectory(m_currentDirectory, false); // refresh
			}
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().ItemSpacing.y);
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize("Cancel").x - ImGui::CalcTextSize("OK").x - ImGui::GetStyle().ItemSpacing.x * 4.0f - ImGui::GetStyle().FramePadding.x * 4.0f);
		if (ImGui::Button("OK"))
			m_finalize(m_inputTextbox);
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			m_isOpen = false;
	}
}