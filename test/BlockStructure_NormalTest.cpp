// db_imp_test.cpp

#include <cppunit/config/SourcePrefix.h>
#include <binary_stream/binary_istream.h>
#include <binary_stream/binary_ostream.h>
#include "BlockStructure_NormalTest.h"
#include <Util.h>
#include <Config.h>
#include <check_function.h>

#include "test_def.h"
#define private public
#define protected public
#include <data_file/BlockStructure.h>
#include <data_file/BlockStructure_Normal.h>
#undef private
#undef protected


// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( BlockStructure_NormalTest );

using namespace DuoDuo;
using namespace Essential;

void BlockStructure_NormalTest::setUp()
{
}


void BlockStructure_NormalTest::tearDown()
{
}


void BlockStructure_NormalTest::test_constructor()
{
}

void BlockStructure_NormalTest::test_create()
{
}

void BlockStructure_NormalTest::test_InitBlock()
{
    block_t block;
    block.resize(4096);
    BlockStructure_Normal normal(block);
    normal.InitBlock();
    CPPUNIT_ASSERT(block.size() == static_cast<size_t>(Config::Ins().ssd_block_size()));
    CPPUNIT_ASSERT(block[0] == '\0');
    CPPUNIT_ASSERT(block[1] == '\0');
    CPPUNIT_ASSERT(block[2] == BlockStructure::eBlockType_Normal);
    CPPUNIT_ASSERT(block[3] == '\0');
    CPPUNIT_ASSERT(block[4] == '\0');
}

void BlockStructure_NormalTest::test_KeySectionSize()
{
    BlockStructure_Normal::StructCalc normal_calc(4096);
    CPPUNIT_ASSERT(normal_calc.KeySectionSize() == 1276); // (4096 * 5 / 16) - 4
}

void BlockStructure_NormalTest::test_KeySection_BodySize()
{
    BlockStructure_Normal::StructCalc normal_calc(4096);
    CPPUNIT_ASSERT(normal_calc.KeySection_BodySize() == 1274);
}

void BlockStructure_NormalTest::test_DataSectionSize()
{
    BlockStructure_Normal::StructCalc normal_calc(4096);
    CPPUNIT_ASSERT(normal_calc.DataSectionSize() == 2816); // (4096 * 11 / 16)
}


void BlockStructure_NormalTest::test_DataSection_BodySize()
{
    BlockStructure_Normal::StructCalc normal_calc(4096);
    CPPUNIT_ASSERT(normal_calc.DataSection_BodySize() == 2814);
}

void BlockStructure_NormalTest::test_GetKeyNeedLen()
{
    CPPUNIT_ASSERT(BlockStructure_Normal::GetKeyNeedLen(std::string("1234567890")) == 15);
    CPPUNIT_ASSERT(BlockStructure_Normal::GetKeyNeedLen(std::string("1")) == 6);
    CPPUNIT_ASSERT_THROW(BlockStructure_Normal::GetKeyNeedLen(std::string("")), AssertException);
}

void BlockStructure_NormalTest::test_GetValueNeedLen()
{
    CPPUNIT_ASSERT(BlockStructure_Normal::GetValueNeedLen(std::string("1234567890")) == 13);
    CPPUNIT_ASSERT(BlockStructure_Normal::GetValueNeedLen(std::string("1")) == 4);
    CPPUNIT_ASSERT(BlockStructure_Normal::GetValueNeedLen(std::string("")) == 3);
}

void BlockStructure_NormalTest::test_MakeKeyBuffer()
{
    {
        Essential::_binary_buf buf = BlockStructure_Normal::MakeKeyBuffer("123", 12);
        Essential::_binary_istream<Essential::_binary_buf> istr(buf);
        CPPUNIT_ASSERT(istr.Unpack<uint16_t>() == 3);
        CPPUNIT_ASSERT(istr.Unpack<uint16_t>() == 12);
        CPPUNIT_ASSERT(istr.Unpack(3) == std::string("123"));
        CPPUNIT_ASSERT(istr.Unpack<char>() == char(0xF1));
    }

    {
        CPPUNIT_ASSERT_THROW(BlockStructure_Normal::MakeKeyBuffer("", 12), AssertException);
    }
}

