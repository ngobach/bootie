#ifdef __APPLE__
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <format>
#include <string>
#include <vector>

#include "nativeio.hpp"
#include "pugixml.hpp"

namespace nativeio {
const size_t BUFFER_SIZE = 1024 * 1024;
int8_t buffer[BUFFER_SIZE];
const char* CONTENT_TYPE_GPT_DISK = "GUID_partition_scheme";

std::vector<DiskInfo> get_disks() {
  std::vector<DiskInfo> disks;
  memset(buffer, 0, sizeof(buffer));

  FILE* fp = popen("diskutil list -plist", "r");

  if (fp == NULL) {
    throw std::runtime_error("Failed to run diskutil");
  }

  int32_t nread = fread(buffer, 1, BUFFER_SIZE, fp);

  if (nread < 0) {
    throw std::runtime_error("Failed to read diskutil output");
  }

  if (nread >= BUFFER_SIZE) {
    throw std::runtime_error("Diskutil output too large");
  }

  if (fclose(fp) != 0) {
    throw std::runtime_error("Failed to close file");
  }

  pugi::xml_document doc;

  if (!doc.load_buffer_inplace(buffer, nread)) {
    throw std::runtime_error("Failed to parse diskutil output");
  }

  auto plist = doc.child("plist");

  auto disk_and_partitions =
      doc.select_node("//key[. = 'AllDisksAndPartitions']/following-sibling::*")
          .node();

  for (auto item = disk_and_partitions.first_child(); item;
       item = item.next_sibling()) {
    auto contentNode =
        item.select_node("key[. = 'Content']/following-sibling::*").node();
    bool is_gpt =
        strcmp(contentNode.text().as_string(), CONTENT_TYPE_GPT_DISK) == 0;

    DiskInfo di = {};

    std::string label =
        item.select_node("key[. = 'DeviceIdentifier']/following-sibling::*")
            .node()
            .text()
            .as_string();

    di.identifier = "/dev/" + label;
    di.label = label;

    di.size = item.select_node("key[. = 'Size']/following-sibling::*")
                  .node()
                  .text()
                  .as_llong();

    disks.emplace_back(di);
  }

  return disks;
}
}  // namespace nativeio
#endif
