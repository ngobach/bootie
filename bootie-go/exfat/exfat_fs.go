package exfat

import (
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"os"
	"time"
	"unicode/utf16"

	"github.com/diskfs/go-diskfs/filesystem"
)

const (
	exfatFirstDataCluster = 2
	exfatClusterEnd       = 0xFFFFFFFF
	exfatClusterFree      = 0
	exfatClusterBad       = 0xFFFFFFF7
	exfatUpcaseChars      = 0x10000
	exfatNameMax          = 255
	exfatEnameMax         = 15

	exfatEntryValid     = 0x80
	exfatEntryFile      = 0x85
	exfatEntryFileInfo  = 0xC0
	exfatEntryFileName  = 0xC1
	exfatEntryBitmapType    = 0x81
	exfatEntryUpcaseType    = 0x82
	exfatEntryLabelType     = 0x83

	exfatFlagAlways1   = 0x01
	exfatFlagContiguous = 0x02

	exfatAttribRO     = 0x01
	exfatAttribHidden = 0x02
	exfatAttribSystem = 0x04
	exfatAttribVolume = 0x08
	exfatAttribDir    = 0x10
	exfatAttribArch   = 0x20
)

var errExfatNoDev = errors.New("exFAT: device not open")
var errExfatNotFound = errors.New("exFAT: file not found")

type exfatSuperBlock struct {
	Jump              [3]byte
	OemName           [8]byte
	_                 [53]byte
	SectorStart       uint64
	SectorCount       uint64
	FatSectorStart    uint32
	FatSectorCount    uint32
	ClusterSectorStart uint32
	ClusterCount      uint32
	RootdirCluster    uint32
	VolumeSerial      uint32
	VersionMinor      uint8
	VersionMajor      uint8
	VolumeState       uint16
	SectorBits        uint8
	SpcBits           uint8
	FatCount          uint8
	DriveNo           uint8
	AllocatedPercent  uint8
	_                 [397]byte
	BootSignature     uint16
}

func (sb *exfatSuperBlock) sectorSize() int64  { return 1 << sb.SectorBits }
func (sb *exfatSuperBlock) clusterSize() int64 { return int64(1<<sb.SectorBits) << sb.SpcBits }

type exfatNode struct {
	parent        *exfatNode
	child         *exfatNode
	next          *exfatNode
	prev          *exfatNode
	references    int
	fptrIndex     uint32
	fptrCluster   uint32
	entryOffset   int64
	startCluster  uint32
	attrib        uint16
	continuations uint8
	isContiguous  bool
	isCached      bool
	isDirty       bool
	isUnlinked    bool
	validSize     uint64
	size          uint64
	mtime         time.Time
	atime         time.Time
	name          []uint16
}

type exfatEntry struct {
	Type uint8
	Data [31]byte
}

type exfatEntryMeta1 struct {
	Type         uint8
	Continuations uint8
	Checksum     uint16
	Attrib       uint16
	_            uint16
	Crtime       uint16
	Crdate       uint16
	Mtime        uint16
	Mdate        uint16
	Atime        uint16
	Adate        uint16
	CrtimeCs     uint8
	MtimeCs      uint8
	CrtimeTzo    uint8
	MtimeTzo     uint8
	AtimeTzo     uint8
	_            [7]byte
}

type exfatEntryMeta2 struct {
	Type       uint8
	Flags      uint8
	_          uint8
	NameLength uint8
	NameHash   uint16
	_          [2]byte
	ValidSize  uint64
	_          [4]byte
	StartCluster uint32
	Size       uint64
}

type exfatEntryName struct {
	Type uint8
	_    uint8
	Name [exfatEnameMax]uint16
}

type exfatEntryBitmap struct {
	Type         uint8
	_            [19]byte
	StartCluster uint32
	Size         uint64
}

type exfatEntryUpcase struct {
	Type         uint8
	_            [3]byte
	Checksum     uint32
	_            [12]byte
	StartCluster uint32
	Size         uint64
}

type exfatEntryLabel struct {
	Type   uint8
	Length uint8
	Name   [exfatEnameMax]uint16
}

type exfatIterator struct {
	parent  *exfatNode
	current *exfatNode
}

type exfatFile struct {
	ef   *Exfat
	node *exfatNode
	pos  int64
}

func (f *exfatFile) Read(b []byte) (int, error) {
	n, err := exfatGenericPread(f.ef, f.node, b, int64(len(b)), f.pos)
	f.pos += int64(n)
	return n, err
}

func (f *exfatFile) Write(b []byte) (int, error) {
	n, err := exfatGenericPwrite(f.ef, f.node, b, int64(len(b)), f.pos)
	f.pos += int64(n)
	return n, err
}

func (f *exfatFile) Seek(offset int64, whence int) (int64, error) {
	switch whence {
	case io.SeekStart:
		f.pos = offset
	case io.SeekCurrent:
		f.pos += offset
	case io.SeekEnd:
		f.pos = int64(f.node.size) + offset
	}
	return f.pos, nil
}

func (f *exfatFile) Close() error {
	if err := exfatFlushNode(f.ef, f.node); err != nil {
		return err
	}
	exfatPutNode(f.ef, f.node)
	return nil
}

type exfatFileInfo struct {
	name    string
	size    int64
	mode    os.FileMode
	modTime time.Time
	isDir   bool
}

func (fi *exfatFileInfo) Name() string      { return fi.name }
func (fi *exfatFileInfo) Size() int64        { return fi.size }
func (fi *exfatFileInfo) Mode() os.FileMode   { return fi.mode }
func (fi *exfatFileInfo) ModTime() time.Time  { return fi.modTime }
func (fi *exfatFileInfo) IsDir() bool         { return fi.isDir }
func (fi *exfatFileInfo) Sys() interface{}    { return nil }

// Mount
func (e *Exfat) mount() error {
	return exfatMount(e)
}

// Close
func (e *Exfat) close() error {
	if !e.mounted {
		return nil
	}
	exfatFlushNodes(e)
	exfatFlush(e)
	exfatPutNode(e, e.root)
	exfatResetCache(e)
	e.mounted = false
	return nil
}

// Chmod
func (e *Exfat) chmod(name string, mode os.FileMode) error {
	const validMode = os.ModeSymlink | os.ModeDir | 0777
	if mode & ^validMode != 0 {
		return fmt.Errorf("invalid mode %#o", mode)
	}
	return nil
}

// Chown
func (e *Exfat) chown(name string, uid, gid int) error {
	return nil
}