void BlockStructure_NormalTest::test_MakeDeletedKeyBuffer()
{
    {
        Essential::_binary_buf buf = BlockStructure_Normal::MakeDeletedKeyBuffer("123");
        //printf("\nbuf=============: %s\n", buf.to_cstyle_str().c_str());
        Essential::_binary_istream<Essential::_binary_buf> istr(buf);
        CPPUNIT_ASSERT(istr.Unpack<uint16_t>() == 3);
        CPPUNIT_ASSERT(istr.Unpack<uint16_t>() == 0);
        CPPUNIT_ASSERT(istr.Unpack(3) == std::string("123"));
        CPPUNIT_ASSERT(istr.Unpack<char>() == char(0xF1));
    }

    {
        CPPUNIT_ASSERT_THROW(BlockStructure_Normal::MakeKeyBuffer("", 12), AssertException);
    }
}

void BlockStructure_NormalTest::test_IsEnoughForData()
{
    // test keySection
    {
        // normal
        {
            block_t block;
            block.resize(4096);
            BlockStructure_Normal normal(block);
            CPPUNIT_ASSERT(normal.IsEnoughForData("key23423", "value23423") == true);

            normal.AddDataToKeyValueMap("key12345", "value123456");
            CPPUNIT_ASSERT(normal.IsEnoughForData("key23423", "value23423") == true);
        }

        // key section sold out
        {
            block_t block;
            block.resize(64);
            BlockStructure_Normal normal(block);
            BlockStructure_Normal::StructCalc normal_calc(64);
            CPPUNIT_ASSERT(normal_calc.KeySection_BodySize() == 14); // (64 * 5 / 16) - 4 - 2
            CPPUNIT_ASSERT(normal_calc.DataSection_BodySize() == 42); // (64 * 11 / 16) - 2
            std::string key1 = "1k";
            std::string value1 = "1v34567890";
            size_t key1_needLen = BlockStructure_Normal::GetKeyNeedLen(key1);
            CPPUNIT_ASSERT(key1_needLen == 7);
            size_t value1_needLen = BlockStructure_Normal::GetValueNeedLen(value1);
            CPPUNIT_ASSERT(value1_needLen = 13);

            CPPUNIT_ASSERT(normal.IsEnoughForData(key1, value1) == true);
            normal.AddDataToKeyValueMap(key1, value1);

            // sold out
            {
                std::string key2 = "2k345";
                size_t key2_needLen = BlockStructure_Normal::GetKeyNeedLen(key2);
                CPPUNIT_ASSERT(key2_needLen == 10);
                std::string value2 = "2v34567890";
                CPPUNIT_ASSERT(normal.IsEnoughForData(key2, value2) == false);
            }

            // duplication in one normalBlock
            {
                CPPUNIT_ASSERT(normal.IsEnoughForData(key1, "new valuegasdfasdf") == true);
            }

            // in boundary
            {
                std::string key3 = "3k";
                size_t key3_needLen = BlockStructure_Normal::GetKeyNeedLen(key3);
                CPPUNIT_ASSERT(key3_needLen == 7);
                std::string value3 = "2v34567890";
                CPPUNIT_ASSERT(normal.IsEnoughForData(key3, value3) == true);
            }

            // out boundary
            {
                std::string key4 = "3k3";
                size_t key4_needLen = BlockStructure_Normal::GetKeyNeedLen(key4);
                CPPUNIT_ASSERT(key4_needLen == 8);
                std::string value4 = "2v34567890";
                CPPUNIT_ASSERT(normal.IsEnoughForData(key4, value4) == false);
            }
        }

        // key is empty
        {
            block_t block;
            block.resize(64);
            BlockStructure_Normal normal(block);

            CPPUNIT_ASSERT_THROW(normal.IsEnoughForData("", "fasdfasdfas"), AssertException);
        }

        // value is empty
        {
            block_t block;
            block.resize(64);
            BlockStructure_Normal normal(block);

            std::string key3 = "3k";
            size_t key3_needLen = BlockStructure_Normal::GetKeyNeedLen(key3);
            CPPUNIT_ASSERT(key3_needLen == 7);
            std::string value3 = "2v34567890";
            CPPUNIT_ASSERT(normal.IsEnoughForData(key3, "") == true);
        }

        // add data with delete identity

        // key too large for a empty normalBlock
        {
            block_t block;
            block.resize(64);
            BlockStructure_Normal normal(block);

            CPPUNIT_ASSERT(normal.IsEnoughForData("123456789", "gasdf") == true);
            CPPUNIT_ASSERT(normal.IsEnoughForData("1234567890", "fdgs") == false);
        }

    } // end of test keySection


    // test dataSection
    {
        // data section sold out
        {
            block_t block;
            block.resize(64);
            BlockStructure_Normal normal(block);
            BlockStructure_Normal::StructCalc normal_calc(64);
            CPPUNIT_ASSERT(normal_calc.KeySection_BodySize() == 14); // (64 * 5 / 16) - 4 - 2
            CPPUNIT_ASSERT(normal_calc.DataSection_BodySize() == 42); // (64 * 11 / 16) - 2
            std::string key1 = "1";
            std::string value1 = "1v34567890";
            size_t key1_needLen = BlockStructure_Normal::GetKeyNeedLen(key1);
            CPPUNIT_ASSERT(key1_needLen == 6);
            size_t value1_needLen = BlockStructure_Normal::GetValueNeedLen(value1);
            CPPUNIT_ASSERT(value1_needLen = 13);

            CPPUNIT_ASSERT(normal.IsEnoughForData(key1, value1) == true);
            normal.AddDataToKeyValueMap(key1, value1);
 
            // sold out
            {
                std::string key2 = "2";
                std::string value2 = "2v345678901234567890123456789012345";  // keyNeedLen = 38
                CPPUNIT_ASSERT(normal.IsEnoughForData(key2, value2) == false);
            }

            // duplication in one normalBlock
            {
                std::string key2 = "1";
                std::string value2 = "2v345678901234567890123456789012345";  // keyNeedLen = 38
                CPPUNIT_ASSERT(normal.IsEnoughForData(key2, value2) == true);
            }

            // in boundary
            {
                std::string key2 = "2";
                std::string value2 = "2v345678901234567890123456";  // keyNeedLen = 29
                CPPUNIT_ASSERT(normal.IsEnoughForData(key2, value2) == true);
            }

            // out boundary
            {
                std::string key2 = "2";
                std::string value2 = "2v3456789012345678901234567";  // keyNeedLen = 30
                CPPUNIT_ASSERT(normal.IsEnoughForData(key2, value2) == false);
            }
        }

        // value is empty
        {
            block_t block;
            block.resize(128);
            BlockStructure_Normal normal(block);
            BlockStructure_Normal::StructCalc normal_calc(128);
            CPPUNIT_ASSERT(normal_calc.KeySection_BodySize() == 34); // (128 * 5 / 16) - 4 - 2
            CPPUNIT_ASSERT(normal_calc.DataSection_BodySize() == 86); // (128 * 11 / 16) - 2

            std::string value1 = "1v34567890"; // value need len: 13
            normal.AddDataToKeyValueMap("1", value1);
            CPPUNIT_ASSERT(normal.IsEnoughForData("n", "") == true);

            std::string value2 = "123456789012345678901234567890"
                "123456789012345678901234567890"
                "1234567";  // value need len = 70
            CPPUNIT_ASSERT(normal.IsEnoughForData("n", value2) == true);
            normal.AddDataToKeyValueMap("2", value2);

            std::string value3 = ""; // value need len = 3
            CPPUNIT_ASSERT(normal.IsEnoughForData("n", "") == true);
            CPPUNIT_ASSERT(normal.IsEnoughForData("n", "1") == false);
        }

        // value too large for a empty normalBlock
        {
            block_t block;
            block.resize(64);
            BlockStructure_Normal normal(block);

            std::string key2 = "2";
            std::string value2 = "2v3456789012345678901234567890123456789";  // keyNeedLen = 42
            CPPUNIT_ASSERT(normal.IsEnoughForData(key2, value2) == true);
            std::string value3 = "2v34567890123456789012345678901234567890";  // keyNeedLen = 43
            CPPUNIT_ASSERT(normal.IsEnoughForData(key2, value3) == false);
        }

        // add data with delete identity


    } // end of test dataSection
}

