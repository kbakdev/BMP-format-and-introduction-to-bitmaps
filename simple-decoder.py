#!/usr/bin/python
# -*- coding: utf-8 -*-
from struct import unpack
from zlib import crc32

# A small but very helpful class for reading data from a buffer.
class StreamReader:
    def __init__(self, data):
        self.offset = 0
        self.data = data
    
    def peek_block(self, size):
        return self.data[self.offset:self.offset + size]

    def get_block(self, size):
        b = self.data[self.offset:self.offset + size]
        self.offset += size
        return b

    def get_uint32(self):
        return unpack(">I", self.get_block(4))[0]

    def get_uint16(self):
        return unpack(">H", self.get_block(2))[0]

    def get_uint8(self):
        return self.get_block(1)

    # The exception is thrown on error in decoding.
    class PNGError(Exception):
        pass

    class PNGReader:
        def __init__(self, fname):
            self.fname = fname
            self.png = None
            self.bitmap_data = []
            self.header = { }
            self.bitmap = {
                "header": self.header,
                "rgb": [ ]
                }
        
        def _verify_magic(self):
            PNG_HEADER = "\x89PNG\r\n\xla\n"
            return PNG_HEADER == self.png.get_block(len(PNG_HEADER))

        def _read_chunk(self):
            # Load a single block.
            chunk = { }
            chunk["length"] = self.png.get_uint32()
            chunk["type"]   = self.png.get_block(4)
            chunk["data"]   = self.png.get_block(chunk["length"])
            chunk["crc32"]  = self.png.get_uint32()

            # CRC32 enumeration with data and type.
            crc = crc32(chunk["type"])
            crc = crc32(chunk["data"], crc) & 0xffffffff

            if chunk["crc32"] != crc:
                raise PNGError("chunk %s CRC32 incorrect" % chunk["type"])

            return chunk

        def _process_IHDR(self, chunk):
            # Load header fields.
            data = StreamReader(chunk["data"])
            self.header["width"]       = data.get_uint32()
            self.header["height"]      = data.get_uint32()
            self.header["bpp"]         = data.get_uint8()
            self.header["color"]       = data.get_uint8()
            self.header["compression"] = data.get_uint8()
            self.header["filter"]      = data.get_uint8()
            self.header["interlace"]   = data.get_uint8()

            # This decoder only supports 24-bpp non-interleaved PNG.
            if self.header["bpp"] != 8:
                raise PNGError("unsupported bpp (%u)" % self.header["bpp"])
            
            if self.header["color"] != 2: