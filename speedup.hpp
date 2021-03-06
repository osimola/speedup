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

#pragma once

#include<cstdint>
#include<cstddef>

/* Non-mutating functions. */

uint64_t simple_loop(const uint8_t *buf, size_t bufsize);
uint64_t cheaty_mccheatface(const uint8_t *buf, size_t bufsize);
uint64_t lookup_table(const uint8_t *buf, size_t bufsize);
uint64_t bit_fiddling(const uint8_t *buf, size_t bufsize);
uint64_t bucket(const uint8_t *buf, size_t bufsize);
uint64_t multiply_filter(const uint8_t *buf, size_t bufsize);
uint64_t parallel_add_lookup(const uint8_t *buf, size_t bufsize);

/* Mutating functions */

uint64_t partition(uint8_t *buf, size_t bufsize);
uint64_t zeroing(uint8_t *buf, size_t bufsize);
