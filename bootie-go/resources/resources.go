package resources

import (
	"embed"
)

//go:embed mbr.raw
var MBR []byte

//go:embed grldr.mbr.raw
var GrldrMBR []byte

//go:embed efi-part/*
var EfiFiles embed.FS

//go:embed data-part/*
var DataFiles embed.FS
