// src/ImFileDialog/ImFileDialog_Render.cpp
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "ImFileDialog.h"
#include "UIControls.h"
#include "IconsFontAwesome6.h"
#include "stb_image.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <fstream>
#include <algorithm>

#ifndef ICON_FA_FOLDER
#define ICON_FA_FOLDER "\xef\x81\xbb"
#endif
#ifndef ICON_FA_FILE
#define ICON_FA_FILE "\xef\x85\x9b"
#endif

#define ICON_SIZE ImGui::GetFontSize() + 3
#define GUI_ELEMENT_SIZE std::max(ImGui::GetFontSize() + 10.f, 24.f)

namespace ifd
{
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
