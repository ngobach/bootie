package resources

import (
	"embed"
)

//go:embed gptdisk-64-sectors.raw
var SeedSectors []byte

//go:embed efi-part/**/*
var EfiFiles embed.FS

//go:embed data-part/*
var DataFiles embed.FS
