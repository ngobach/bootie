#/usr/bin/env bash

set -e -o pipefail

cmd_run() {
  if [ ! hash qemu-system-x86_64 2>/dev/null ]; then
    echo "Missing qemu-system-x86_64"
    exit 1
  fi

  local with_tools=y
  local target=
  local my_args="-m 4G -smp 4"

  while [ -n "$1" ]; do
    case "$1" in
      --no-tools)
        with_tools=
        shift
        ;;
      --target)
        if [ "$#" -lt 2 ]; then
          echo "Missing argument for --target"
          exit 1
        fi
        target=$2
        shift 2
        ;;
      --uefi)
        my_args="$my_args -drive file=tools/bios.bin,if=pflash,readonly=true"
        shift
        ;;
      --cdrom)
        if [ "$#" -lt 2 ]; then
          echo "Missing argument for --cdrom"
          exit 1
        fi
        my_args="$my_args -cdrom $2 -boot order=d"
        shift 2
        ;;
      *)
        break
        ;;
    esac
  done

  if [ -z "$target" ]; then
    echo "Missing target"
    exit 1
  fi

  my_args="$my_args -hda /dev/$target"

  if [ -n "$with_tools" ]; then
    my_args="$my_args -hdb fat:rw:./tools"
  fi

  my_args="$my_args $@"

  echo "Unmounting disks..."
  diskutil umountDisk force "$target"

  echo "QEMU args: $my_args"
  echo "Running qemu-system-x86_64..."
  qemu-system-x86_64 $my_args
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

print_usage() {
  echo "Usage:"
  echo "  $0 run [--target <target> | --uefi | --cdrom <iso> | --no-tools] [...args]"
  echo "  $0 install <target>"
}

# Check root
if [ "$EUID" -ne 0 ]; then
  echo "Please run as root"
  exit 1
fi

CMD=$1

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
  help | "")
    print_usage
    ;;
  *)
    echo "Unknown command: $CMD"
    exit 1
    ;;
esac
