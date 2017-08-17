/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <limits>
#include <random>

#include <gtest/gtest.h>

#include <geode/DataOutput.hpp>
#include <geode/CacheFactory.hpp>
#include "ByteArrayFixture.hpp"
#include "DataOutputInternal.hpp"
#include "SerializationRegistry.hpp"

namespace {

using namespace apache::geode::client;

class TestDataOutput : public DataOutputInternal {
 public:
  TestDataOutput(Cache* cache)
      : DataOutputInternal(cache), m_byteArray(nullptr), m_serializationRegistry() {
    // NOP
  }

  virtual ~TestDataOutput() {
    delete m_byteArray;
    m_byteArray = nullptr;
  }

  const ByteArray& getByteArray() const {
    if (!m_byteArray) {
      m_byteArray = new ByteArray(getBuffer(), getBufferLength());
    }
    return *m_byteArray;
  }

 protected:
  virtual const SerializationRegistry& getSerializationRegistry()
      const override {
    return m_serializationRegistry;
  }

 private:
  mutable ByteArray* m_byteArray;
  SerializationRegistry m_serializationRegistry;
};

class DataOutputTest : public ::testing::Test, public ByteArrayFixture {
 public:
  DataOutputTest() : m_mersennesTwister(m_randomDevice()) {
    // NOP
  }

  virtual ~DataOutputTest() {
    // NOP
  }

 protected:
  std::random_device m_randomDevice;
  std::mt19937 m_mersennesTwister;

