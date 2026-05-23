package exfat

import (
	"io"

	"github.com/diskfs/go-diskfs/backend"
)

type backendAdapter struct {
	b      backend.Storage
	start  int64
	offset int64
}

func (a *backendAdapter) Read(p []byte) (int, error) {
	n, err := a.b.ReadAt(p, a.start+a.offset)
	a.offset += int64(n)
	return n, err
}

func (a *backendAdapter) Write(p []byte) (int, error) {
	w, err := a.b.Writable()
	if err != nil {
		return 0, err
	}
	n, err := w.WriteAt(p, a.start+a.offset)
	a.offset += int64(n)
	return n, err
}

func (a *backendAdapter) Seek(offset int64, whence int) (int64, error) {
	switch whence {
	case io.SeekStart:
		a.offset = offset
	case io.SeekCurrent:
		a.offset += offset
	case io.SeekEnd:
		fi, err := a.b.Stat()
		if err != nil {
			return 0, err
		}
		a.offset = fi.Size() - a.start + offset
	}
	return a.offset, nil
}

func (a *backendAdapter) Close() error {
	return a.b.Close()
}

func NewExfatFromBackend(b backend.Storage, start int64) (*Exfat, error) {
	return NewExfat(&backendAdapter{b: b, start: start})
}