// exfatGenericPread reads from a node at a given offset
func exfatGenericPread(ef *Exfat, node *exfatNode, b []byte, count int64, offset int64) (int, error) {
	if offset < 0 {
		return 0, errors.New("negative offset")
	}
	if uint64(offset) >= node.size {
		return 0, io.EOF
	}
	if count == 0 {
		return 0, nil
	}

	if uint64(offset)+uint64(count) > node.validSize {
		nread := int64(0)
		if uint64(offset) < node.validSize {
			n, err := exfatGenericPread(ef, node, b, int64(node.validSize)-offset, offset)
			if err != nil {
				return int(n), err
			}
			nread = int64(n)
		}
		remaining := int64(node.size) - offset
		if count > remaining {
			count = remaining
		}
		for i := nread; i < count; i++ {
			b[i] = 0
		}
		return int(count), nil
	}

	clusterSize := ef.sb.clusterSize()
	cluster := exfatAdvanceCluster(ef, node, uint32(uint64(offset)/uint64(clusterSize)))
	if cluster == exfatClusterBad {
		return 0, errors.New("invalid cluster while reading")
	}

	nread := int64(0)
	loffset := uint64(offset) % uint64(clusterSize)
	remainder := count
	if uint64(offset)+uint64(remainder) > node.size {
		remainder = int64(node.size) - offset
	}
	for remainder > 0 {
		if cluster == exfatClusterBad {
			return int(nread), errors.New("invalid cluster while reading")
		}
		lsize := int64(clusterSize) - int64(loffset)
		if remainder < lsize {
			lsize = remainder
		}
		off := exfatC2O(ef, cluster) + int64(loffset)
		if _, err := ef.dev.Seek(off, io.SeekStart); err != nil {
			return int(nread), err
		}
		if _, err := io.ReadFull(ef.dev, b[nread:nread+lsize]); err != nil {
			return int(nread), err
		}
		nread += lsize
		loffset = 0
		remainder -= lsize
		cluster = exfatNextCluster(ef, node, cluster)
	}
	return int(nread), nil
}

// exfatGenericPwrite writes to a node at a given offset
func exfatGenericPwrite(ef *Exfat, node *exfatNode, b []byte, count int64, offset int64) (int, error) {
	if offset < 0 {
		return 0, errors.New("negative offset")
	}
	if uint64(offset) > node.size {
		if err := exfatTruncate(ef, node, uint64(offset), true); err != nil {
			return 0, err
		}
	}
	if uint64(offset)+uint64(count) > node.size {
		if err := exfatTruncate(ef, node, uint64(offset)+uint64(count), false); err != nil {
			return 0, err
		}
	}
	if count == 0 {
		return 0, nil
	}

	clusterSize := ef.sb.clusterSize()
	cluster := exfatAdvanceCluster(ef, node, uint32(uint64(offset)/uint64(clusterSize)))
	if cluster == exfatClusterBad {
		return 0, errors.New("invalid cluster while writing")
	}

	nwritten := int64(0)
	loffset := uint64(offset) % uint64(clusterSize)
	remainder := count
	for remainder > 0 {
		if cluster == exfatClusterBad {
			return int(nwritten), errors.New("invalid cluster while writing")
		}
		lsize := int64(clusterSize) - int64(loffset)
		if remainder < lsize {
			lsize = remainder
		}
		off := exfatC2O(ef, cluster) + int64(loffset)
		if _, err := ef.dev.Seek(off, io.SeekStart); err != nil {
			return int(nwritten), err
		}
		if _, err := ef.dev.Write(b[nwritten : nwritten+lsize]); err != nil {
			return int(nwritten), err
		}
		nwritten += lsize
		loffset = 0
		remainder -= lsize
		newVal := uint64(offset) + uint64(nwritten)
		if newVal > node.validSize {
			node.validSize = newVal
		}
		cluster = exfatNextCluster(ef, node, cluster)
	}
	exfatUpdateMtime(node)
	return int(nwritten), nil
}

// exfatC2O converts cluster number to absolute offset
func exfatC2O(ef *Exfat, cluster uint32) int64 {
	if cluster < exfatFirstDataCluster {
		return 0
	}
	clusterSectorStart := int64(ef.sb.ClusterSectorStart)
	spc := int64(1 << ef.sb.SpcBits)
	sectorSize := int64(1 << ef.sb.SectorBits)
	return (clusterSectorStart + int64(cluster-exfatFirstDataCluster)*spc) * sectorSize
}

// exfatNextCluster returns the next cluster in the FAT chain
func exfatNextCluster(ef *Exfat, node *exfatNode, cluster uint32) uint32 {
	if cluster < exfatFirstDataCluster {
		return exfatClusterBad
	}
	if node.isContiguous {
		return cluster + 1
	}
	fatOffset := int64(ef.sb.FatSectorStart) * int64(1<<ef.sb.SectorBits) + int64(cluster)*4
	if _, err := ef.dev.Seek(fatOffset, io.SeekStart); err != nil {
		return exfatClusterBad
	}
	var next uint32
	if err := binary.Read(ef.dev, binary.LittleEndian, &next); err != nil {
		return exfatClusterBad
	}
	return next
}

// exfatAdvanceCluster advances the node's cluster pointer by count clusters
func exfatAdvanceCluster(ef *Exfat, node *exfatNode, count uint32) uint32 {
	if node.fptrIndex > count {
		node.fptrIndex = 0
		node.fptrCluster = node.startCluster
	}
	for i := node.fptrIndex; i < count; i++ {
		node.fptrCluster = exfatNextCluster(ef, node, node.fptrCluster)
		if node.fptrCluster >= exfatClusterBad {
			break
		}
	}
	node.fptrIndex = count
	return node.fptrCluster
}

// exfatTruncate truncates a node to the given size
func exfatTruncate(ef *Exfat, node *exfatNode, size uint64, erase bool) error {
	if node.size == size {
		return nil
	}
	clusterSize := uint64(ef.sb.clusterSize())
	c1 := (node.size + clusterSize - 1) / clusterSize
	c2 := (size + clusterSize - 1) / clusterSize

	var rc error
	if c1 < c2 {
		rc = growFile(ef, node, uint32(c1), uint32(c2-c1))
	} else if c1 > c2 {
		rc = shrinkFile(ef, node, uint32(c1), uint32(c1-c2))
	}
	if rc != nil {
		return rc
	}

	if erase && c1 < c2 {
		zeros := make([]byte, clusterSize)
		for c := c1; c < c2; c++ {
			cluster := exfatAdvanceCluster(ef, node, uint32(c))
			if cluster != exfatClusterBad && cluster != exfatClusterFree {
				offset := exfatC2O(ef, cluster)
				if _, err := ef.dev.Seek(offset, io.SeekStart); err != nil {
					return err
				}
				if _, err := ef.dev.Write(zeros); err != nil {
					return err
				}
			}
		}
	}

	if erase {
		node.validSize = size
	} else if size < node.validSize {
		node.validSize = size
	}
	exfatUpdateMtime(node)
	node.size = size
	node.isDirty = true
	return nil
}

func growFile(ef *Exfat, node *exfatNode, current, difference uint32) error {
	if difference == 0 {
		return nil
	}
	var previous uint32
	if node.startCluster != exfatClusterFree {
		previous = exfatAdvanceCluster(ef, node, current-1)
		if previous >= exfatClusterBad {
			return errors.New("invalid cluster while growing")
		}
	} else {
		previous = allocateCluster(ef, 0)
		if previous >= exfatClusterBad {
			return errors.New("no space left")
		}
		node.fptrCluster = previous
		node.startCluster = previous
		node.isContiguous = true
	}

	allocated := uint32(1)
	for allocated < difference {
		next := allocateCluster(ef, previous+1)
		if next >= exfatClusterBad {
			if allocated != 0 {
				shrinkFile(ef, node, current+allocated, allocated)
			}
			return errors.New("no space left")
		}
		if next != previous+1 && node.isContiguous {
			if err := makeNoncontiguous(ef, node.startCluster, previous); err != nil {
				return err
			}
			node.isContiguous = false
			node.isDirty = true
		}
		if err := setNextCluster(ef, node.isContiguous, previous, next); err != nil {
			return err
		}
		previous = next
		allocated++
	}
	return setNextCluster(ef, node.isContiguous, previous, exfatClusterEnd)
}

