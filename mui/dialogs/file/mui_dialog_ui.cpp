// src/ImFileDialog/UIControls.cpp
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include "mui_dialog_ui.h"
#include "IconsFontAwesome6.h"
#include "../../widgets/iconbutton.hpp"
#include <imgui_internal.h>
#include <unordered_map>
#include <algorithm>
#include <vector>

#ifndef ICON_FA_FOLDER
#define ICON_FA_FOLDER "\xef\x81\xbb"
#endif
#ifndef ICON_FA_FILE
#define ICON_FA_FILE "\xef\x85\x9b"
#endif

namespace
{
	inline float GetGuiElementSize() { return ImGui::GetFrameHeight(); }
}

namespace mui_dlg
{
	bool FolderNode(const char *label, ImTextureID icon, bool &clicked)
	{
		clicked = false;
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

		std::string displayStr = label;
		if (icon)
		{
			displayStr = std::string("    ") + label;
		}
		else
		{
			displayStr = std::string(ICON_FA_FOLDER) + " " + label;
		}

		bool nodeOpen = ImGui::TreeNodeEx(label, flags, "%s", displayStr.c_str());

		if (ImGui::IsItemClicked(0))
		{
			clicked = true;
		}

		if (icon)
		{
			ImVec2 min = ImGui::GetItemRectMin();
			float fontSize = ImGui::GetFontSize();
			float iconX = min.x + ImGui::GetTreeNodeToLabelSpacing() - fontSize - ImGui::GetStyle().ItemInnerSpacing.x;
			float iconY = min.y + (ImGui::GetItemRectSize().y - fontSize) * 0.5f;
			ImGui::GetWindowDrawList()->AddImage(icon, ImVec2(iconX, iconY), ImVec2(iconX + fontSize, iconY + fontSize));
		}

		return nodeOpen;
	}

	bool FileIcon(const char *label, bool isSelected, ImTextureID icon, bool isDirectory, ImVec2 size, bool hasPreview, int previewWidth, int previewHeight)
	{
		bool ret = false;
		float iconSize = size.y - ImGui::GetFontSize() * 2;
		ImGui::PushID(label);
		auto btn = mui::IconButton::create(label)
					   ->setLayout(mui::IconButtonLayout::Vertical)
					   ->setSize(size.x, size.y)
					   ->setSelected(isSelected)
					   ->onClick([&]()
								 { ret = true; })
					   ->onDoubleClick([&]()
									   { ret = true; });

		if (hasPreview && icon)
		{
			float scale = std::min<float>(size.x / previewWidth, iconSize / previewHeight);
			btn->setIconSize(previewWidth * scale, previewHeight * scale);
			btn->setIconTexture(icon);
		}
		else if (icon)
		{
			btn->setIconSize(iconSize, iconSize);
			btn->setIconTexture(icon);
		}
		else
		{
			btn->setIconSize(iconSize, iconSize);
			btn->setIconText(isDirectory ? ICON_FA_FOLDER : ICON_FA_FILE);
		}

		btn->render();

		float windowSpace = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
		float lastButtomPos = ImGui::GetItemRectMax().x;
		float thisButtonPos = lastButtomPos + ImGui::GetStyle().ItemSpacing.x + size.x;
		if (thisButtonPos < windowSpace)
			ImGui::SameLine();
		ImGui::PopID();
		return ret;
	}
}