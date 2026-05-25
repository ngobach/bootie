package main

import (
	"io"
	"io/fs"
	"os"
	"path/filepath"
	"strings"

	log "github.com/charmbracelet/log"
	"github.com/diskfs/go-diskfs/filesystem"
)

func copyToLocalFilesystem(root fs.FS, source, dest string) error {
	return fs.WalkDir(root, source, func(path string, d fs.DirEntry, err error) error {
		if err != nil {
			return err
		}

		if path == source {
			return nil
		}

		destFilePath := filepath.Join(dest, strings.TrimPrefix(path, source))
		log.Debugf("Creating %s", destFilePath)

		if d.IsDir() {
			return os.MkdirAll(destFilePath, os.ModePerm)
		}

		sourceFile, err := root.Open(path)
		if err != nil {
			return err
		}
		defer sourceFile.Close()

		destFile, err := os.OpenFile(destFilePath, os.O_WRONLY|os.O_CREATE, os.ModePerm)
		if err != nil {
			return err
		}
		defer destFile.Close()

		_, err = io.Copy(destFile, sourceFile)

		return err
	})
}

func copyToFilesystem(root fs.FS, source string, dest filesystem.FileSystem) error {
	return fs.WalkDir(root, source, func(path string, d fs.DirEntry, err error) error {
		if err != nil {
			return err
		}

		if path == source {
			return nil
		}

		relPath := strings.TrimPrefix(path, source)
		log.Debugf("Creating %s", relPath)

		if d.IsDir() {
			return dest.Mkdir(relPath)
		}

		sourceFile, err := root.Open(path)
		if err != nil {
			return err
		}
		defer sourceFile.Close()

		destFile, err := dest.OpenFile(relPath, os.O_RDWR|os.O_CREATE)
		if err != nil {
			return err
		}
		defer destFile.Close()

		_, err = io.Copy(destFile, sourceFile)

		return err
	})
}