func shrinkFile(ef *Exfat, node *exfatNode, current, difference uint32) error {
	if difference == 0 || node.startCluster == exfatClusterFree {
		return nil
	}
	if current < difference {
		return errors.New("file underflow")
	}

	var previous uint32
	if current > difference {
		last := exfatAdvanceCluster(ef, node, current-difference-1)
		if last >= exfatClusterBad {
			return errors.New("invalid cluster while shrinking")
		}
		previous = exfatNextCluster(ef, node, last)
		if err := setNextCluster(ef, node.isContiguous, last, exfatClusterEnd); err != nil {
			return err
		}
	} else {
		previous = node.startCluster
		node.startCluster = exfatClusterFree
		node.isDirty = true
	}
	node.fptrIndex = 0
	node.fptrCluster = node.startCluster

	for difference > 0 {
		if previous >= exfatClusterBad {
			return errors.New("invalid cluster while freeing")
		}
		next := exfatNextCluster(ef, node, previous)
		if err := setNextCluster(ef, node.isContiguous, previous, exfatClusterFree); err != nil {
			return err
		}
		freeCluster(ef, previous)
		previous = next
		difference--
	}
	return nil
}

func allocateCluster(ef *Exfat, hint uint32) uint32 {
	hint -= exfatFirstDataCluster
	if hint >= ef.cmapSize {
		hint = 0
	}
	cluster := findBitAndSet(ef.cmapChunk, uint64(hint), uint64(ef.cmapSize))
	if cluster == exfatClusterEnd {
		cluster = findBitAndSet(ef.cmapChunk, 0, uint64(hint))
	}
	if cluster == exfatClusterEnd {
		return exfatClusterEnd
	}
	ef.cmapDirty = true
	return cluster + exfatFirstDataCluster
}

func freeCluster(ef *Exfat, cluster uint32) {
	index := cluster - exfatFirstDataCluster
	if uint64(index) >= uint64(ef.cmapSize) {
		return
	}
	block := index / 64
	mask := uint64(1) << (index % 64)
	ef.cmapChunk[block] &= ^mask
	ef.cmapDirty = true
}

func findBitAndSet(bitmap []uint64, start, end uint64) uint32 {
	startIdx := start / 64
	endIdx := (end + 63) / 64
	for i := startIdx; i < endIdx; i++ {
		if bitmap[i] == ^uint64(0) {
			continue
		}
		startBit := start
		if i*64 > start {
			startBit = i * 64
		}
		endBit := (i+1)*64 - 1
		if endBit >= end {
			endBit = end - 1
		}
		for c := startBit; c <= endBit; c++ {
			if bitmap[c/64]&(uint64(1)<<(c%64)) == 0 {
				bitmap[c/64] |= uint64(1) << (c % 64)
				return uint32(c)
			}
		}
	}
	return exfatClusterEnd
}

func setNextCluster(ef *Exfat, contiguous bool, current, next uint32) error {
	if contiguous {
		return nil
	}
	fatOffset := int64(ef.sb.FatSectorStart)*int64(1<<ef.sb.SectorBits) + int64(current)*4
	if _, err := ef.dev.Seek(fatOffset, io.SeekStart); err != nil {
		return err
	}
	return binary.Write(ef.dev, binary.LittleEndian, next)
}

func makeNoncontiguous(ef *Exfat, first, last uint32) error {
	for c := first; c < last; c++ {
		if err := setNextCluster(ef, false, c, c+1); err != nil {
			return err
		}
	}
	return nil
}

// Node management

func exfatGetNode(node *exfatNode) *exfatNode {
	node.references++
	return node
}

func exfatPutNode(ef *Exfat, node *exfatNode) {
	node.references--
}

func exfatResetCache(ef *Exfat) {
	resetCache(ef, ef.root)
}

func resetCache(ef *Exfat, node *exfatNode) {
	for node.child != nil {
		p := node.child
		resetCache(ef, p)
		treeDetach(p)
	}
	node.isCached = false
	for node.references > 0 {
		exfatPutNode(ef, node)
	}
}

func treeAttach(dir, node *exfatNode) {
	node.parent = dir
	if dir.child != nil {
		dir.child.prev = node
		node.next = dir.child
	}
	dir.child = node
}

func treeDetach(node *exfatNode) {
	if node.prev != nil {
		node.prev.next = node.next
	} else {
		node.parent.child = node.next
	}
	if node.next != nil {
		node.next.prev = node.prev
	}
	node.parent = nil
	node.prev = nil
	node.next = nil
}

// Directory entry operations

func readEntries(ef *Exfat, dir *exfatNode, entries []exfatEntry, offset int64) error {
	count := int64(len(entries))
	size := count * 32
	buf := make([]byte, size)
	n, err := exfatGenericPread(ef, dir, buf, size, offset)
	if err != nil {
		if n == 0 {
			return errExfatNotFound
		}
		return err
	}
	if n == 0 {
		return errExfatNotFound
	}
	if n != int(size) {
		return errors.New("short read of directory entries")
	}
	for i := range entries {
		entries[i].Type = buf[i*32]
		copy(entries[i].Data[:], buf[i*32+1:(i+1)*32])
	}
	return nil
}

func writeEntries(ef *Exfat, dir *exfatNode, entries []exfatEntry, offset int64) error {
	count := len(entries)
	size := count * 32
	buf := make([]byte, size)
	for i := range entries {
		buf[i*32] = entries[i].Type
		copy(buf[i*32+1:(i+1)*32], entries[i].Data[:])
	}
	_, err := exfatGenericPwrite(ef, dir, buf, int64(size), offset)
	return err
}

// exfatCacheDirectory reads all entries in a directory and builds the node cache
func exfatCacheDirectory(ef *Exfat, dir *exfatNode) error {
	if dir.isCached {
		return nil
	}
	var offset int64
	var current *exfatNode
	for {
		node, err := readDirEntry(ef, dir, &offset)
		if err != nil {
			if err == errExfatNotFound {
				break
			}
			return err
		}
		node.parent = dir
		if current != nil {
			current.next = node
			node.prev = current
		} else {
			dir.child = node
		}
		current = node
	}
	dir.isCached = true
	return nil
}

func readDirEntry(ef *Exfat, dir *exfatNode, offset *int64) (*exfatNode, error) {
	for {
		entries := make([]exfatEntry, 1)
		if err := readEntries(ef, dir, entries, *offset); err != nil {
			return nil, err
		}
		entry := entries[0]

		switch entry.Type {
		case exfatEntryFile:
			return parseFileEntry(ef, dir, offset, entry)
		case exfatEntryBitmapType:
			parseBitmapEntry(ef, entry)
		case exfatEntryUpcaseType:
			if err := parseUpcaseEntry(ef, entry); err != nil {
				return nil, err
			}
		case exfatEntryLabelType:
			parseLabelEntry(ef, entry)
		default:
			if entry.Type&exfatEntryValid == 0 {
				*offset += 32
				continue
			}
		}
		*offset += 32
	}
}

