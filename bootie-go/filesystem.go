package main

import (
	"io"

	"github.com/diskfs/go-diskfs/filesystem"
)

type FileSystem interface {
	filesystem.FileSystem
	Create(w io.WriterAt, start int64, size int64, label string) error
}
