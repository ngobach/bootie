package exfat_test

import (
	"io"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/diskfs/go-diskfs/backend/file"
	"ngobach.com/bootie-go/exfat"
)

func TestCreateAndMountExfat(t *testing.T) {
	dir := t.TempDir()
	imgPath := filepath.Join(dir, "test.exfat")

	f, err := os.Create(imgPath)
	if err != nil {
		t.Fatal(err)
	}

	const imgSize = 64 * 1024 * 1024
	if err := f.Truncate(imgSize); err != nil {
		t.Fatal(err)
	}
	if err := f.Close(); err != nil {
		t.Fatal(err)
	}

	f, err = os.OpenFile(imgPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()

	if err := exfat.CreateExfat(f, 0, imgSize, "Bootie"); err != nil {
		t.Fatal("CreateExfat:", err)
	}

	ef, err := exfat.NewExfat(f)
	if err != nil {
		t.Fatal("NewExfat:", err)
	}
	defer ef.Close()

	if ef.Label() != "Bootie" {
		t.Errorf("label = %q, want %q", ef.Label(), "Bootie")
	}
}

func TestExfatFileOps(t *testing.T) {
	dir := t.TempDir()
	imgPath := filepath.Join(dir, "test.exfat")

	f, err := os.Create(imgPath)
	if err != nil {
		t.Fatal(err)
	}
	const imgSize = 64 * 1024 * 1024
	if err := f.Truncate(imgSize); err != nil {
		t.Fatal(err)
	}
	if err := f.Close(); err != nil {
		t.Fatal(err)
	}

	f, err = os.OpenFile(imgPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()

	if err := exfat.CreateExfat(f, 0, imgSize, "Test"); err != nil {
		t.Fatal("CreateExfat:", err)
	}

	ef, err := exfat.NewExfat(f)
	if err != nil {
		t.Fatal("NewExfat:", err)
	}
	defer ef.Close()

	t.Run("Mkdir", func(t *testing.T) {
		if err := ef.Mkdir("/mydir"); err != nil {
			t.Fatal(err)
		}
		entries, err := ef.ReadDir("/")
		if err != nil {
			t.Fatal(err)
		}
		found := false
		for _, e := range entries {
			if e.Name() == "mydir" && e.IsDir() {
				found = true
				break
			}
		}
		if !found {
			t.Fatal("mydir not found in root directory listing")
		}
	})

	t.Run("CreateFile", func(t *testing.T) {
		if err := ef.Mknod("/hello.txt", 0, 0); err != nil {
			t.Fatal(err)
		}
		file, err := ef.OpenFile("/hello.txt", os.O_RDWR)
		if err != nil {
			t.Fatal(err)
		}
		content := "Hello, exFAT!"
		n, err := file.Write([]byte(content))
		if err != nil {
			t.Fatal(err)
		}
		if n != len(content) {
			t.Fatalf("wrote %d bytes, want %d", n, len(content))
		}
		if err := file.Close(); err != nil {
			t.Fatal(err)
		}
	})

	t.Run("ReadFile", func(t *testing.T) {
		file, err := ef.OpenFile("/hello.txt", os.O_RDONLY)
		if err != nil {
			t.Fatal(err)
		}
		defer file.Close()

		data, err := io.ReadAll(file)
		if err != nil {
			t.Fatal(err)
		}
		if string(data) != "Hello, exFAT!" {
			t.Fatalf("read %q, want %q", string(data), "Hello, exFAT!")
		}
	})

	t.Run("ReadDir", func(t *testing.T) {
		entries, err := ef.ReadDir("/")
		if err != nil {
			t.Fatal(err)
		}
		names := make([]string, len(entries))
		for i, e := range entries {
			names[i] = e.Name()
		}
		hasFile := false
		hasDir := false
		for _, e := range entries {
			if e.Name() == "hello.txt" && !e.IsDir() {
				hasFile = true
			}
			if e.Name() == "mydir" && e.IsDir() {
				hasDir = true
			}
		}
		if !hasFile {
			t.Fatal("hello.txt not found")
		}
		if !hasDir {
			t.Fatal("mydir not found")
		}
	})

	t.Run("Rename", func(t *testing.T) {
		if err := ef.Rename("/hello.txt", "/renamed.txt"); err != nil {
			t.Fatal(err)
		}
		if _, err := ef.OpenFile("/hello.txt", os.O_RDONLY); err == nil {
			t.Fatal("expected error opening renamed file at old path")
		}
		f, err := ef.OpenFile("/renamed.txt", os.O_RDONLY)
		if err != nil {
			t.Fatal(err)
		}
		f.Close()
	})

	t.Run("Remove", func(t *testing.T) {
		if err := ef.Remove("/renamed.txt"); err != nil {
			t.Fatal(err)
		}
		if _, err := ef.OpenFile("/renamed.txt", os.O_RDONLY); err == nil {
			t.Fatal("expected error after removing file")
		}
	})

	t.Run("Rmdir", func(t *testing.T) {
		if err := ef.Remove("/mydir"); err != nil {
			t.Fatal(err)
		}
		entries, err := ef.ReadDir("/")
		if err != nil {
			t.Fatal(err)
		}
		for _, e := range entries {
			if e.Name() == "mydir" {
				t.Fatal("mydir should have been removed")
			}
		}
	})

	t.Run("SetLabel", func(t *testing.T) {
		if err := ef.SetLabel("NewLabel"); err != nil {
			t.Fatal(err)
		}
		if ef.Label() != "NewLabel" {
			t.Fatalf("label = %q, want %q", ef.Label(), "NewLabel")
		}
	})

	t.Run("LargeFile", func(t *testing.T) {
		if err := ef.Mknod("/large.bin", 0, 0); err != nil {
			t.Fatal(err)
		}
		f, err := ef.OpenFile("/large.bin", os.O_RDWR)
		if err != nil {
			t.Fatal(err)
		}
		const size = 1024 * 1024
		data := make([]byte, size)
		for i := range data {
			data[i] = byte(i)
		}
		n, err := f.Write(data)
		if err != nil {
			t.Fatal(err)
		}
		if n != size {
			t.Fatalf("wrote %d bytes, want %d", n, size)
		}
		if err := f.Close(); err != nil {
			t.Fatal(err)
		}

		f, err = ef.OpenFile("/large.bin", os.O_RDONLY)
		if err != nil {
			t.Fatal(err)
		}
		readback, err := io.ReadAll(f)
		if err != nil {
			t.Fatal(err)
		}
		f.Close()
		if len(readback) != size {
			t.Fatalf("read %d bytes, want %d", len(readback), size)
		}
		for i := range readback {
			if readback[i] != byte(i) {
				t.Fatalf("byte %d: got %d, want %d", i, readback[i], byte(i))
			}
		}
	})

	t.Run("NestedDirs", func(t *testing.T) {
		for _, p := range []string{"/a", "/a/b", "/a/b/c"} {
			if err := ef.Mkdir(p); err != nil {
				t.Fatal(err)
			}
		}
		entries, err := ef.ReadDir("/a/b/c")
		if err != nil {
			t.Fatal(err)
		}
		if len(entries) != 0 {
			t.Fatalf("expected empty dir, got %d entries", len(entries))
		}
	})

	t.Run("CaseInsensitive", func(t *testing.T) {
		if err := ef.Mknod("/CaseTest.txt", 0, 0); err != nil {
			t.Fatal(err)
		}
		f, err := ef.OpenFile("/casetest.txt", os.O_RDONLY)
		if err != nil {
			t.Fatal("case-insensitive lookup failed:", err)
		}
		f.Close()
	})

	t.Run("UnsupportedOps", func(t *testing.T) {
		if err := ef.Link("/a", "/b"); err == nil {
			t.Fatal("expected ErrNotSupported for Link")
		}
		if err := ef.Symlink("/a", "/b"); err == nil {
			t.Fatal("expected ErrNotSupported for Symlink")
		}
	})
}

func TestExfatNewExfatErrors(t *testing.T) {
	dir := t.TempDir()
	imgPath := filepath.Join(dir, "empty.exfat")
	f, err := os.Create(imgPath)
	if err != nil {
		t.Fatal(err)
	}
	if _, err := exfat.NewExfat(f); err == nil {
		t.Fatal("expected error on empty file")
	}
	f.Close()

	f, err = os.OpenFile(imgPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()

	if err := exfat.CreateExfat(f, 0, 64*1024*1024, "Test"); err != nil {
		t.Fatal(err)
	}

	ef, err := exfat.NewExfat(f)
	if err != nil {
		t.Fatal(err)
	}
	defer ef.Close()

	if _, err := ef.OpenFile("/nonexistent", os.O_RDONLY); err == nil {
		t.Fatal("expected error for nonexistent file")
	}
	if _, err := ef.ReadDir("/nonexistent"); err == nil {
		t.Fatal("expected error for nonexistent directory")
	}
}

func TestExfatClose(t *testing.T) {
	dir := t.TempDir()
	imgPath := filepath.Join(dir, "close.exfat")
	f, err := os.Create(imgPath)
	if err != nil {
		t.Fatal(err)
	}
	if err := f.Truncate(64 * 1024 * 1024); err != nil {
		t.Fatal(err)
	}
	f.Close()

	f, err = os.OpenFile(imgPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()

	if err := exfat.CreateExfat(f, 0, 64*1024*1024, "Test"); err != nil {
		t.Fatal(err)
	}

	ef, err := exfat.NewExfat(f)
	if err != nil {
		t.Fatal(err)
	}

	if err := ef.Mkdir("/testdir"); err != nil {
		t.Fatal(err)
	}

	if err := ef.Close(); err != nil {
		t.Fatal("Close:", err)
	}

	if err := ef.Close(); err != nil {
		t.Fatal("double close:", err)
	}
}

func TestExfatWriteAfterSeek(t *testing.T) {
	dir := t.TempDir()
	imgPath := filepath.Join(dir, "seek.exfat")
	f, err := os.Create(imgPath)
	if err != nil {
		t.Fatal(err)
	}
	const imgSize = 64 * 1024 * 1024
	if err := f.Truncate(imgSize); err != nil {
		t.Fatal(err)
	}
	f.Close()

	f, err = os.OpenFile(imgPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()

	if err := exfat.CreateExfat(f, 0, imgSize, "Test"); err != nil {
		t.Fatal(err)
	}

	ef, err := exfat.NewExfat(f)
	if err != nil {
		t.Fatal(err)
	}
	defer ef.Close()

	if err := ef.Mknod("/seektest.txt", 0, 0); err != nil {
		t.Fatal(err)
	}

	file, err := ef.OpenFile("/seektest.txt", os.O_RDWR)
	if err != nil {
		t.Fatal(err)
	}
	defer file.Close()

	initial := []byte("Hello World")
	if _, err := file.Write(initial); err != nil {
		t.Fatal(err)
	}

	if _, err := file.Seek(6, io.SeekStart); err != nil {
		t.Fatal(err)
	}
	if _, err := file.Write([]byte("Go")); err != nil {
		t.Fatal(err)
	}

	if _, err := file.Seek(0, io.SeekStart); err != nil {
		t.Fatal(err)
	}
	data, err := io.ReadAll(file)
	if err != nil {
		t.Fatal(err)
	}
	if string(data) != "Hello Go" && !strings.HasPrefix(string(data), "Hello Go") {
		t.Fatalf("got %q, expected to start with %q", string(data), "Hello Go")
	}
}

func TestNewExfatFromBackend(t *testing.T) {
	dir := t.TempDir()
	imgPath := filepath.Join(dir, "test.exfat")

	f, err := os.Create(imgPath)
	if err != nil {
		t.Fatal(err)
	}
	const imgSize = 64 * 1024 * 1024
	if err := f.Truncate(imgSize); err != nil {
		t.Fatal(err)
	}
	if err := f.Close(); err != nil {
		t.Fatal(err)
	}

	f, err = os.OpenFile(imgPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()

	if err := exfat.CreateExfat(f, 0, imgSize, "Bootie"); err != nil {
		t.Fatal("CreateExfat:", err)
	}
	f.Close()

	f, err = os.OpenFile(imgPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()

	b := file.New(f, false)
	ef, err := exfat.NewExfatFromBackend(b, 0)
	if err != nil {
		t.Fatal("NewExfatFromBackend:", err)
	}
	defer ef.Close()

	if ef.Label() != "Bootie" {
		t.Errorf("label = %q, want %q", ef.Label(), "Bootie")
	}

	if err := ef.Mkdir("/mydir"); err != nil {
		t.Fatal("Mkdir:", err)
	}

	if err := ef.Mknod("/hello.txt", 0, 0); err != nil {
		t.Fatal("Mknod:", err)
	}

	fh, err := ef.OpenFile("/hello.txt", os.O_RDWR)
	if err != nil {
		t.Fatal("OpenFile:", err)
	}
	msg := "Hello from backend!"
	if _, err := fh.Write([]byte(msg)); err != nil {
		t.Fatal("Write:", err)
	}
	if _, err := fh.Seek(0, io.SeekStart); err != nil {
		t.Fatal("Seek:", err)
	}
	buf := make([]byte, len(msg))
	if _, err := io.ReadFull(fh, buf); err != nil {
		t.Fatal("ReadFull:", err)
	}
	if string(buf) != msg {
		t.Fatalf("got %q, want %q", string(buf), msg)
	}
	if err := fh.Close(); err != nil {
		t.Fatal("fh.Close:", err)
	}

	entries, err := ef.ReadDir("/")
	if err != nil {
		t.Fatal("ReadDir:", err)
	}
	foundDir, foundFile := false, false
	for _, e := range entries {
		switch e.Name() {
		case "mydir":
			foundDir = true
		case "hello.txt":
			foundFile = true
		}
	}
	if !foundDir {
		t.Error("mydir not found")
	}
	if !foundFile {
		t.Error("hello.txt not found")
	}

	if err := ef.Remove("/hello.txt"); err != nil {
		t.Fatal("Remove:", err)
	}
	if err := ef.Remove("/mydir"); err != nil {
		t.Fatal("Remove /mydir:", err)
	}

	if err := ef.Close(); err != nil {
		t.Fatal("Close:", err)
	}

	if err := b.Close(); err != nil {
		t.Fatal("backend.Close:", err)
	}
}