  int32_t getRandomSequenceNumber() {
    // One would normally just use std::uniform_int_distribution but gcc 4.4.7
    // is lacking.
    const std::mt19937::result_type upperLimit =
        static_cast<std::mt19937::result_type>(
            std::numeric_limits<int32_t>::max());
    std::mt19937::result_type result;
    while (upperLimit < (result = m_mersennesTwister())) {
      // Try again.
    }
    return static_cast<int32_t>(result);
  }
};

TEST_F(DataOutputTest, TestWriteUint8) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.write(static_cast<uint8_t>(55U));
  dataOutput.write(static_cast<uint8_t>(66U));
  EXPECT_BYTEARRAY_EQ("3742", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteInt8) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.write(static_cast<int8_t>(66));
  dataOutput.write(static_cast<int8_t>(55));
  EXPECT_BYTEARRAY_EQ("4237", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteSequenceNumber) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeInt((int32_t)55);
  dataOutput.writeInt((int32_t)17);
  dataOutput.writeInt((int32_t)0);
  dataOutput.writeInt(getRandomSequenceNumber());
  dataOutput.write(static_cast<uint8_t>(0U));
  EXPECT_BYTEARRAY_EQ("000000370000001100000000\\h{8}00",
                      dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteBoolean) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeBoolean(true);
  dataOutput.writeBoolean(false);
  EXPECT_BYTEARRAY_EQ("0100", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteBytesSigned) {
  int8_t bytes[] = {0, 1, 2, 3, 4, 5, -4, -3, -2, -1, 0};

  TestDataOutput dataOutput(nullptr);
  dataOutput.writeBytes(bytes, 11);
  EXPECT_BYTEARRAY_EQ("0B000102030405FCFDFEFF00", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteBytesOnlyUnsigned) {
  uint8_t bytes[] = {0, 1, 2, 3, 4, 5, 4, 3, 2, 1, 0};

  TestDataOutput dataOutput(nullptr);
  dataOutput.writeBytesOnly(bytes, 11);
  EXPECT_BYTEARRAY_EQ("0001020304050403020100", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteBytesOnlySigned) {
  int8_t bytes[] = {0, 1, 2, 3, 4, 5, -4, -3, -2, -1, 0};

  TestDataOutput dataOutput(nullptr);
  dataOutput.writeBytesOnly(bytes, 11);
  EXPECT_BYTEARRAY_EQ("000102030405FCFDFEFF00", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteIntUInt16) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeInt(static_cast<uint16_t>(66));
  dataOutput.writeInt(static_cast<uint16_t>(55));
  dataOutput.writeInt(static_cast<uint16_t>(3333));
  EXPECT_BYTEARRAY_EQ("004200370D05", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteCharUInt16) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeChar(static_cast<uint16_t>(66));
  dataOutput.writeChar(static_cast<uint16_t>(55));
  dataOutput.writeChar(static_cast<uint16_t>(3333));
  EXPECT_BYTEARRAY_EQ("004200370D05", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteIntUInt32) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeInt(static_cast<uint32_t>(3435973836));
  EXPECT_BYTEARRAY_EQ("CCCCCCCC", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteIntUInt64) {
  TestDataOutput dataOutput(nullptr);
  uint64_t big = 13455272147882261178U;
  dataOutput.writeInt(big);
  EXPECT_BYTEARRAY_EQ("BABABABABABABABA", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteIntInt16) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeInt(static_cast<int16_t>(66));
  dataOutput.writeInt(static_cast<int16_t>(55));
  dataOutput.writeInt(static_cast<int16_t>(3333));
  EXPECT_BYTEARRAY_EQ("004200370D05", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteIntInt32) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeInt(static_cast<int32_t>(3435973836));
  EXPECT_BYTEARRAY_EQ("CCCCCCCC", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteIntInt64) {
  TestDataOutput dataOutput(nullptr);
  int64_t big = 773738426788457421;
  dataOutput.writeInt(big);
  EXPECT_BYTEARRAY_EQ("0ABCDEFFEDCBABCD", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteArrayLength) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeArrayLen(static_cast<int32_t>(3435973836));
  EXPECT_BYTEARRAY_EQ("CC", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteFloat) {
  TestDataOutput dataOutput(nullptr);
  float pi = 3.14f;
  dataOutput.writeFloat(pi);
  EXPECT_BYTEARRAY_EQ("4048F5C3", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteDouble) {
  TestDataOutput dataOutput(nullptr);
  double pi = 3.14159265359;
  dataOutput.writeDouble(pi);
  EXPECT_BYTEARRAY_EQ("400921FB54442EEA", dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteASCII) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeASCII("You had me at meat tornado.");
  EXPECT_BYTEARRAY_EQ(
      "001B596F7520686164206D65206174206D65617420746F726E61646F2E",
      dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteNativeString) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeNativeString("You had me at meat tornado.");
  EXPECT_BYTEARRAY_EQ(
      "57001B596F7520686164206D65206174206D65617420746F726E61646F2E",
      dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteASCIIHuge) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeASCIIHuge("You had me at meat tornado.");
  EXPECT_BYTEARRAY_EQ(
      "0000001B596F7520686164206D65206174206D65617420746F726E61646F2E",
      dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteFullUTF) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeFullUTF("You had me at meat tornado.");
  EXPECT_BYTEARRAY_EQ(
      "0000001B00596F7520686164206D65206174206D65617420746F726E61646F2E",
      dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteUTF) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeUTF("You had me at meat tornado.");
  EXPECT_BYTEARRAY_EQ(
      "001B596F7520686164206D65206174206D65617420746F726E61646F2E",
      dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteUTFHuge) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeUTFHuge("You had me at meat tornado.");
  EXPECT_BYTEARRAY_EQ(
      "0000001B0059006F007500200068006100640020006D00650020006100740020006D0065"
      "0061007400200074006F0072006E00610064006F002E",
      dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteUTFWide) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeUTF(L"You had me at meat tornado!");
  EXPECT_BYTEARRAY_EQ(
      "001B596F7520686164206D65206174206D65617420746F726E61646F21",
      dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteUTFHugeWide) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeUTFHuge(L"You had me at meat tornado.");
  EXPECT_BYTEARRAY_EQ(
      "0000001B0059006F007500200068006100640020006D00650020006100740020006D0065"
      "0061007400200074006F0072006E00610064006F002E",
      dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestEncodedLength) {
  TestDataOutput dataOutput(nullptr);
  EXPECT_EQ(27, dataOutput.getEncodedLength("You had me at meat tornado!"));
}

TEST_F(DataOutputTest, TestEncodedLengthWide) {
  TestDataOutput dataOutput(nullptr);
  EXPECT_EQ(27, dataOutput.getEncodedLength(L"You had me at meat tornado."));
}

TEST_F(DataOutputTest, TestWriteObjectSharedPtr) {
  TestDataOutput dataOutput(nullptr);
  std::shared_ptr<CacheableString> objptr =
      CacheableString::create("You had me at meat tornado.");
  dataOutput.writeObject(objptr);
  EXPECT_BYTEARRAY_EQ(
      "57001B596F7520686164206D65206174206D65617420746F726E61646F2E",
      dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestWriteObjectCacheableString) {
  TestDataOutput dataOutput(nullptr);
  CacheableStringPtr objptr =
      CacheableString::create("You had me at meat tornado.");
  dataOutput.writeObject(objptr);
  EXPECT_BYTEARRAY_EQ(
      "57001B596F7520686164206D65206174206D65617420746F726E61646F2E",
      dataOutput.getByteArray());
}

TEST_F(DataOutputTest, TestCursorAdvance) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeUTF("You had me at meat tornado.");
  EXPECT_BYTEARRAY_EQ(
      "001B596F7520686164206D65206174206D65617420746F726E61646F2E",
      dataOutput.getByteArray());

  const uint32_t originalLength = dataOutput.getBufferLength();
  dataOutput.advanceCursor(2);
  EXPECT_EQ((originalLength + 2), dataOutput.getBufferLength())
      << "Correct length after advance";
}

TEST_F(DataOutputTest, TestCursorNegativeAdvance) {
  TestDataOutput dataOutput(nullptr);
  dataOutput.writeUTF("You had me at meat tornado.");
  EXPECT_BYTEARRAY_EQ(
      "001B596F7520686164206D65206174206D65617420746F726E61646F2E",
      dataOutput.getByteArray());

  const uint32_t originalLength = dataOutput.getBufferLength();
  dataOutput.advanceCursor(-2);
  EXPECT_EQ((originalLength - 2), dataOutput.getBufferLength())
      << "Correct length after negative advance";
}

}  // namespace