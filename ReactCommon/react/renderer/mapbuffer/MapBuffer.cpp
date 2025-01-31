/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "MapBuffer.h"

using namespace facebook::react;

namespace facebook {
namespace react {

MapBuffer::MapBuffer(uint8_t *const data, uint16_t dataSize) {
  // Should we move the memory here or document it?
  _data = data;

  _count = 0;
  memcpy(
      reinterpret_cast<uint8_t *>(&_count),
      reinterpret_cast<const uint8_t *>(_data + HEADER_COUNT_OFFSET),
      UINT16_SIZE);

  // TODO: extract memcpy calls into an inline function to simplify the code
  _dataSize = 0;
  memcpy(
      reinterpret_cast<uint8_t *>(&_dataSize),
      reinterpret_cast<const uint8_t *>(_data + HEADER_BUFFER_SIZE_OFFSET),
      UINT16_SIZE);

  if (dataSize != _dataSize) {
    LOG(ERROR) << "Error: Data size does not match, expected " << dataSize
               << " found: " << _dataSize;
    throw "Error: Data size does not match";
  }
}

int MapBuffer::getInt(Key key) const {
  int value = 0;
  memcpy(
      reinterpret_cast<uint8_t *>(&value),
      reinterpret_cast<const uint8_t *>(_data + getValueOffset(key)),
      INT_SIZE);
  return value;
}

bool MapBuffer::getBool(Key key) const {
  return getInt(key) != 0;
}

double MapBuffer::getDouble(Key key) const {
  // TODO: extract this code into a "template method" and reuse it for other
  // types
  double value = 0;
  memcpy(
      reinterpret_cast<uint8_t *>(&value),
      reinterpret_cast<const uint8_t *>(_data + getValueOffset(key)),
      DOUBLE_SIZE);
  return value;
}

int MapBuffer::getDynamicDataOffset() const {
  // The begininig of dynamic data can be calculated as the offset of the next
  // key in the map
  return getKeyOffset(_count);
}

std::string MapBuffer::getString(Key key) const {
  // TODO Add checks to verify that offsets are under the boundaries of the map
  // buffer
  int dynamicDataOffset = getDynamicDataOffset();
  int stringLength = 0;
  memcpy(
      reinterpret_cast<uint8_t *>(&stringLength),
      reinterpret_cast<const uint8_t *>(_data + dynamicDataOffset),
      INT_SIZE);

  int valueOffset = getInt(key) + sizeof(stringLength);

  char *value = new char[stringLength];

  memcpy(
      reinterpret_cast<char *>(value),
      reinterpret_cast<const char *>(_data + dynamicDataOffset + valueOffset),
      stringLength);

  return std::string(value);
}

MapBuffer MapBuffer::getMapBuffer(Key key) const {
  // TODO Add checks to verify that offsets are under the boundaries of the map
  // buffer
  int dynamicDataOffset = getDynamicDataOffset();

  uint16_t mapBufferLength = 0;

  memcpy(
      reinterpret_cast<uint8_t *>(&mapBufferLength),
      reinterpret_cast<const uint8_t *>(_data + dynamicDataOffset),
      UINT16_SIZE);

  int valueOffset = getInt(key) + UINT16_SIZE;

  uint8_t *value = new Byte[mapBufferLength];

  memcpy(
      reinterpret_cast<uint8_t *>(value),
      reinterpret_cast<const uint8_t *>(
          _data + dynamicDataOffset + valueOffset),
      mapBufferLength);

  return MapBuffer(value, mapBufferLength);
}

bool MapBuffer::isNull(Key key) const {
  return getInt(key) == NULL_VALUE;
}

uint16_t MapBuffer::getBufferSize() const {
  return _dataSize;
}

void MapBuffer::copy(uint8_t *output) const {
  memcpy(output, _data, _dataSize);
}

uint16_t MapBuffer::getCount() const {
  uint16_t size = 0;

  memcpy(
      reinterpret_cast<uint16_t *>(&size),
      reinterpret_cast<const uint16_t *>(
          _data + UINT16_SIZE), // TODO refactor this: + UINT16_SIZE describes
                                // the position in the header
      UINT16_SIZE);

  return size;
}

MapBuffer::~MapBuffer() {
  delete[] _data;
}

} // namespace react
} // namespace facebook
