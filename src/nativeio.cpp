#include "nativeio.hpp"

#include "battery/embed.hpp"
#include "pugixml.hpp"

namespace nativeio {

std::string friendly_size_name(uint64_t size) {
  if (size < 1024) {
    return std::to_string(size) + "B";
  } else if (size < 1024 * 1024) {
    return std::to_string(size / 1024) + "KB";
  } else if (size < 1024 * 1024 * 1024) {
    return std::to_string(size / (1024 * 1024)) + "MB";
  } else {
    return std::to_string(size / (1024 * 1024 * 1024)) + "GB";
  }
}

UUID UUID::parse(const char* buffer) {
  UUID uuid;

  uuid.bytes[3] = buffer[0];
  uuid.bytes[2] = buffer[1];
  uuid.bytes[1] = buffer[2];
  uuid.bytes[0] = buffer[3];
  uuid.bytes[5] = buffer[4];
  uuid.bytes[4] = buffer[5];
  uuid.bytes[7] = buffer[6];
  uuid.bytes[6] = buffer[7];
  uuid.bytes[8] = buffer[8];
  uuid.bytes[9] = buffer[9];
  uuid.bytes[10] = buffer[10];
  uuid.bytes[11] = buffer[11];
  uuid.bytes[12] = buffer[12];
  uuid.bytes[13] = buffer[13];
  uuid.bytes[14] = buffer[14];
  uuid.bytes[15] = buffer[15];

  return uuid;
}

void UUID::print(FILE* fp) const {
  for (int i = 0; i < 16; i++) {
    if (i == 4 || i == 6 || i == 8 || i == 10) {
      fprintf(fp, "-");
    }
    fprintf(fp, "%02x", this->bytes[i]);
  }
}

bool UUID::is_zeroed() const {
  for (int i = 0; i < 16; i++) {
    if (this->bytes[i] != 0) {
      return false;
    }
  }
  return true;
}

std::vector<GptPart> GptPart::read_parts(FILE* fd) {
  std::vector<GptPart> result;
  char buffer[512];

  if (fseek(fd, 512, SEEK_SET) != 0) {
    throw std::runtime_error("Failed to seek");
  }

  if (fread(buffer, 512, 1, fd) != 1) {
    throw std::runtime_error("Failed to read");
  }

  if (memcmp(buffer, "EFI PART", 8) != 0) {
    throw std::runtime_error("Invalid GPT signature");
  }

  GptPart entry;

  for (int i = 0; i < 128; i++) {
    if (fread(buffer, 128, 1, fd) != 1) {
      throw std::runtime_error("Failed to read");
    }

    entry.partGUID = UUID::parse(buffer);
    entry.uniqueGUID = UUID::parse(buffer + 16);
    entry.startLBA = *(uint64_t*)(buffer + 32);
    entry.endLBA = *(uint64_t*)(buffer + 40);
    entry.attrs = *(uint64_t*)(buffer + 48);
    memcpy(entry.label, buffer + 56, 72);

    if (!entry.partGUID.is_zeroed()) {
      result.emplace_back(entry);
    }
  }

  return result;
}
int32_t copy_sector(FILE* dst, const char* src, size_t offset, bool is_mbr) {
  char buffer[512];
  memset(buffer, 0, 512);
  memcpy(buffer, src + offset * 512, 512);

  if (is_mbr) {
    if (fseek(dst, offset * 512 + 446, SEEK_SET) != 0) {
      fprintf(stderr, "Failed to seek\n");
      return -1;
    }

    if (fread(buffer + 446, 64, 1, dst) != 1) {
      fprintf(stderr, "Failed to read\n");
      return -1;
    }
  }

  if (fseek(dst, offset * 512, SEEK_SET) != 0) {
    fprintf(stderr, "Failed to seek\n");
    return -1;
  }

  if (fwrite(buffer, 512, 1, dst) != 1) {
    fprintf(stderr, "Failed to write\n");
    return -1;
  }

  return 0;
}

void install_to(std::string target) {
  FILE* target_fd = fopen(target.c_str(), "r+");

  if (target_fd == NULL) {
    throw std::runtime_error("Failed to open target disk");
  }

  fprintf(stderr, "Parsing GPT disk at %s\n", target.c_str());
  auto parts = nativeio::GptPart::read_parts(target_fd);

  if (parts.empty()) {
    throw std::runtime_error("No partitions found");
  }

  fprintf(stderr, "Found %d partitions\n", parts.size());

  for (auto& part : parts) {
    uint32_t part_offset = part.startLBA * 512;

    if (part_offset < 64 * 512) {
      char* msg = new char[1024];
      sprintf(msg,
              "Error: Parition \"%s\" starts before 64 sectors (LBA: %llu)\n",
              part.label, part.startLBA);
      throw std::runtime_error(msg);
    }
  }

  fprintf(stderr, "Installing to %s\n", target.c_str());

  auto src = b::embed<"resources/gptdisk-64-sectors.raw">();
  auto src_data = src.data();
  copy_sector(target_fd, src_data, 0, true);

  for (int32_t i = 34; i < 64; i++) {
    copy_sector(target_fd, src_data, i, false);
  }

  fclose(target_fd);
}

#if defined(_WIN32)
#elif defined(__linux__)
#elif defined(__APPLE__)

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
  fclose(fp);

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

    if (!is_gpt) {
      continue;
    }

    DiskInfo di = {};
    di.is_gpt = is_gpt;

    std::string label =
        item.select_node("key[. = 'DeviceIdentifier']/following-sibling::*")
            .node()
            .text()
            .as_string();

    di.identifier = "/dev/r" + label;
    di.label = label;

    di.size = item.select_node("key[. = 'Size']/following-sibling::*")
                  .node()
                  .text()
                  .as_llong();

    disks.emplace_back(di);
  }

  return disks;
}
#else
#throw "Unsupported platform"
#endif

}  // namespace nativeio
