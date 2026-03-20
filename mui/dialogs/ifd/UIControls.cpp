// src/ImFileDialog/UIControls.cpp
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include "UIControls.h"
#include "IconsFontAwesome6.h"
#include <imgui_internal.h>
#include <algorithm>
#include <vector>

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
}