void BlockStructure_NormalTest::test_GetKey()
{
    // normal
    {
        block_t block;
        Essential::_binary_ostream<Essential::_binary_buf> ostrm(block);
        ostrm.Pack<uint16_t>(10);
        ostrm.Pack<BlockStructure_Normal::data_pos_t>(234);
        ostrm.Pack("1234567890", 10);
        ostrm.Pack<char>(char(0xF1));
        ostrm.Pack("otherdata", sizeof("otherdata"));

        Essential::_binary_istream<Essential::_binary_buf> istrm(block);
        BlockStructure_Normal::data_pos_t dataPos = -1;
        const std::string& key = BlockStructure_Normal::GetKey(istrm, dataPos);
        CPPUNIT_ASSERT(key == std::string("1234567890"));
        CPPUNIT_ASSERT(dataPos == 234);
    }
}

void BlockStructure_NormalTest::test_GetValue()
{
    // normal
    {
        block_t block;
        Essential::_binary_ostream<Essential::_binary_buf> ostrm(block);
        ostrm.Pack<uint16_t>(10);
        ostrm.Pack("1234567890", 10);
        ostrm.Pack<char>(char(0xF2));

        Essential::_binary_istream<Essential::_binary_buf> istrm(block);
        const std::string& value = BlockStructure_Normal::GetValue(istrm);
        CPPUNIT_ASSERT(value == std::string("1234567890"));
    }

    // zero value
    {
        block_t block;
        Essential::_binary_ostream<Essential::_binary_buf> ostrm(block);
        ostrm.Pack<uint16_t>(0);
        ostrm.Pack<char>(char(0xF2));

        Essential::_binary_istream<Essential::_binary_buf> istrm(block);
        const std::string& value = BlockStructure_Normal::GetValue(istrm);
        CPPUNIT_ASSERT(value == std::string(""));
    }

    // wrong buff
    {
        block_t block;
        Essential::_binary_ostream<Essential::_binary_buf> ostrm(block);
        ostrm.Pack<uint16_t>(11);
        ostrm.Pack("1234567890", 10);
        ostrm.Pack<char>(char(0xF2));

        Essential::_binary_istream<Essential::_binary_buf> istrm(block);
        CPPUNIT_ASSERT_THROW(BlockStructure_Normal::GetValue(istrm), Essential::AssertException);
    }
}

