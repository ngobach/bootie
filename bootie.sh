#/usr/bin/env bash

set -e -o pipefail

cmd_run() {
  if [ ! hash qemu-system-x86_64 2>/dev/null ]; then
    echo "Missing qemu-system-x86_64"
    exit 1
  fi

  local target=
  local uefi=false
  local my_args=()

  while getopts ":t:u" opt; do
    case $opt in
      t)
        target="$OPTARG"
        ;;
      u)
        uefi=true
        ;;
      \?)
        echo "Invalid option: -$OPTARG" >&2
        print_usage
        exit 1
        ;;
      :)
        echo "Option -$OPTARG requires an argument." >&2
        print_usage
        exit 1
        ;;
    esac
  done
  shift $((OPTIND -1))

  if [ -n "$target" ]; then
    my_args+=("-drive" "file=/dev/$target,if=ide,format=raw")
  fi

  if $uefi; then
    my_args+=("-bios" "resources/bios.bin")
  fi

  my_args+=("$@")

  if [ -n "$target" ]; then
    check_root
    echo "Unmounting disks $target..."
    diskutil umountDisk force "$target"
  fi

  echo "Running qemu-system-x86_64 with args: ${my_args[*]}"

  qemu-system-x86_64 \
    -M q35 \
    -m 2G \
    -smp 4 \
    -monitor none \
    -boot order=dc \
    "${my_args[@]}"
}

cmd_run_arm64() {
  local bios="/opt/homebrew/Cellar/qemu/9.2.2/share/qemu/edk2-aarch64-code.fd"
  local my_args=()

  while getopts "" opt; do
    case $opt in
      \?)
        echo "Invalid option: -$OPTARG" >&2
        print_usage
        exit 1
        ;;
      :)
        echo "Option -$OPTARG requires an argument." >&2
        print_usage
        exit 1
        ;;
    esac
  done
  shift $((OPTIND -1))

  my_args+=("$@")

  qemu-system-aarch64 \
    -M virt,accel=hvf,highmem=off \
    -cpu cortex-a76 \
    -m 2G \
    -smp 2 \
    -nographic \
    -monitor none \
    -bios "$bios" \
    -nic user,model=virtio-net-pci \
    -boot order=dc,menu=off \
    "${my_args[@]}"
}

cmd_create() {
  local target=
  while getopts ":t:" opt; do
    case $opt in
      t)
        target="$OPTARG"
        ;;
      \?)
        echo "Invalid option: -$OPTARG" >&2
        print_usage
        exit 1
        ;;
      :)
        echo "Option -$OPTARG requires an argument." >&2
        print_usage
        exit 1
        ;;
    esac
  done
  shift $((OPTIND -1))

  if [ -z "$target" ]; then
    echo "Missing target"
    exit 1
  fi

  echo "Creating $target..."
  if [ "$(uname -s)" == "Darwin" ]; then
    hdiutil create -size 200m -layout GPTSPUD -align 32 -fs FAT32 -volname "EFI" "$target"
    mv "$target.dmg" "$target"
  else
    echo "Unsupported OS"
    exit 1
  fi
}

print_usage() {
  echo "Usage:"
  echo "  $0 run [--target <target> | --uefi] [...args]"
  echo "  $0 run-arm64 [...args]"
  echo "  $0 create -t <target>"
}

check_root() {
  if [ "$EUID" -ne 0 ]; then
    echo "Please run as root"
    exit 1
  fi
}

CMD=$1

if [ "$#" -gt 0 ]; then
  shift
fi

case "$CMD" in
  run)
    cmd_run $@
    ;;
  run-arm64)
    cmd_run_arm64 $@
    ;;
  create)
    cmd_create $@
    ;;
  help | "")
    print_usage
    ;;
  *)
    echo "Unknown command: $CMD"
    exit 1
    ;;
esac
