#/usr/bin/env bash

set -e -o pipefail

cmd_mount() {
  local disk_identifier=$1
  if [ -z "$disk_identifier" ]; then
    echo "Missing disk identifier"
    exit 1
  fi
  echo "Mounting $disk_identifier as NBD..."
  diskutil umountdisk force $disk_identifier && sudo qemu-nbd -t -f raw /dev/$disk_identifier
}

cmd_run() {
  if [ ! hash qemu-system-x86_64 2>/dev/null ]; then
    echo "Missing qemu-system-x86_64"
    exit 1
  fi

  local my_args="-m 4G -smp 4 -hda nbd://localhost -hdb fat:rw:./tools"

  while [ -n "$1" ]; do
    case "$1" in
      --uefi)
        my_args="$my_args -drive file=tools/bios.bin,if=pflash,readonly=true"
        shift
        ;;
      --cdrom)
        my_args="$my_args -cdrom $2 -boot menu=on"
        shift 2
        ;;
      *)
        break
        ;;
    esac
  done

  if [ -n "$my_args" ]; then
    echo "QEMU args: $my_args"
  fi

  echo "Running qemu-system-x86_64..."
  qemu-system-x86_64 $my_args $@
}

cmd_install() {
  local target=$1

  if [ -z "$target" ]; then
    echo "Missing target"
    exit 1
  fi

  echo "Installing to $target..."
  echo "Running ./tools/bootlace.com --gpt $target"
  ./tools/bootlace.com --gpt "$target"
}

# Check root
if [ "$EUID" -ne 0 ]; then
  echo "Please run as root"
  exit 1
fi

CMD=$1

if [ -z "$CMD" ]; then
  CMD=run
fi

if [ "$#" -gt 0 ]; then
  shift
fi

case "$CMD" in
  run)
    cmd_run $@
    ;;
  mount)
    cmd_mount $@
    ;;
  install)
    cmd_install $@
    ;;
  *)
    echo "Unknown command: $CMD"
    exit 1
    ;;
esac
