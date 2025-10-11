package main

import (
	"fmt"
	"io"
	"io/fs"
	"os"
	"path/filepath"
	"strings"
)

func extractFiles(root fs.FS, source, dest string) error {
	return fs.WalkDir(root, source, func(path string, d fs.DirEntry, err error) error {
		if err != nil {
			return err
		}

		if path == source {
			return nil
		}

		destFilePath := filepath.Join(dest, strings.TrimPrefix(path, source))
		fmt.Println("Creating", destFilePath)

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
