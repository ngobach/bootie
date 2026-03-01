package resources

import (
	"embed"
)

//go:embed gptdisk-64-sectors.raw
var SeedSectors []byte

//go:embed efi-part/*
var EfiFiles embed.FS

//go:embed data-part/*
var DataFiles embed.FS

//go:embed bios.bin
var BiosBin []byte

//go:embed edk2-aarch64-code.fd
var Edk2Aarch64Code []byte
