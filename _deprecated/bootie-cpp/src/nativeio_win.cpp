#if defined(_WIN32)

#include <windows.h>

#include <format>
#include <vector>

#include "nativeio.hpp"

namespace nativeio {

static int64_t disk_get_size(HANDLE hDevice) {
  DISK_GEOMETRY diskGeometry = {0};
  DWORD bytesReturned = 0;

  BOOL success = DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL,
                                 0, &diskGeometry, sizeof(diskGeometry),
                                 &bytesReturned, NULL);

  if (success) {
    return diskGeometry.Cylinders.QuadPart * diskGeometry.TracksPerCylinder *
           diskGeometry.SectorsPerTrack * diskGeometry.BytesPerSector;
  } else {
    return 0;
  }
}

// disk_get_name

static std::string disk_get_name(HANDLE hDevice) {
  STORAGE_PROPERTY_QUERY query;
  memset(&query, 0, sizeof(query));
  query.PropertyId = StorageDeviceProperty;
  query.QueryType = PropertyStandardQuery;

  // First call to get required buffer size
  STORAGE_DEVICE_DESCRIPTOR deviceDescriptorHeader = {0};
  DWORD bytesReturned = 0;

  BOOL success =
      DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &query,
                      sizeof(query), &deviceDescriptorHeader,
                      sizeof(deviceDescriptorHeader), &bytesReturned, NULL);

  if (!success && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
    fprintf(stderr, "[!] Failed to query device descriptor size. Error: %lu\n",
            GetLastError());
    return "Unavailable";
  }

  // Allocate a buffer of the required size
  DWORD bufferSize = deviceDescriptorHeader.Size;
  BYTE* buffer = (BYTE*)malloc(bufferSize);

  if (!buffer) {
    fprintf(stderr, "[!] Memory allocation failed\n");
    return "Unavailable";
  }

  // Second call to get full descriptor
  success =
      DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &query,
                      sizeof(query), buffer, bufferSize, &bytesReturned, NULL);

  if (!success) {
    printf("[!] Failed to query device properties. Error: %lu\n",
           GetLastError());
    free(buffer);
    return "Unavailable";
  }

  // Cast the buffer to STORAGE_DEVICE_DESCRIPTOR
  STORAGE_DEVICE_DESCRIPTOR* desc = (STORAGE_DEVICE_DESCRIPTOR*)buffer;
  std::string label =
      std::format("{}", (char*)(buffer + desc->ProductIdOffset));
  free(buffer);

  return label;
}

std::vector<DiskInfo> get_disks() {
  std::vector<DiskInfo> disks;

  for (int i = 0; i < 32; i++) {
    std::string path = "\\\\.\\PhysicalDrive" + std::to_string(i);
    HANDLE hDevice = CreateFile(path.c_str(), GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                OPEN_EXISTING, 0, NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
      continue;
    }
    fprintf(stderr, "Found disk: %s\n", path.c_str());

    DiskInfo entry;
    entry.identifier = path;
    entry.label = disk_get_name(hDevice);
    entry.is_removable = true;
    entry.size = disk_get_size(hDevice);
    disks.push_back(entry);

    CloseHandle(hDevice);
  }

  return disks;
}
}  // namespace nativeio

#endif