void BlockStructure_NormalTest::test_MakeKeySectionBodyAndDataSectionBody()
{
    key_value_map_t keyValues;

    // zero key/value
    {
        block_t block;
        block.resize(128);
        BlockStructure_Normal normal(block);
        const BlockStructure_Normal::KeyDataBodyResult& result = normal.MakeKeySectionBodyAndDataSectionBody(keyValues);
        for (size_t i = 0; i < result.key_section_body.size(); ++i)
            CPPUNIT_ASSERT(result.key_section_body[i] == char(0));
        for (size_t i = 0; i < result.data_section_body.size(); ++i)
            CPPUNIT_ASSERT(result.data_section_body[i] == char(0));
    }


    // one key/value
    {
        block_t block;
        block.resize(128);
        BlockStructure_Normal normal(block);
        keyValues["1"] = "1234567890";
        const BlockStructure_Normal::KeyDataBodyResult& result = normal.MakeKeySectionBodyAndDataSectionBody(keyValues);

        Essential::_binary_istream<Essential::_binary_buf>key_stream(result.key_section_body);
        Essential::_binary_istream<Essential::_binary_buf>data_stream(result.data_section_body);
        BlockStructure_Normal::data_pos_t data_pos = -1;
        CPPUNIT_ASSERT(BlockStructure_Normal::GetKey(key_stream, data_pos) == std::string("1"));
        CPPUNIT_ASSERT(data_pos == normal.m_StructCalc.DataSection_BodyStartPos());
        CPPUNIT_ASSERT(BlockStructure_Normal::GetValue(data_stream) == std::string("1234567890"));
    }

    // more key/value
    {
        block_t block;
        block.resize(128);
        BlockStructure_Normal normal(block);
        BlockStructure_Normal::StructCalc normal_calc(128);
        CPPUNIT_ASSERT(normal_calc.KeySection_BodySize() == 34); // (128 * 5 / 16) - 4 - 2
        CPPUNIT_ASSERT(normal_calc.DataSection_BodySize() == 86); // (128 * 11 / 16) - 2
        keyValues["1"] = "1234567890";
        keyValues["2"] = "123456";
        const BlockStructure_Normal::KeyDataBodyResult& result = normal.MakeKeySectionBodyAndDataSectionBody(keyValues);

        Essential::_binary_istream<Essential::_binary_buf>key_stream(result.key_section_body);
        Essential::_binary_istream<Essential::_binary_buf>data_stream(result.data_section_body);
        BlockStructure_Normal::data_pos_t data_pos = -1;

        CPPUNIT_ASSERT(BlockStructure_Normal::GetKey(key_stream, data_pos) == std::string("1"));
        CPPUNIT_ASSERT(data_pos == normal.m_StructCalc.DataSection_BodyStartPos());
        CPPUNIT_ASSERT(BlockStructure_Normal::GetValue(data_stream) == std::string("1234567890"));

        CPPUNIT_ASSERT(BlockStructure_Normal::GetKey(key_stream, data_pos) == std::string("2"));
        size_t first_data_buff_len = 10 + 3;
        CPPUNIT_ASSERT(data_pos == normal.m_StructCalc.DataSection_BodyStartPos() + first_data_buff_len);
        CPPUNIT_ASSERT(BlockStructure_Normal::GetValue(data_stream) == std::string("123456"));
    }

}

