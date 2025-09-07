package main

import "io"

type RawIo interface {
	io.Closer
	ReadSector(int64) ([]byte, error)
	WriteSector(int64, []byte) error
}
