package exfat

import (
	"testing"
	"time"
)

func TestTimezoneTimestampEncodingDecoding(t *testing.T) {
	// Let's test with a positive offset timezone (e.g. UTC+7)
	locPlus7 := time.FixedZone("UTC+7", 7*3600)
	tPlus7 := time.Date(2026, 6, 2, 23, 11, 19, 0, locPlus7)

	_, _, _, tzoffset := exfatEncodeTimestamp(tPlus7)
	t.Logf("UTC+7 timezone offset encoded: 0x%X", tzoffset)

	// According to exfat spec:
	// tzoffset is a 1-byte field.
	// Bit 7: OffsetValid (should be 1).
	// Bits 0-6: OffsetFromUtc in 15-minute increments (7-bit signed).
	// For UTC+7, offset is +7 hours = +28 increments.
	// So 7-bit signed value should be 28 (0x1C).
	// Fully encoded value: 0x1C | 0x80 = 0x9C.
	
	expectedTzOffsetPlus7 := uint8(28) | 0x80
	if tzoffset != expectedTzOffsetPlus7 {
		t.Errorf("For UTC+7, expected tzoffset 0x%02X, got 0x%02X", expectedTzOffsetPlus7, tzoffset)
	}

	// Let's test with a negative offset timezone (e.g. UTC-8)
	locMinus8 := time.FixedZone("UTC-8", -8*3600)
	tMinus8 := time.Date(2026, 6, 2, 23, 11, 19, 0, locMinus8)

	_, _, _, tzoffsetMinus8 := exfatEncodeTimestamp(tMinus8)
	t.Logf("UTC-8 timezone offset encoded: 0x%X", tzoffsetMinus8)

	// For UTC-8, offset is -8 hours = -32 increments.
	// So 7-bit signed value should be -32, which is 224 (0xE0) in 2's complement of 8-bit, or -32 inside 7-bit:
	// -32 in 7-bit two's complement is 128 - 32 = 96 = 0x60.
	// Fully encoded value: 0x60 | 0x80 = 0xE0.
	
	expectedTzOffsetMinus8 := uint8(0xE0)
	if tzoffsetMinus8 != expectedTzOffsetMinus8 {
		t.Errorf("For UTC-8, expected tzoffset 0x%02X, got 0x%02X", expectedTzOffsetMinus8, tzoffsetMinus8)
	}
}
