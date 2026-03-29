// src/ImFileDialog/Models.cpp
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "mui_models.h"
#include <sys/stat.h>

namespace mui_dlg
{
  FileData::FileData(const std::filesystem::path &path)
  {
    Path = path;

    struct stat attr;
    if (stat(path.string().c_str(), &attr) == 0)
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
    IconPreviewHeight = 0;
    IconPreviewWidth = 0;
  }
} // namespace mui_dlg
