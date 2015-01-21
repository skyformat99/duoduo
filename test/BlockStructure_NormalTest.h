// DataFileTest.h
#ifndef _BLOCK_STRUCTURE_TEST_H_
#define _BLOCK_STRUCTURE_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

class BlockStructure_NormalTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( BlockStructure_NormalTest );
  CPPUNIT_TEST( test_constructor );
  CPPUNIT_TEST( test_create );
  CPPUNIT_TEST( test_InitBlock );
  CPPUNIT_TEST( test_KeySectionSize );
  CPPUNIT_TEST( test_DataSectionSize );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void test_constructor();
  void test_create();
  void test_InitBlock();
  void test_KeySectionSize();
  void test_DataSectionSize();
};

#endif  // _BLOCK_STRUCTURE_TEST_H_
