#!/usr/bin/env python3
"""Deterministic structured seed generator for the OSS-Fuzz harnesses.

Writes three seed sets next to this script:

  seeds/         plain JSON documents (fuzz_parse, fuzz_basic_parser)
  seeds_parser/  6-byte framed inputs for fuzz_parser
                 (byte0: parse options+depth, byte1: strategy+number
                 precision, bytes2-3: memlimit1, bytes4-5: memlimit2,
                 then the JSON text)
  seeds_direct/  documents shaped like the described `Object` struct that
                 fuzz_direct_parse feeds to parse_into

Usage: python3 gen_seeds.py   (idempotent, deterministic)
"""

import os

HERE = os.path.dirname(os.path.abspath(__file__))

DOCS = [
    b'{"a":1}',
    b'{"key":"value","list":[1,2,3],"nested":{"b":true,"n":null}}',
    b'[true,false,null,1.5e10,-0.5]',
    b'{"escaped":"a\\/b\\tc\\u0041","big":1234567890123456789}',
    b'[[[[[1]]]]]',
    b'{"dup":1,"dup":2}',
    b'1.7976931348623157e308',
    b'{"comment":"// not a comment"}',
    b'{"\xc3\xa9":"\xf0\x9f\x8e\x89"}',
    b'{"a":1} x',
    b'{"a":1,}',
    b'/* c */ {"a":1}',
    b'{"bad":"\xff\xfe"}',
    b'1e-400',
    b'{"f":3.141592653589793,"d":2.718281828459045}',
    b'{"deep":{"deep":{"deep":{"deep":{"deep":1}}}}}',
]

# (options byte, strategy byte) combinations for the framed fuzz_parser set:
# options bit0 allow_comments, bit1 allow_trailing_commas, bit2
# allow_invalid_utf8, bits3-7 max_depth; strategy bits0-1 select the storage
# strategy, bits2-3 the number precision.
FRAMES = [
    (0x00, 0x00),  # defaults, default storage
    (0x07, 0x01),  # all options, max_depth 0, monotonic
    (0x2F, 0x02),  # no options, max_depth 5, local buffer
    (0x0B, 0x03),  # comments+trailing commas, dyn-less buffers
]


def write(relpath, data):
    path = os.path.join(HERE, relpath)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'wb') as f:
        f.write(data)


def framed(opts, strategy, doc):
    # memlimits 4096 bytes each, little-endian in two 256-scale bytes
    return bytes([opts, strategy, 0x10, 0x00, 0x10, 0x00]) + doc


FULL_DIRECT = (
    b'{"b":true,"i64":-1234567890,"u64":18446744073709551615,'
    b'"f":1.5,"d":2.25e-3,"s":"hello",'
    b'"v1":[true,false],"v2":[1,-2,3],"v3":[0,18446744073709551615],'
    b'"a1":[true,true,false],"a2":[1,2,3],"a3":[4,5,6],'
    b'"m1":{"x":1,"y":-2},"m2":{"k":"v"},"m3":{"pi":3.14},'
    b'"t1":[true,42,-42,1.25,"s"],'
    b'"t2":[["a","b","c"],[1.0,2.0,3.0],null],'
    b'"t3":[["p","q"],[0.5,1.5]],'
    b'"v":"i am a string",'
    b'"ob":true,"oi":7,"ou":8,"od":9.5,"os":"opt"}'
)

DIRECT_VARIANTS = [
    FULL_DIRECT,
    # without the optional members
    b'{"b":false,"i64":1,"u64":2,"f":0.0,"d":0.0,"s":"",'
    b'"v1":[],"v2":[7],"v3":[9],"a1":[false,true,false],'
    b'"a2":[-1,0,1],"a3":[2,3,4],"m1":{},"m2":{"a":"b"},"m3":{"x":-1.5},'
    b'"t1":[false,0,18446744073709551615,-0.0,""],'
    b'"t2":[["","",""],[0.0,0.0,0.0],null],'
    b'"t3":[[],[]],"v":true}',
    # wrong kinds in a few slots (error paths)
    b'{"b":"not a bool","i64":"not an int","v2":[1,"two",3],'
    b'"a1":[true,true],"t1":[1,2,3,4,5],"v":{"not":"a variant kind"}}',
    # missing members entirely
    b'{"b":true,"i64":1}',
    # array-shaped root (kind mismatch)
    b'[1,2,3]',
]


def main():
    for i, doc in enumerate(DOCS):
        write('seeds/plain_%02d.json' % i, doc)
    for i, doc in enumerate(DOCS):
        for j, (opts, strat) in enumerate(FRAMES):
            write('seeds_parser/framed_%02d_%d.bin' % (i, j),
                  framed(opts, strat, doc))
    for i, doc in enumerate(DIRECT_VARIANTS):
        write('seeds_direct/direct_%02d.json' % i, doc)
    print('seeds written under', HERE)


if __name__ == '__main__':
    main()
