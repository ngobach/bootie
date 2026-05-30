package core

import (
	"fmt"
	"strconv"
	"strings"
)

func ParseSize(sizeStr string) (int64, error) {
	sizeStr = strings.TrimSpace(sizeStr)

	var numStr, unit string
	for i, char := range sizeStr {
		if char >= '0' && char <= '9' {
			continue
		}
		numStr = sizeStr[:i]
		unit = strings.ToUpper(sizeStr[i:])
		break
	}

	if numStr == "" {
		return 0, fmt.Errorf("invalid size format: %s", sizeStr)
	}

	num, err := strconv.ParseInt(numStr, 10, 64)
	if err != nil {
		return 0, fmt.Errorf("failed to parse number: %w", err)
	}

	switch unit {
	case "K", "KB":
		return num * 1024, nil
	case "M", "MB":
		return num * 1024 * 1024, nil
	case "G", "GB":
		return num * 1024 * 1024 * 1024, nil
	case "":
		return num, nil
	default:
		return 0, fmt.Errorf("unsupported unit: %s", unit)
	}
}