void BlockStructure_NormalTest::test_PackAndLoad()
{
    {
        block_t block;
        block.resize(128);
        BlockStructure_Normal normal(block);
        normal.AddData("1", "12345678901234567890"); // key_body: 6, data_body: 23
        normal.AddData("2", "123456789012345678901234567890"); // key_body: 6, data_body: 33
        CPPUNIT_ASSERT_THROW(normal.AddData("123456789012345678", "2342"), Essential::AssertException);
        normal.AddData("12345678901234567", ""); // key_body: 22, data_body: 3
        CPPUNIT_ASSERT_THROW(normal.AddData("4", "2342"), Essential::AssertException);
        normal.AddData("1", "1234567890123456789"); // key_body: 6, data_body: 23

        normal.PackBlock();

        BlockStructure_Normal normal2(block);
        normal2.LoadFromBlock();
        CPPUNIT_ASSERT(normal2.m_KeyValues.size() == 3);
        CPPUNIT_ASSERT(normal2.m_KeyValues["1"] == std::string("1234567890123456789"));
        CPPUNIT_ASSERT(normal2.m_KeyValues["2"] == std::string("123456789012345678901234567890"));
        CPPUNIT_ASSERT(normal2.m_KeyValues["12345678901234567"] == std::string(""));
    }
}

void BlockStructure_NormalTest::test_AddData()
{
    block_t block;
    block.resize(128);
    BlockStructure_Normal normal(block);
    BlockStructure_Normal::StructCalc normal_calc(128);
    CPPUNIT_ASSERT(normal_calc.KeySection_BodySize() == 34); // (128 * 5 / 16) - 4 - 2
    CPPUNIT_ASSERT(normal_calc.DataSection_BodySize() == 86); // (128 * 11 / 16) - 2

    // normal
    {
        normal.AddData("1", "12345678901234567890"); // key_body: 6, data_body: 23
        normal.AddData("2", "123456789012345678901234567890"); // key_body: 6, data_body: 33

        // value out of range
        CPPUNIT_ASSERT_THROW(normal.AddData("v", "1234567890123456789012345678"), Essential::AssertException); //key_body: 6, data_body: 31

        normal.AddData("12345678901234567", "123456789012345678901234567"); // key_body: 22, data_body: 30

        // key out of range
        CPPUNIT_ASSERT_THROW(normal.AddData("4", "2342"), Essential::AssertException);

        BlockStructure_Normal normal2(block);
        normal2.LoadFromBlock();
        CPPUNIT_ASSERT(normal2.m_KeyValues.size() == 3);
        CPPUNIT_ASSERT(normal2.m_KeyValues["1"] == std::string("12345678901234567890"));
        CPPUNIT_ASSERT(normal2.m_KeyValues["2"] == std::string("123456789012345678901234567890"));
        CPPUNIT_ASSERT(normal2.m_KeyValues["12345678901234567"] == std::string("123456789012345678901234567"));

    }


    // change value of one key
    {
        normal.AddData("1", "123456789012345678"); // key_body: 6, data_body: 21
        normal.AddData("12345678901234567", ""); // key_body: 22, data_body: 3

        BlockStructure_Normal normal2(block);
        normal2.LoadFromBlock();
        CPPUNIT_ASSERT(normal2.m_KeyValues.size() == 3);
        CPPUNIT_ASSERT(normal2.m_KeyValues["1"] == std::string("123456789012345678"));
        CPPUNIT_ASSERT(normal2.m_KeyValues["2"] == std::string("123456789012345678901234567890"));
        CPPUNIT_ASSERT(normal2.m_KeyValues["12345678901234567"] == std::string(""));

        // out of range
    }


    // delete one key/value
    {
    }

}