func parseFileEntry(ef *Exfat, dir *exfatNode, offset *int64, first exfatEntry) (*exfatNode, error) {
	data := [32]byte{first.Type}
	copy(data[1:], first.Data[:])

	continuations := data[1]
	n := 1 + int(continuations)

	entries := make([]exfatEntry, n)
	entries[0] = first
	if err := readEntries(ef, dir, entries[1:], *offset+32); err != nil {
		return nil, err
	}

	metamtime := binary.LittleEndian.Uint16(data[12:14])
	metamdate := binary.LittleEndian.Uint16(data[14:16])
	metaatime := binary.LittleEndian.Uint16(data[16:18])
	metaadate := binary.LittleEndian.Uint16(data[18:20])
	metamtimeCs := data[21]
	metamtimeTzo := data[23]
	metaatimeTzo := data[24]

	node := &exfatNode{
		entryOffset:   *offset,
		attrib:        binary.LittleEndian.Uint16(data[4:6]),
		continuations: continuations,
	}

	if n >= 2 {
		meta2Buf := [32]byte{entries[1].Type}
		copy(meta2Buf[1:], entries[1].Data[:])
		meta2 := readMeta2(meta2Buf)
		node.validSize = meta2.ValidSize
		node.size = meta2.Size
		node.startCluster = meta2.StartCluster
		node.fptrCluster = node.startCluster
		node.isContiguous = (meta2.Flags & exfatFlagContiguous) != 0

		// Read name from continuation entries
		nameEntries := n - 2
		for i := 0; i < nameEntries; i++ {
			ename := entries[2+i]
			var nameBuf [32]byte
			nameBuf[0] = ename.Type
			copy(nameBuf[1:], ename.Data[:])
			en := readEntryName(nameBuf)
			node.name = append(node.name, en.Name[:]...)
		}
		// Trim null terminator
		for len(node.name) > 0 && node.name[len(node.name)-1] == 0 {
			node.name = node.name[:len(node.name)-1]
		}
	}

	// Time parsing
	if n >= 1 {
		node.mtime = exfatDecodeTimestamp(metamdate, metamtime, metamtimeCs, metamtimeTzo)
		node.atime = exfatDecodeTimestamp(metaadate, metaatime, 0, metaatimeTzo)
	}

	*offset += 32 * int64(n)
	return node, nil
}

func parseBitmapEntry(ef *Exfat, entry exfatEntry) {
	var buf [32]byte
	buf[0] = entry.Type
	copy(buf[1:], entry.Data[:])
	be := readBitmapEntry(buf)
	ef.cmapStart = be.StartCluster
	ef.cmapSize = ef.sb.ClusterCount
	chunkSize := uint64(ef.cmapSize)
	bitmapBytes := (chunkSize + 7) / 8
	ef.cmapChunk = make([]uint64, (bitmapBytes+7)/8)

	off := exfatC2O(ef, be.StartCluster)
	if _, err := ef.dev.Seek(off, io.SeekStart); err != nil {
		return
	}
	bmap := make([]byte, bitmapBytes)
	if _, err := io.ReadFull(ef.dev, bmap); err != nil {
		return
	}
	for i := uint64(0); i < chunkSize; i++ {
		if bmap[i/8]&(1<<(i%8)) != 0 {
			ef.cmapChunk[i/64] |= uint64(1) << (i % 64)
		}
	}
}

func parseUpcaseEntry(ef *Exfat, entry exfatEntry) error {
	if ef.upcase != nil {
		return nil
	}
	var buf [32]byte
	buf[0] = entry.Type
	copy(buf[1:], entry.Data[:])
	ue := readUpcaseEntry(buf)

	upcaseSize := ue.Size
	if upcaseSize == 0 || upcaseSize > exfatUpcaseChars*2 || upcaseSize%2 != 0 {
		return errors.New("bad upcase table size")
	}

	src := make([]uint16, upcaseSize/2)
	off := exfatC2O(ef, ue.StartCluster)
	if _, err := ef.dev.Seek(off, io.SeekStart); err != nil {
		return err
	}
	if err := binary.Read(ef.dev, binary.LittleEndian, src); err != nil {
		return err
	}

	ef.upcase = make([]uint16, exfatUpcaseChars)
	for i := range ef.upcase {
		ef.upcase[i] = uint16(i)
	}
	oi := 0
	for si := 0; si < len(src) && oi < exfatUpcaseChars; si++ {
		ch := src[si]
		if ch == 0xFFFF && si+1 < len(src) {
			si++
			oi += int(src[si])
		} else {
			ef.upcase[oi] = ch
			oi++
		}
	}
	return nil
}

func parseLabelEntry(ef *Exfat, entry exfatEntry) {
	var buf [32]byte
	buf[0] = entry.Type
	copy(buf[1:], entry.Data[:])
	le := readLabelEntry(buf)
	length := le.Length
	if length > exfatEnameMax {
		length = exfatEnameMax
	}
	runes := utf16.Decode(le.Name[:length])
	ef.label = string(runes)
}

func readMeta2(buf [32]byte) exfatEntryMeta2 {
	return exfatEntryMeta2{
		Type:         buf[0],
		Flags:        buf[1],
		NameLength:   buf[3],
		NameHash:     binary.LittleEndian.Uint16(buf[4:6]),
		ValidSize:    binary.LittleEndian.Uint64(buf[8:16]),
		StartCluster: binary.LittleEndian.Uint32(buf[20:24]),
		Size:         binary.LittleEndian.Uint64(buf[24:32]),
	}
}

func readEntryName(buf [32]byte) exfatEntryName {
	var name [exfatEnameMax]uint16
	for i := 0; i < exfatEnameMax; i++ {
		name[i] = binary.LittleEndian.Uint16(buf[2+i*2:])
	}
	return exfatEntryName{Type: buf[0], Name: name}
}

func readBitmapEntry(buf [32]byte) exfatEntryBitmap {
	return exfatEntryBitmap{
		Type:         buf[0],
		StartCluster: binary.LittleEndian.Uint32(buf[20:24]),
		Size:         binary.LittleEndian.Uint64(buf[24:32]),
	}
}

func readUpcaseEntry(buf [32]byte) exfatEntryUpcase {
	return exfatEntryUpcase{
		Type:         buf[0],
		Checksum:     binary.LittleEndian.Uint32(buf[4:8]),
		StartCluster: binary.LittleEndian.Uint32(buf[20:24]),
		Size:         binary.LittleEndian.Uint64(buf[24:32]),
	}
}

func readLabelEntry(buf [32]byte) exfatEntryLabel {
	var name [exfatEnameMax]uint16
	for i := 0; i < exfatEnameMax; i++ {
		name[i] = binary.LittleEndian.Uint16(buf[2+i*2:])
	}
	return exfatEntryLabel{
		Type:   buf[0],
		Length: buf[1],
		Name:   name,
	}
}

