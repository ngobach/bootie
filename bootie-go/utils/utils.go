package utils

import "os"

func ReadFileAsString(path string) string {
	file, err := os.Open(path)

	if err != nil {
		return ""
	}

	defer file.Close()

	bytes, err := os.ReadFile(path)

	if err != nil {
		return ""
	}

	return string(bytes)
}
