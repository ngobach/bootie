#include <unistd.h>

#include <cstdio>
#include <string>

using namespace std;

struct UUID {
  uint8_t bytes[16];
};

struct GptPart {
  UUID partGUID;
  UUID uniqueGUID;
  uint64_t startLBA;
  uint64_t endLBA;
  uint64_t attrs;
  char label[128];
};

void uuid_parse(UUID* target, char* buffer) {
  target->bytes[3] = buffer[0];
  target->bytes[2] = buffer[1];
  target->bytes[1] = buffer[2];
  target->bytes[0] = buffer[3];
  target->bytes[5] = buffer[4];
  target->bytes[4] = buffer[5];
  target->bytes[7] = buffer[6];
  target->bytes[6] = buffer[7];
  target->bytes[8] = buffer[8];
  target->bytes[9] = buffer[9];
  target->bytes[10] = buffer[10];
  target->bytes[11] = buffer[11];
  target->bytes[12] = buffer[12];
  target->bytes[13] = buffer[13];
  target->bytes[14] = buffer[14];
  target->bytes[15] = buffer[15];
}

void uuid_print(UUID* uuid) {
  for (int i = 0; i < 16; i++) {
    if (i == 4 || i == 6 || i == 8 || i == 10) {
      fprintf(stderr, "-");
    }
    fprintf(stderr, "%02x", uuid->bytes[i]);
  }
}

bool uuid_is_zero(UUID* uuid) {
  for (int i = 0; i < 16; i++) {
    if (uuid->bytes[i] != 0) {
      return false;
    }
  }
  return true;
}

// return number of parts, -1 if failed
uint32_t gpt_read_parts(FILE* fd, GptPart result[], uint32_t max_parts) {
  char buffer[512];

  if (fseek(fd, 512, SEEK_SET) != 0) {
    fprintf(stderr, "Failed to seek\n");
    return -1;
  }

  if (fread(buffer, 512, 1, fd) != 1) {
    fprintf(stderr, "Failed to read\n");
    return -1;
  }

  // Validate GPT signature (first 8 bytes)
  if (memcmp(buffer, "EFI PART", 8) != 0) {
    fprintf(stderr, "Invalid GPT signature\n");
    return -1;
  }

  int32_t num_entries = 0;
  GptPart entry;

  for (int i = 0; i < max_parts; i++) {
    if (fread(buffer, 128, 1, fd) != 1) {
      fprintf(stderr, "Failed to read\n");
      return -1;
    }

    uuid_parse(&entry.partGUID, buffer);
    uuid_parse(&entry.uniqueGUID, buffer + 16);
    entry.startLBA = *(uint64_t*)(buffer + 32);
    entry.endLBA = *(uint64_t*)(buffer + 40);
    entry.attrs = *(uint64_t*)(buffer + 48);
    memcpy(entry.label, buffer + 56, 72);

    if (uuid_is_zero(&entry.partGUID)) {
      break;
    }

    result[num_entries++] = entry;
  }

  return num_entries;
}

int32_t copy_sector(FILE* dst, FILE* src, size_t offset, bool is_mbr) {
  char buffer[512];
  memset(buffer, 0, 512);

  if (fseek(src, offset * 512, SEEK_SET) != 0) {
    fprintf(stderr, "Failed to seek\n");
    return -1;
  }

  if (fread(buffer, 512, 1, src) != 1) {
    fprintf(stderr, "Failed to read\n");
    return -1;
  }

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

int cmd_install(string target) {
  GptPart parts[128];
  FILE* fd = fopen(target.c_str(), "r+");

  if (fd == NULL) {
    fprintf(stderr, "Failed to open target\n");
    return 1;
  }

  fprintf(stderr, "Parsing GPT disk at %s\n", target.c_str());
  int32_t num_parts = gpt_read_parts(fd, parts, 128);

  if (num_parts < 0) {
    return 1;
  }

  if (num_parts == 0) {
    fprintf(stderr, "No partitions found\n");
    return 1;
  }

  fprintf(stderr, "Found %d partitions\n", num_parts);

  for (int32_t i = 0; i < num_parts; i++) {
    GptPart& part = parts[i];
    uint32_t part_offset = part.startLBA * 512;

    if (part_offset < 64 * 512) {
      fprintf(stderr,
              "Error: Parition %d starts before 64 sectors (LBA: %llu)\n", i,
              part.startLBA);
      return 1;
    }
  }

  FILE* src = fopen("../resources/gptdisk-64-sectors.raw", "r");

  if (src == NULL) {
    fprintf(stderr, "Failed to open source file\n");
    return 1;
  }

  fprintf(stderr, "Installing to %s\n", target.c_str());

  if (copy_sector(fd, src, 0, true) < 0) {
    return 1;
  }

  for (int32_t i = 34; i < 64; i++) {
    if (copy_sector(fd, src, i, false) < 0) {
      return 1;
    }
  }

  fclose(fd);
  fclose(src);
  fprintf(stderr, "Done\n");

  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing command\n");
    return 1;
  }

  char* command = argv[1];

  if (strcmp(command, "install") == 0) {
    if (argc < 3) {
      fprintf(stderr, "Missing target\n");
      return 1;
    }

    return cmd_install(string(argv[2]));
  }

  return 1;
}
