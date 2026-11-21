"""Read flattened device trees and validate FIT checksums."""

import hashlib
import struct
import zlib

DEVICE = 'ruijie_rg-be72-pro-mod'
SPI = '/soc/spi@11007000'
QUAD_GROUPS = b'spi0\x00spi0_wp_hold\x00'


def tree(blob):
    magic, total, start, names, *_ = struct.unpack_from('>10I', blob)
    assert magic == 0xd00dfeed and total <= len(blob)
    path = []
    nodes = {}
    cursor = start
    while cursor < total:
        token, = struct.unpack_from('>I', blob, cursor)
        cursor += 4
        if token == 1:
            end = blob.index(0, cursor)
            path.append(blob[cursor:end].decode())
            nodes['/'.join(path) or '/'] = {}
            cursor = (end + 4) & ~3
        elif token == 2:
            path.pop()
        elif token == 3:
            length, name = struct.unpack_from('>II', blob, cursor)
            key = blob[names + name:blob.index(0, names + name)].decode()
            cursor += 8
            nodes['/'.join(path) or '/'][key] = blob[cursor:cursor + length]
            cursor = (cursor + length + 3) & ~3
        elif token == 9:
            return nodes
        elif token != 4:
            raise ValueError(f'Unexpected FDT token {token}')
    raise ValueError('Missing FDT end token')


def mux_node(nodes):
    selected = nodes[SPI]['pinctrl-0']
    active, = (path for path, entry in nodes.items() if entry.get('phandle') == selected)
    assert nodes[active + '/mux']['function'] == b'spi\x00'
    return active + '/mux'


def image_path(config, name):
    return '/images/' + config[name].rstrip(b'\x00').decode()


def verify_hashes(nodes, node):
    image = nodes[node]['data']
    assert nodes[node + '/hash-1']['value'] == struct.pack('>I', zlib.crc32(image))
    assert nodes[node + '/hash-2']['value'] == hashlib.sha1(image).digest()