// exfatFlushNode writes a dirty node back to disk
func exfatFlushNode(ef *Exfat, node *exfatNode) error {
	if !node.isDirty {
		return nil
	}
	if node.parent == nil {
		return nil
	}

	n := 1 + int(node.continuations)
	entries := make([]exfatEntry, n)
	if err := readEntries(ef, node.parent, entries, node.entryOffset); err != nil {
		return err
	}

	// Update meta1
	meta1Buf := make([]byte, 32)
	meta1Buf[0] = exfatEntryFile
	meta1Buf[1] = node.continuations
	binary.LittleEndian.PutUint16(meta1Buf[4:6], node.attrib)

	t := time.Now()
	edate, etime, ecs, etzo := exfatEncodeTimestamp(t)
	binary.LittleEndian.PutUint16(meta1Buf[8:10], etime)  // crtime
	binary.LittleEndian.PutUint16(meta1Buf[10:12], edate) // crdate
	binary.LittleEndian.PutUint16(meta1Buf[12:14], etime) // mtime
	binary.LittleEndian.PutUint16(meta1Buf[14:16], edate) // mdate
	binary.LittleEndian.PutUint16(meta1Buf[16:18], etime) // atime
	binary.LittleEndian.PutUint16(meta1Buf[18:20], edate) // adate
	meta1Buf[20] = ecs  // crtime_cs
	meta1Buf[21] = ecs  // mtime_cs
	meta1Buf[22] = etzo // crtime_tzo
	meta1Buf[23] = etzo // mtime_tzo
	meta1Buf[24] = etzo // atime_tzo

	entries[0].Type = meta1Buf[0]
	copy(entries[0].Data[:], meta1Buf[1:])

	// Update meta2
	if n >= 2 {
		meta2Buf := make([]byte, 32)
		meta2Buf[0] = exfatEntryFileInfo
		meta2Buf[1] = exfatFlagAlways1
		if node.size != 0 && node.isContiguous {
			meta2Buf[1] |= exfatFlagContiguous
		}
		meta2Buf[3] = uint8(len(node.name))
		binary.LittleEndian.PutUint16(meta2Buf[4:6], exfatCalcNameHash(ef, node.name))
		binary.LittleEndian.PutUint64(meta2Buf[8:16], node.validSize)
		binary.LittleEndian.PutUint32(meta2Buf[20:24], node.startCluster)
		binary.LittleEndian.PutUint64(meta2Buf[24:32], node.size)

		entries[1].Type = meta2Buf[0]
		copy(entries[1].Data[:], meta2Buf[1:])
	}

	// Update name entries
	for i := 2; i < n; i++ {
		nameBuf := make([]byte, 32)
		nameBuf[0] = exfatEntryFileName
		base := (i - 2) * exfatEnameMax
		for j := 0; j < exfatEnameMax && base+j < len(node.name); j++ {
			binary.LittleEndian.PutUint16(nameBuf[2+j*2:], node.name[base+j])
		}
		entries[i].Type = nameBuf[0]
		copy(entries[i].Data[:], nameBuf[1:])
	}

	// Calculate checksum
	checksum := exfatStartChecksum(entries[0])
	for i := 1; i < n; i++ {
		checksum = exfatAddChecksum(entries[i], checksum)
	}
	// Write checksum into meta1
	entries[0].Data[1] = byte(checksum)
	entries[0].Data[2] = byte(checksum >> 8)

	if err := writeEntries(ef, node.parent, entries, node.entryOffset); err != nil {
		return err
	}

	node.isDirty = false
	return exfatFlush(ef)
}

func exfatStartChecksum(entry exfatEntry) uint16 {
	var sum uint16
	buf := make([]byte, 32)
	buf[0] = entry.Type
	copy(buf[1:], entry.Data[:])
	for i := 0; i < 32; i++ {
		if i == 2 || i == 3 {
			continue
		}
		sum = ((sum << 15) | (sum >> 1)) + uint16(buf[i])
	}
	return sum
}

func exfatAddChecksum(entry exfatEntry, sum uint16) uint16 {
	buf := make([]byte, 32)
	buf[0] = entry.Type
	copy(buf[1:], entry.Data[:])
	for i := 0; i < 32; i++ {
		sum = ((sum << 15) | (sum >> 1)) + uint16(buf[i])
	}
	return sum
}

func exfatCalcNameHash(ef *Exfat, name []uint16) uint16 {
	var hash uint16
	for _, c := range name {
		c = ef.upcase[c]
		hash = ((hash << 15) | (hash >> 1)) + (c & 0xFF)
		hash = ((hash << 15) | (hash >> 1)) + (c >> 8)
	}
	return hash
}

func exfatFlushNodes(ef *Exfat) error {
	return flushNodes(ef, ef.root)
}

func flushNodes(ef *Exfat, node *exfatNode) error {
	for p := node.child; p != nil; p = p.next {
		if err := flushNodes(ef, p); err != nil {
			return err
		}
	}
	return exfatFlushNode(ef, node)
}

func exfatFlush(ef *Exfat) error {
	if !ef.cmapDirty {
		return nil
	}
	off := exfatC2O(ef, ef.cmapStart)
	size := uint64(ef.cmapSize)
	bitmapBytes := (size + 7) / 8
	bmap := make([]byte, bitmapBytes)
	for i := uint64(0); i < size; i++ {
		if ef.cmapChunk[i/64]&(uint64(1)<<(i%64)) != 0 {
			bmap[i/8] |= 1 << (i % 8)
		}
	}
	if _, err := ef.dev.Seek(off, io.SeekStart); err != nil {
		return err
	}
	if _, err := ef.dev.Write(bmap); err != nil {
		return err
	}
	ef.cmapDirty = false
	return nil
}

// Mount
func exfatMount(ef *Exfat) error {
	if ef.dev == nil {
		return errExfatNoDev
	}

	// Read superblock
	if _, err := ef.dev.Seek(0, io.SeekStart); err != nil {
		return err
	}
	if err := binary.Read(ef.dev, binary.LittleEndian, &ef.sb); err != nil {
		return err
	}

	if string(ef.sb.OemName[:]) != "EXFAT   " {
		return errors.New("exFAT file system is not found")
	}
	if ef.sb.SectorBits < 9 {
		return errors.New("too small sector size")
	}
	if int(ef.sb.SectorBits)+int(ef.sb.SpcBits) > 25 {
		return errors.New("too big cluster size")
	}
	if ef.sb.VersionMajor != 1 || ef.sb.VersionMinor != 0 {
		return fmt.Errorf("unsupported exFAT version: %d.%d", ef.sb.VersionMajor, ef.sb.VersionMinor)
	}
	if ef.sb.FatCount != 1 {
		return fmt.Errorf("unsupported FAT count: %d", ef.sb.FatCount)
	}

	// Initialize root node
	ef.root = &exfatNode{
		attrib:        exfatAttribDir,
		startCluster:  ef.sb.RootdirCluster,
		fptrCluster:   ef.sb.RootdirCluster,
	}
	ef.root.name = []uint16{0}
	ef.root.validSize = rootDirSize(ef)
	ef.root.size = ef.root.validSize
	if ef.root.size == 0 {
		return errors.New("failed to calculate root directory size")
	}
	exfatGetNode(ef.root)

	// Cache root directory
	if err := exfatCacheDirectory(ef, ef.root); err != nil {
		exfatPutNode(ef, ef.root)
		exfatResetCache(ef)
		return err
	}
	if ef.upcase == nil {
		return errors.New("upcase table is not found")
	}
	if ef.cmapChunk == nil {
		return errors.New("clusters bitmap is not found")
	}

	ef.mounted = true
	return nil
}

func rootDirSize(ef *Exfat) uint64 {
	clusters := uint32(0)
	maxClusters := ef.sb.ClusterCount
	cluster := ef.sb.RootdirCluster
	for cluster != exfatClusterEnd {
		if clusters >= maxClusters {
			return 0
		}
		if cluster < exfatFirstDataCluster || cluster >= exfatClusterBad {
			return 0
		}
		cluster = exfatNextCluster(ef, ef.root, cluster)
		clusters++
	}
	return uint64(clusters) * uint64(ef.sb.clusterSize())
}

// Path operations

func getComp(path string) (string, string) {
	// skip leading slashes
	i := 0
	for i < len(path) && path[i] == '/' {
		i++
	}
	path = path[i:]
	// find next slash
	for i = 0; i < len(path); i++ {
		if path[i] == '/' {
			return path[:i], path[i:]
		}
	}
	return path, ""
}

func exfatLookup(ef *Exfat, path string) (*exfatNode, error) {
	parent := exfatGetNode(ef.root)
	node := parent
	p := path
	for {
		comp, rest := getComp(p)
		if comp == "" {
			return node, nil
		}
		if comp == "." {
			p = rest
			continue
		}
		found, err := lookupName(ef, parent, comp)
		if err != nil {
			exfatPutNode(ef, parent)
			return nil, err
		}
		exfatPutNode(ef, parent)
		node = found
		parent = found
		p = rest
	}
}

func lookupName(ef *Exfat, parent *exfatNode, name string) (*exfatNode, error) {
	name16 := utf8ToUtf16(name)
	if err := exfatCacheDirectory(ef, parent); err != nil {
		return nil, err
	}
	it := exfatIterator{parent: parent}
	if err := exfatOpendir(ef, parent, &it); err != nil {
		return nil, err
	}
	for {
		node := exfatReaddir(&it)
		if node == nil {
			break
		}
		if compareName(ef, name16, node.name) == 0 {
			exfatClosedir(ef, &it)
			return node, nil
		}
		exfatPutNode(ef, node)
	}
	exfatClosedir(ef, &it)
	return nil, errExfatNotFound
}

func compareName(ef *Exfat, a, b []uint16) int {
	i := 0
	for i < len(a) && i < len(b) && a[i] != 0 && b[i] != 0 {
		ca := int(ef.upcase[a[i]])
		cb := int(ef.upcase[b[i]])
		if ca != cb {
			return ca - cb
		}
		i++
	}
	ca := 0
	if i < len(a) && a[i] != 0 {
		ca = int(ef.upcase[a[i]])
	}
	cb := 0
	if i < len(b) && b[i] != 0 {
		cb = int(ef.upcase[b[i]])
	}
	return ca - cb
}

func exfatOpendir(ef *Exfat, dir *exfatNode, it *exfatIterator) error {
	exfatGetNode(dir)
	it.parent = dir
	it.current = nil
	return exfatCacheDirectory(ef, dir)
}

func exfatClosedir(ef *Exfat, it *exfatIterator) {
	exfatPutNode(ef, it.parent)
	it.parent = nil
	it.current = nil
}

func exfatReaddir(it *exfatIterator) *exfatNode {
	if it.current == nil {
		it.current = it.parent.child
	} else {
		it.current = it.current.next
	}
	if it.current != nil {
		return exfatGetNode(it.current)
	}
	return nil
}

// ReadDir
func (e *Exfat) readDir(path string) ([]os.FileInfo, error) {
	if path == "" {
		path = "/"
	}
	dir, err := exfatLookup(e, path)
	if err != nil {
		return nil, err
	}
	if dir.attrib&exfatAttribDir == 0 {
		return nil, errors.New("not a directory")
	}
	defer exfatPutNode(e, dir)

	if err := exfatCacheDirectory(e, dir); err != nil {
		return nil, err
	}

	var result []os.FileInfo
	it := exfatIterator{parent: dir}
	exfatOpendir(e, dir, &it)
	defer exfatClosedir(e, &it)

	for {
		node := exfatReaddir(&it)
		if node == nil {
			break
		}
		name := utf16ToUtf8(node.name)
		fi := &exfatFileInfo{
			name:    name,
			size:    int64(node.size),
			modTime: node.mtime,
			isDir:   node.attrib&exfatAttribDir != 0,
		}
		if fi.isDir {
			fi.mode = os.ModeDir | 0755
		} else {
			fi.mode = 0644
		}
		result = append(result, fi)
		exfatPutNode(e, node)
	}
	return result, nil
}

// OpenFile
func (e *Exfat) openFile(path string, flag int) (filesystem.File, error) {
	node, err := exfatLookup(e, path)
	if err != nil {
		if flag&os.O_CREATE != 0 {
			if err := create(e, path, exfatAttribArch); err != nil {
				return nil, err
			}
			node, err = exfatLookup(e, path)
		}
		if err != nil {
			return nil, err
		}
	}
	if node.attrib&exfatAttribDir != 0 {
		return nil, errors.New("is a directory")
	}
	return &exfatFile{ef: e, node: exfatGetNode(node)}, nil
}

// Mkdir
func (e *Exfat) mkdir(path string) error {
	return exfatMkdir(e, path)
}

func exfatMkdir(ef *Exfat, path string) error {
	if err := create(ef, path, exfatAttribDir); err != nil {
		return err
	}
	node, err := exfatLookup(ef, path)
	if err != nil {
		return err
	}
	clusterSize := uint64(ef.sb.clusterSize())
	if err := exfatTruncate(ef, node, clusterSize, true); err != nil {
		deleteNode(ef, node)
		exfatPutNode(ef, node)
		return err
	}
	if err := exfatFlushNode(ef, node); err != nil {
		deleteNode(ef, node)
		exfatPutNode(ef, node)
		return err
	}
	exfatPutNode(ef, node)
	return nil
}

// Mknod
func (e *Exfat) mknod(path string) error {
	return create(e, path, exfatAttribArch)
}

// Remove
func (e *Exfat) remove(path string) error {
	node, err := exfatLookup(e, path)
	if err != nil {
		return err
	}

	var rc error
	if node.attrib&exfatAttribDir != 0 {
		rc = exfatRmdir(e, node)
	} else {
		rc = exfatUnlink(e, node)
	}

	exfatPutNode(e, node)
	if rc != nil {
		return rc
	}
	return exfatCleanupNode(e, node)
}

func exfatUnlink(ef *Exfat, node *exfatNode) error {
	if node.attrib&exfatAttribDir != 0 {
		return errors.New("is a directory")
	}
	return deleteNode(ef, node)
}

func exfatRmdir(ef *Exfat, node *exfatNode) error {
	if node.attrib&exfatAttribDir == 0 {
		return errors.New("not a directory")
	}
	if err := exfatCacheDirectory(ef, node); err != nil {
		return err
	}
	if node.child != nil {
		return errors.New("directory not empty")
	}
	return deleteNode(ef, node)
}

func exfatCleanupNode(ef *Exfat, node *exfatNode) error {
	if node.isUnlinked {
		return exfatTruncate(ef, node, 0, true)
	}
	return nil
}

func deleteNode(ef *Exfat, node *exfatNode) error {
	parent := node.parent

	exfatGetNode(parent)
	if err := eraseNode(ef, node); err != nil {
		exfatPutNode(ef, parent)
		return err
	}
	treeDetach(node)
	node.isUnlinked = true

	exfatUpdateMtime(parent)
	if err := exfatFlushNode(ef, parent); err != nil {
		exfatPutNode(ef, parent)
		return err
	}
	exfatPutNode(ef, parent)

	// Free clusters
	if node.references == 0 {
		exfatTruncate(ef, node, 0, true)
	}
	return nil
}

