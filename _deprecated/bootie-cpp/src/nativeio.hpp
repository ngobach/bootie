#include <string>
#include <vector>
#include <cstdint>

namespace nativeio {
struct UUID {
  uint8_t bytes[16];
  static UUID parse(const char* buffer);
  void print(FILE*) const;
  bool is_zeroed() const;
};

struct GptPart {
  UUID partGUID;
  UUID uniqueGUID;
  uint64_t startLBA;
  uint64_t endLBA;
  uint64_t attrs;
  char label[128];

  static std::vector<GptPart> read_parts(FILE* fd);
};

struct DiskInfo {
  int64_t size;
  std::string identifier;
  std::string label;
  bool is_removable;
};

std::vector<DiskInfo> get_disks();
std::string friendly_size_name(uint64_t size);
void install_to(std::string target);

};  // namespace nativeio
