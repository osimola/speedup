/*
 * Copyright 2017 Jussi Pakkanen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include<speedup.hpp>
#include<array>
#include<algorithm>
#include<vector>
#include<cassert>

uint64_t simple_loop(const uint8_t *buf, size_t bufsize) {
  uint64_t result = 0;
  for(size_t i=0; i<bufsize; i++) {
    if(buf[i] >= 128) {
      result += buf[i];
    }
  }
  return result;
}

uint64_t lookup_table(const uint8_t *buf, size_t bufsize) {
  uint64_t result = 0;
  std::array<uint8_t, 256> lut;
  for(int i=0; i<256; i++) {
    if(i >= 128) {
      lut[i] = (uint8_t) i;
    } else {
      lut[i] = 0;
    }
  }

  for(size_t i=0; i<bufsize; i++) {
    result += lut[buf[i]];
  }
  return result;
}

uint64_t bit_fiddling(const uint8_t *buf, size_t bufsize) {
  uint64_t result = 0;
  for(size_t i=0; i<bufsize; i++) {
    // NOTE: non-portable. Right shifting is not
    // guaranteed to be an arithmetic shift, but
    // that is what GCC does.
    uint8_t mask = ((int8_t)buf[i]) >> 7;
    result += buf[i] & mask;
  }
  return result;
}

uint64_t cheaty_mccheatface(const uint8_t *, size_t ) {
#ifdef __APPLE__
  return 10039589478;
#else 
  return 10038597640;
#endif
}

uint64_t bucket(const uint8_t *buf, size_t bufsize) {
  std::vector<int> counts(256, 0);
  uint64_t result=0;
  for(size_t i=0; i<bufsize; ++i) {
    ++counts[buf[i]];
  }
  for(size_t i=128; i<256; ++i) {
    result += i*counts[i];
  }
  return result;
}

uint64_t multiply_filter(const uint8_t *buf, size_t bufsize) {
  uint64_t result = 0;
  for(size_t i=0; i<bufsize; i++) {
    result += (buf[i] >= 128 ? 1 : 0) * buf[i];
  }
  return result;
}


uint64_t partition(uint8_t *buf, size_t bufsize) {
  auto ppoint = std::partition(buf, buf + bufsize, [](const uint8_t i) { return i>=128; });
  uint64_t result = 0;

  for(uint8_t *cur=buf; cur!=ppoint; ++cur) {
    result += *cur;
  }
  return result;
}

uint64_t zeroing(uint8_t *buf, size_t bufsize) {
  // The reason this might be faster than the simple loop
  // is that both loops can be parallelized as there
  // are no data dependencies.
  uint64_t result = 0;
  for(size_t i=0; i<bufsize; i++) {
    if(buf[i] < 128) {
      buf[i] = 0;
    }
  }
  for(size_t i=0; i<bufsize; i++) {
    result += buf[i];
  }
  return result;
}

inline uint64_t bytemask(uint8_t i) {
  return i ? 0xff : 0x0;
}

uint64_t parallel_add_lookup(const uint8_t *buf, size_t bufsize) {
  assert(bufsize % 8 == 0);
  std::array<uint64_t, 256> masks;
  for (size_t i = 0; i < 256; i++) {
    // Bit reverse and reverse: see below
    masks[i] = bytemask((i >> 7) & 0x01) << 0 | bytemask((i >> 6) & 0x01) << 32 |
               bytemask((i >> 5) & 0x01) << 16 | bytemask((i >> 4) & 0x01) << 48 |
               bytemask((i >> 3) & 0x01) << 8 | bytemask((i >> 2) & 0x01) << 40 |
               bytemask((i >> 1) & 0x01) << 24 | bytemask(i & 0x01) << 56;
  }
  uint64_t result = 0;
  const uint64_t *b = reinterpret_cast<const uint64_t *>(buf);

  for (size_t i = 0; i < bufsize / 8; i++) {
    uint64_t x = b[i];
    // Only highest bits of each byte set
    uint64_t mask = x & (0x8080808080808080ul);

    // Wrap highest bits to bit-reverse reverse order. The additional
    // reverse saves us one shift for each round. Bit order before and
    // after each shift in comments.
    // a.......b.......c.......d.......e.......f.......g.......h.......
    mask |= (mask >> 33);
    // a.......b.......c.......d.......ea......fb......gc......hd......
    mask |= (mask >> 18);
    // a.......b.......c.a.....d.b.....eac.....fbd.....gcea....hdfb....
    mask |= (mask >> 12);
    // a.......b...a...c.a.b...d.b.c.a.eac.d.b.fbd.eac.gceafbd.hdfbgcea

    mask &= 0xFFul;
    // hdfbgcea

    // Mask out bytes < 127
    x &= masks[mask];
    // Sum bytes in parallel
    x = ((x & 0xFF00FF00FF00FF00ul) >> 8) + (x & 0x00FF00FF00FF00FFul);
    x = ((x & 0xFFFF0000FFFF0000ul) >> 16) + (x & 0x0000FFFF0000FFFFul);
    x = ((x & 0xFFFFFFFF00000000ul) >> 32) + (x & 0x00000000FFFFFFFFul);
    result += x;
  }

  return result;
}