func eraseNode(ef *Exfat, node *exfatNode) error {
	exfatGetNode(node.parent)
	n := 1 + int(node.continuations)
	entries := make([]exfatEntry, n)
	if err := readEntries(ef, node.parent, entries, node.entryOffset); err != nil {
		exfatPutNode(ef, node.parent)
		return err
	}
	for i := range entries {
		entries[i].Type &^= exfatEntryValid
	}
	if err := writeEntries(ef, node.parent, entries, node.entryOffset); err != nil {
		exfatPutNode(ef, node.parent)
		return err
	}
	exfatPutNode(ef, node.parent)
	return nil
}

// Rename
func (e *Exfat) rename(oldPath, newPath string) error {
	return exfatRename(e, oldPath, newPath)
}

func exfatRename(ef *Exfat, oldPath, newPath string) error {
	node, err := exfatLookup(ef, oldPath)
	if err != nil {
		return err
	}
	defer exfatPutNode(ef, node)

	dir, name, err := exfatSplit(ef, newPath)
	if err != nil {
		return err
	}
	defer exfatPutNode(ef, dir)

	// Check if target is subdirectory of source
	if node.attrib&exfatAttribDir != 0 {
		for p := dir; p != nil; p = p.parent {
			if node == p {
				return errors.New("cannot move directory into itself")
			}
		}
	}

	name16 := utf8ToUtf16(name)
	offset, err := findSlot(ef, dir, 2+len(name16)/exfatEnameMax)
	if err != nil {
		return err
	}
	return renameEntry(ef, dir, node, name16, offset)
}

func renameEntry(ef *Exfat, dir *exfatNode, node *exfatNode, name []uint16, newOffset int64) error {
	n := 2 + (len(name)+exfatEnameMax-1)/exfatEnameMax

	// Read existing meta1 and meta2
	entries := make([]exfatEntry, 2)
	if err := readEntries(ef, node.parent, entries, node.entryOffset); err != nil {
		return err
	}

	// Update meta1
	entries[0].Data[0] = uint8(n - 1) // continuations

	// Update meta2
	entries[1].Data[2] = uint8(len(name)) // name_length
	nameHash := exfatCalcNameHash(ef, name)
	binary.LittleEndian.PutUint16(entries[1].Data[3:5], nameHash)

	// Erase old entry
	if err := eraseNode(ef, node); err != nil {
		return err
	}

	node.entryOffset = newOffset
	node.continuations = uint8(n - 1)

	// Build name entries
	nameEntries := make([]exfatEntry, n-2)
	for i := range nameEntries {
		nameEntries[i].Type = exfatEntryFileName
		base := i * exfatEnameMax
		for j := 0; j < exfatEnameMax && base+j < len(name); j++ {
			binary.LittleEndian.PutUint16(nameEntries[i].Data[1+j*2:], name[base+j])
		}
	}

	allEntries := make([]exfatEntry, n)
	allEntries[0] = entries[0]
	allEntries[1] = entries[1]
	copy(allEntries[2:], nameEntries)

	// Calculate checksum
	checksum := exfatStartChecksum(allEntries[0])
	for i := 1; i < n; i++ {
		checksum = exfatAddChecksum(allEntries[i], checksum)
	}
	allEntries[0].Data[1] = byte(checksum)
	allEntries[0].Data[2] = byte(checksum >> 8)

	if err := writeEntries(ef, dir, allEntries, newOffset); err != nil {
		return err
	}

	node.name = make([]uint16, len(name))
	copy(node.name, name)
	treeDetach(node)
	treeAttach(dir, node)
	return nil
}

func exfatSplit(ef *Exfat, path string) (parent *exfatNode, name string, err error) {
	// Find the parent directory and the last component
	parent = exfatGetNode(ef.root)
	p := path
	for {
		comp, rest := getComp(p)
		if comp == "" {
			if rest == "" {
				// Root path, no parent
				exfatPutNode(ef, parent)
				return nil, "", errors.New("cannot split root")
			}
			break
		}
		if rest == "" {
			// This is the last component
			name = comp
			return parent, name, nil
		}
		// Intermediate component, look it up
		node, err := lookupName(ef, parent, comp)
		if err != nil {
			exfatPutNode(ef, parent)
			return nil, "", err
		}
		exfatPutNode(ef, parent)
		parent = node
		p = rest
	}
	return parent, name, nil
}

// Create
func create(ef *Exfat, path string, attrib uint16) error {
	dir, name, err := exfatSplit(ef, path)
	if err != nil {
		return err
	}
	defer exfatPutNode(ef, dir)

	// Check if already exists
	if _, err := lookupName(ef, dir, name); err == nil {
		return errors.New("file already exists")
	}

	name16 := utf8ToUtf16(name)
	slotCount := 2 + (len(name16)+exfatEnameMax-1)/exfatEnameMax
	offset, err := findSlot(ef, dir, slotCount)
	if err != nil {
		return err
	}
	if err := commitEntry(ef, dir, name16, offset, attrib); err != nil {
		return err
	}
	exfatUpdateMtime(dir)
	return exfatFlushNode(ef, dir)
}

func findSlot(ef *Exfat, dir *exfatNode, n int) (int64, error) {
	if !dir.isCached {
		return 0, errors.New("directory is not cached")
	}
	dirSize := dir.size
	numEntries := dirSize / 32
	dmap := make([]bool, numEntries)

	for p := dir.child; p != nil; p = p.next {
		for i := 0; i < 1+int(p.continuations); i++ {
			idx := p.entryOffset/32 + int64(i)
			if idx < int64(len(dmap)) {
				dmap[idx] = true
			}
		}
	}

	// Mark metadata entries (label, bitmap, upcase) as occupied
	var scanOffset int64
	scanEntries := make([]exfatEntry, 1)
	for uint64(scanOffset) < dirSize {
		if err := readEntries(ef, dir, scanEntries, scanOffset); err != nil {
			break
		}
		if scanEntries[0].Type == exfatEntryFile {
			break
		}
		if scanEntries[0].Type&exfatEntryValid == 0 {
			break
		}
		idx := scanOffset / 32
		if idx < int64(len(dmap)) {
			dmap[idx] = true
		}
		scanOffset += 32
	}

	contiguous := 0
	var offset int64 = -1
	for i := int64(0); i < int64(len(dmap)); i++ {
		if !dmap[i] {
			if contiguous == 0 {
				offset = i * 32
			}
			contiguous++
			if contiguous == n {
				return offset, nil
			}
		} else {
			contiguous = 0
		}
	}

	// No slot found, extend directory
	newSize := dirSize
	if contiguous == 0 {
		offset = int64(dirSize)
	}
	newSize = ((dirSize + uint64(n-contiguous)*32) + uint64(ef.sb.clusterSize()) - 1) / uint64(ef.sb.clusterSize()) * uint64(ef.sb.clusterSize())
	if err := exfatTruncate(ef, dir, uint64(newSize), true); err != nil {
		return 0, err
	}
	return offset, nil
}

func commitEntry(ef *Exfat, dir *exfatNode, name []uint16, offset int64, attrib uint16) error {
	nameLength := len(name)
	nameEntries := (nameLength + exfatEnameMax - 1) / exfatEnameMax
	n := 2 + nameEntries
	allEntries := make([]exfatEntry, n)

	// Meta1
	meta1Buf := make([]byte, 32)
	meta1Buf[0] = exfatEntryFile
	meta1Buf[1] = uint8(n - 1)
	binary.LittleEndian.PutUint16(meta1Buf[4:6], attrib)

	now := time.Now()
	edate, etime, ecs, etzo := exfatEncodeTimestamp(now)
	binary.LittleEndian.PutUint16(meta1Buf[8:10], etime)  // crtime
	binary.LittleEndian.PutUint16(meta1Buf[10:12], edate) // crdate
	binary.LittleEndian.PutUint16(meta1Buf[12:14], etime) // mtime
	binary.LittleEndian.PutUint16(meta1Buf[14:16], edate) // mdate
	binary.LittleEndian.PutUint16(meta1Buf[16:18], etime) // atime
	binary.LittleEndian.PutUint16(meta1Buf[18:20], edate) // adate
	meta1Buf[20] = ecs  // crtime_cs
	meta1Buf[21] = ecs  // mtime_cs
	meta1Buf[22] = etzo // crtime_tzo
	meta1Buf[23] = etzo // mtime_tzo
	meta1Buf[24] = etzo // atime_tzo

	allEntries[0].Type = meta1Buf[0]
	copy(allEntries[0].Data[:], meta1Buf[1:])

	// Meta2
	meta2Buf := make([]byte, 32)
	meta2Buf[0] = exfatEntryFileInfo
	meta2Buf[1] = exfatFlagAlways1
	meta2Buf[3] = uint8(nameLength)
	meta2Buf[4] = byte(exfatCalcNameHash(ef, name))
	meta2Buf[5] = byte(exfatCalcNameHash(ef, name) >> 8)
	binary.LittleEndian.PutUint32(meta2Buf[20:24], exfatClusterFree)

	allEntries[1].Type = meta2Buf[0]
	copy(allEntries[1].Data[:], meta2Buf[1:])

	// Name entries
	for i := 0; i < nameEntries; i++ {
		nameBuf := make([]byte, 32)
		nameBuf[0] = exfatEntryFileName
		base := i * exfatEnameMax
		for j := 0; j < exfatEnameMax && base+j < nameLength; j++ {
			binary.LittleEndian.PutUint16(nameBuf[2+j*2:], name[base+j])
		}
		allEntries[2+i].Type = nameBuf[0]
		copy(allEntries[2+i].Data[:], nameBuf[1:])
	}

	// Calculate checksum
	checksum := exfatStartChecksum(allEntries[0])
	for i := 1; i < n; i++ {
		checksum = exfatAddChecksum(allEntries[i], checksum)
	}
	allEntries[0].Data[1] = byte(checksum)
	allEntries[0].Data[2] = byte(checksum >> 8)

	if err := writeEntries(ef, dir, allEntries, offset); err != nil {
		return err
	}

	// Build in-memory node
	node := &exfatNode{
		entryOffset:   offset,
		attrib:        attrib,
		continuations: uint8(n - 1),
		startCluster:  exfatClusterFree,
		fptrCluster:   exfatClusterFree,
		validSize:     0,
		size:          0,
	}
	node.name = make([]uint16, nameLength)
	copy(node.name, name)

	exfatUpdateMtime(node)
	node.mtime = now
	node.atime = now

	treeAttach(dir, node)
	return nil
}

// Label
func (e *Exfat) setLabel(label string) error {
	label16 := utf8ToUtf16(label)
	length := len(label16)
	if length > exfatEnameMax {
		length = exfatEnameMax
		label16 = label16[:length]
	}

	// Find existing label entry
	offset, err := findLabelEntry(e)
	if err == errExfatNotFound {
		offset, err = findSlot(e, e.root, 1)
		if err != nil {
			return err
		}
	}

	var buf [32]byte
	buf[0] = exfatEntryLabelType
	buf[1] = uint8(length)
	for i := 0; i < length; i++ {
		binary.LittleEndian.PutUint16(buf[2+i*2:], label16[i])
	}
	if length == 0 {
		buf[0] &^= exfatEntryValid
	}

	entry := exfatEntry{Type: buf[0]}
	copy(entry.Data[:], buf[1:])
	if err := writeEntries(e, e.root, []exfatEntry{entry}, offset); err != nil {
		return err
	}

	e.label = label
	return nil
}

func findLabelEntry(ef *Exfat) (int64, error) {
	var offset int64
	for {
		var entry exfatEntry
		if err := readEntries(ef, ef.root, []exfatEntry{entry}, offset); err != nil {
			return 0, err
		}
		if entry.Type == exfatEntryLabelType {
			return offset, nil
		}
		offset += 32
	}
}

// Time conversion helpers

func exfatDecodeTimestamp(date, etime uint16, centisec uint8, tzoffset uint8) time.Time {
	day := int(date & 0x1F)
	month := int((date >> 5) & 0x0F)
	year := int(date>>9) + 1980
	hour := int((etime >> 11) & 0x1F)
	min := int((etime >> 5) & 0x3F)
	sec := int(etime&0x1F) * 2
	if sec > 59 {
		sec = 59
	}
	var loc *time.Location
	if tzoffset&0x80 != 0 {
		offsetVal := int(tzoffset & 0x7F)
		if offsetVal >= 64 {
			offsetVal -= 128
		}
		offsetSec := offsetVal * 15 * 60
		loc = time.FixedZone("", offsetSec)
	} else {
		loc = time.Local
	}
	return time.Date(year, time.Month(month), day, hour, min, sec, 0, loc)
}

func exfatEncodeTimestamp(t time.Time) (edate, etime uint16, centisec uint8, tzoffset uint8) {
	year := t.Year() - 1980
	if year < 0 {
		year = 0
	}
	month := int(t.Month())
	day := t.Day()
	hour := t.Hour()
	min := t.Minute()
	sec := t.Second()
	twosec := sec / 2
	if twosec > 29 {
		twosec = 29
	}

	edate = uint16(day) | (uint16(month) << 5) | (uint16(year) << 9)
	etime = uint16(twosec) | (uint16(min) << 5) | (uint16(hour) << 11)
	centisec = uint8((sec % 2) * 100)
	_, offset := t.Zone()
	tzoffset = uint8(offset/60/15) | 0x80
	return
}

func exfatUpdateMtime(node *exfatNode) {
	node.mtime = time.Now()
	node.isDirty = true
}

// UTF conversion helpers

func utf8ToUtf16(s string) []uint16 {
	// Count runes
	runes := make([]rune, 0, len(s))
	for _, r := range s {
		runes = append(runes, r)
	}
	return utf16.Encode(runes)
}

func utf16ToUtf8(u []uint16) string {
	runes := utf16.Decode(u)
	return string(runes)
}

// exfatStat fills os.FileInfo for a node
func exfatStat(ef *Exfat, node *exfatNode) *exfatFileInfo {
	name := utf16ToUtf8(node.name)
	mode := os.FileMode(0644)
	isDir := node.attrib&exfatAttribDir != 0
	if isDir {
		mode = os.ModeDir | 0755
	}
	return &exfatFileInfo{
		name:    name,
		size:    int64(node.size),
		mode:    mode,
		modTime: node.mtime,
		isDir:   isDir,
	}
}

// Ensure exfatFile implements filesystem.File
var _ filesystem.File = (*exfatFile)(nil)

// Ensure Exfat implements filesystem.FileSystem
var _ filesystem.FileSystem = (*Exfat)(nil)


