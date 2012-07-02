#include <algorithm>
#include <sstream>
#include <UnitTest++.h>
#include <tightdb/table_macros.hpp>

using namespace tightdb;

TEST(Table1)
{
    Table table;
    table.add_column(COLUMN_TYPE_INT, "first");
    table.add_column(COLUMN_TYPE_INT, "second");

    CHECK_EQUAL(COLUMN_TYPE_INT, table.get_column_type(0));
    CHECK_EQUAL(COLUMN_TYPE_INT, table.get_column_type(1));
    CHECK_EQUAL("first", table.get_column_name(0));
    CHECK_EQUAL("second", table.get_column_name(1));

    // Test adding a single empty row
    // and filling it with values
    size_t ndx = table.add_empty_row();
    table.set_int(0, ndx, 0);
    table.set_int(1, ndx, 10);

    CHECK_EQUAL(0, table.get_int(0, ndx));
    CHECK_EQUAL(10, table.get_int(1, ndx));

    // Test adding multiple rows
    ndx = table.add_empty_row(7);
    for (size_t i = ndx; i < 7; ++i) {
        table.set_int(0, i, 2*i);
        table.set_int(1, i, 20*i);
    }

    for (size_t i = ndx; i < 7; ++i) {
        const int64_t v1 = 2 * i;
        const int64_t v2 = 20 * i;
        CHECK_EQUAL(v1, table.get_int(0, i));
        CHECK_EQUAL(v2, table.get_int(1, i));
    }

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG
}

enum Days {
    Mon,
    Tue,
    Wed,
    Thu,
    Fri,
    Sat,
    Sun
};

TIGHTDB_TABLE_4(TestTable,
                first,  Int,
                second, Int,
                third,  Bool,
                fourth, Enum<Days>)

TEST(Table2)
{
    TestTable table;

    table.add(0, 10, true, Wed);
    const TestTable::Cursor r = table.back(); // last item

    CHECK_EQUAL(0, r.first);
    CHECK_EQUAL(10, r.second);
    CHECK_EQUAL(true, r.third);
    CHECK_EQUAL(Wed, r.fourth);

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG
}

TEST(Table3)
{
    TestTable table;

    for (size_t i = 0; i < 100; ++i) {
        table.add(0, 10, true, Wed);
    }

    // Test column searching
    CHECK_EQUAL(size_t(0),  table.cols().first.find_first(0));
    CHECK_EQUAL(size_t(-1), table.cols().first.find_first(1));
    CHECK_EQUAL(size_t(0),  table.cols().second.find_first(10));
    CHECK_EQUAL(size_t(-1), table.cols().second.find_first(100));
    CHECK_EQUAL(size_t(0),  table.cols().third.find_first(true));
    CHECK_EQUAL(size_t(-1), table.cols().third.find_first(false));
    CHECK_EQUAL(size_t(0) , table.cols().fourth.find_first(Wed));
    CHECK_EQUAL(size_t(-1), table.cols().fourth.find_first(Mon));

    // Test column incrementing
    table.cols().first += 3;
    CHECK_EQUAL(3, table[0].first);
    CHECK_EQUAL(3, table[99].first);

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG
}

TIGHTDB_TABLE_2(TestTableEnum,
                first,      Enum<Days>,
                second,     String)

TEST(Table4)
{
    TestTableEnum table;

    table.add(Mon, "Hello");
    table.add(Mon, "HelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHello");
    const TestTableEnum::Cursor r = table.back(); // last item

    CHECK_EQUAL(Mon, r.first);
    CHECK_EQUAL("HelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHello", (const char*)r.second);

    // Test string column searching
    CHECK_EQUAL(size_t(1),  table.cols().second.find_first("HelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHello"));
    CHECK_EQUAL(size_t(-1), table.cols().second.find_first("Foo"));

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG
}

TEST(Table_Delete)
{
    TestTable table;

    for (size_t i = 0; i < 10; ++i) {
        table.add(0, i, true, Wed);
    }

    table.remove(0);
    table.remove(4);
    table.remove(7);

    CHECK_EQUAL(1, table[0].second);
    CHECK_EQUAL(2, table[1].second);
    CHECK_EQUAL(3, table[2].second);
    CHECK_EQUAL(4, table[3].second);
    CHECK_EQUAL(6, table[4].second);
    CHECK_EQUAL(7, table[5].second);
    CHECK_EQUAL(8, table[6].second);

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG

    // Delete all items one at a time
    for (size_t i = 0; i < 7; ++i) {
        table.remove(0);
    }

    CHECK(table.is_empty());
    CHECK_EQUAL(0, table.size());

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG
}

TEST(Table_Delete_All_Types)
{
    // Create table with all column types
    Table table;
    Spec& s = table.get_spec();
    s.add_column(COLUMN_TYPE_INT,    "int");
    s.add_column(COLUMN_TYPE_BOOL,   "bool");
    s.add_column(COLUMN_TYPE_DATE,   "date");
    s.add_column(COLUMN_TYPE_STRING, "string");
    s.add_column(COLUMN_TYPE_STRING, "string_long");
    s.add_column(COLUMN_TYPE_STRING, "string_enum"); // becomes ColumnStringEnum
    s.add_column(COLUMN_TYPE_BINARY, "binary");
    s.add_column(COLUMN_TYPE_MIXED,  "mixed");
    Spec sub = s.add_subtable_column("tables");
    sub.add_column(COLUMN_TYPE_INT,    "sub_first");
    sub.add_column(COLUMN_TYPE_STRING, "sub_second");
    table.update_from_spec();

    // Add some rows
    for (size_t i = 0; i < 15; ++i) {
        table.insert_int(0, i, i);
        table.insert_bool(1, i, (i % 2 ? true : false));
        table.insert_date(2, i, 12345);

        std::stringstream ss;
        ss << "string" << i;
        table.insert_string(3, i, ss.str().c_str());

        ss << " very long string.........";
        table.insert_string(4, i, ss.str().c_str());

        switch (i % 3) {
            case 0:
                table.insert_string(5, i, "test1");
                break;
            case 1:
                table.insert_string(5, i, "test2");
                break;
            case 2:
                table.insert_string(5, i, "test3");
                break;
        }

        table.insert_binary(6, i, "binary", 7);

        switch (i % 4) {
            case 0:
                table.insert_mixed(7, i, false);
                break;
            case 1:
                table.insert_mixed(7, i, (int64_t)i);
                break;
            case 2:
                table.insert_mixed(7, i, "string");
                break;
            case 3:
            {
                // Add subtable to mixed column
                // We can first set schema and contents when the entire
                // row has been inserted
                table.insert_mixed(7, i, Mixed(COLUMN_TYPE_TABLE));
                break;
            }
        }

        table.insert_subtable(8, i);
        table.insert_done();

        // Add subtable to mixed column
        if (i % 4 == 3) {
            TableRef subtable = table.get_subtable(7, i);
            subtable->add_column(COLUMN_TYPE_INT,    "first");
            subtable->add_column(COLUMN_TYPE_STRING, "second");
            subtable->insert_int(0, 0, 42);
            subtable->insert_string(1, 0, "meaning");
            subtable->insert_done();
        }

        // Add sub-tables to table column
        TableRef subtable = table.get_subtable(8, i);
        subtable->insert_int(0, 0, 42);
        subtable->insert_string(1, 0, "meaning");
        subtable->insert_done();
    }

    // We also want a ColumnStringEnum
    table.optimize();

    // Test Deletes
    table.remove(14);
    table.remove(0);
    table.remove(5);

    CHECK_EQUAL(12, table.size());

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG

    // Test Clear
    table.clear();
    CHECK_EQUAL(0, table.size());

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG
}

TEST(Table_Find_Int)
{
    TestTable table;

    for (int i = 1000; i >= 0; --i) {
        table.add(0, i, true, Wed);
    }

    CHECK_EQUAL(size_t(0),    table.cols().second.find_first(1000));
    CHECK_EQUAL(size_t(1000), table.cols().second.find_first(0));
    CHECK_EQUAL(size_t(-1),   table.cols().second.find_first(1001));

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG
}


/*
TEST(Table6)
{
    TestTableEnum table;

    TDB_QUERY(TestQuery, TestTableEnum) {
    //  first.between(Mon, Thu);
        second == "Hello" || (second == "Hey" && first == Mon);
    }};

    TDB_QUERY_OPT(TestQuery2, TestTableEnum) (Days a, Days b, const char* str) {
        (void)b;
        (void)a;
        //first.between(a, b);
        second == str || second.MatchRegEx(".*");
    }};

    //TestTableEnum result = table.find_all(TestQuery2(Mon, Tue, "Hello")).sort().Limit(10);
    //size_t result2 = table.Range(10, 200).find_first(TestQuery());
    //CHECK_EQUAL((size_t)-1, result2);

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG
}
*/


TEST(Table_FindAll_Int)
{
    TestTable table;

    table.add(0, 10, true, Wed);
    table.add(0, 20, true, Wed);
    table.add(0, 10, true, Wed);
    table.add(0, 20, true, Wed);
    table.add(0, 10, true, Wed);
    table.add(0, 20, true, Wed);
    table.add(0, 10, true, Wed);
    table.add(0, 20, true, Wed);
    table.add(0, 10, true, Wed);
    table.add(0, 20, true, Wed);

    // Search for a value that does not exits
    const TestTable::View v0 = table.cols().second.find_all(5);
    CHECK_EQUAL(0, v0.size());

    // Search for a value with several matches
    const TestTable::View v = table.cols().second.find_all(20);

    CHECK_EQUAL(5, v.size());
    CHECK_EQUAL(1, v.get_source_ndx(0));
    CHECK_EQUAL(3, v.get_source_ndx(1));
    CHECK_EQUAL(5, v.get_source_ndx(2));
    CHECK_EQUAL(7, v.get_source_ndx(3));
    CHECK_EQUAL(9, v.get_source_ndx(4));

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG
}

TEST(Table_Sorted_Int)
{
    TestTable table;

    table.add(0, 10, true, Wed); // 0: 4
    table.add(0, 20, true, Wed); // 1: 7
    table.add(0,  0, true, Wed); // 2: 0
    table.add(0, 40, true, Wed); // 3: 8
    table.add(0, 15, true, Wed); // 4: 6
    table.add(0, 11, true, Wed); // 5: 5
    table.add(0,  6, true, Wed); // 6: 3
    table.add(0,  4, true, Wed); // 7: 2
    table.add(0, 99, true, Wed); // 8: 9
    table.add(0,  2, true, Wed); // 9: 1

    // Search for a value that does not exits
    TestTable::View v = table.cols().second.get_sorted_view();
    CHECK_EQUAL(table.size(), v.size());

    CHECK_EQUAL(2, v.get_source_ndx(0));
    CHECK_EQUAL(9, v.get_source_ndx(1));
    CHECK_EQUAL(7, v.get_source_ndx(2));
    CHECK_EQUAL(6, v.get_source_ndx(3));
    CHECK_EQUAL(0, v.get_source_ndx(4));
    CHECK_EQUAL(5, v.get_source_ndx(5));
    CHECK_EQUAL(4, v.get_source_ndx(6));
    CHECK_EQUAL(1, v.get_source_ndx(7));
    CHECK_EQUAL(3, v.get_source_ndx(8));
    CHECK_EQUAL(8, v.get_source_ndx(9));

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG
}


TEST(Table_Index_Int)
{
    TestTable table;

    table.add(0,  1, true, Wed);
    table.add(0, 15, true, Wed);
    table.add(0, 10, true, Wed);
    table.add(0, 20, true, Wed);
    table.add(0, 11, true, Wed);
    table.add(0, 45, true, Wed);
    table.add(0, 10, true, Wed);
    table.add(0,  0, true, Wed);
    table.add(0, 30, true, Wed);
    table.add(0,  9, true, Wed);

    // Create index for column two
    table.cols().second.set_index();

    // Search for a value that does not exits
    const size_t r1 = table.cols().second.find_first(2);
    CHECK_EQUAL(-1, r1);

    // Find existing values
    CHECK_EQUAL(0, table.cols().second.find_first(1));
    CHECK_EQUAL(1, table.cols().second.find_first(15));
    CHECK_EQUAL(2, table.cols().second.find_first(10));
    CHECK_EQUAL(3, table.cols().second.find_first(20));
    CHECK_EQUAL(4, table.cols().second.find_first(11));
    CHECK_EQUAL(5, table.cols().second.find_first(45));
    //CHECK_EQUAL(6, table.cols().second.find_first(10)); // only finds first match
    CHECK_EQUAL(7, table.cols().second.find_first(0));
    CHECK_EQUAL(8, table.cols().second.find_first(30));
    CHECK_EQUAL(9, table.cols().second.find_first(9));

    // Change some values
    table[2].second = 13;
    table[9].second = 100;

    CHECK_EQUAL(0, table.cols().second.find_first(1));
    CHECK_EQUAL(1, table.cols().second.find_first(15));
    CHECK_EQUAL(2, table.cols().second.find_first(13));
    CHECK_EQUAL(3, table.cols().second.find_first(20));
    CHECK_EQUAL(4, table.cols().second.find_first(11));
    CHECK_EQUAL(5, table.cols().second.find_first(45));
    CHECK_EQUAL(6, table.cols().second.find_first(10));
    CHECK_EQUAL(7, table.cols().second.find_first(0));
    CHECK_EQUAL(8, table.cols().second.find_first(30));
    CHECK_EQUAL(9, table.cols().second.find_first(100));

    // Insert values
    table.add(0, 29, true, Wed);
    //TODO: More than add

    CHECK_EQUAL(0, table.cols().second.find_first(1));
    CHECK_EQUAL(1, table.cols().second.find_first(15));
    CHECK_EQUAL(2, table.cols().second.find_first(13));
    CHECK_EQUAL(3, table.cols().second.find_first(20));
    CHECK_EQUAL(4, table.cols().second.find_first(11));
    CHECK_EQUAL(5, table.cols().second.find_first(45));
    CHECK_EQUAL(6, table.cols().second.find_first(10));
    CHECK_EQUAL(7, table.cols().second.find_first(0));
    CHECK_EQUAL(8, table.cols().second.find_first(30));
    CHECK_EQUAL(9, table.cols().second.find_first(100));
    CHECK_EQUAL(10, table.cols().second.find_first(29));

    // Delete some values
    table.remove(0);
    table.remove(5);
    table.remove(8);

    CHECK_EQUAL(0, table.cols().second.find_first(15));
    CHECK_EQUAL(1, table.cols().second.find_first(13));
    CHECK_EQUAL(2, table.cols().second.find_first(20));
    CHECK_EQUAL(3, table.cols().second.find_first(11));
    CHECK_EQUAL(4, table.cols().second.find_first(45));
    CHECK_EQUAL(5, table.cols().second.find_first(0));
    CHECK_EQUAL(6, table.cols().second.find_first(30));
    CHECK_EQUAL(7, table.cols().second.find_first(100));

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG
}

TIGHTDB_TABLE_4(TestTableAE,
                first,  Int,
                second, String,
                third,  Bool,
                fourth, Enum<Days>)

TEST(TableAutoEnumeration)
{
    TestTableAE table;

    for (size_t i = 0; i < 5; ++i) {
        table.add(1, "abd",     true, Mon);
        table.add(2, "eftg",    true, Tue);
        table.add(5, "hijkl",   true, Wed);
        table.add(8, "mnopqr",  true, Thu);
        table.add(9, "stuvxyz", true, Fri);
    }

    table.optimize();

    for (size_t i = 0; i < 5; ++i) {
        const size_t n = i * 5;
        CHECK_EQUAL(1, table[0+n].first);
        CHECK_EQUAL(2, table[1+n].first);
        CHECK_EQUAL(5, table[2+n].first);
        CHECK_EQUAL(8, table[3+n].first);
        CHECK_EQUAL(9, table[4+n].first);

        CHECK_EQUAL("abd",     (const char*)table[0+n].second);
        CHECK_EQUAL("eftg",    (const char*)table[1+n].second);
        CHECK_EQUAL("hijkl",   (const char*)table[2+n].second);
        CHECK_EQUAL("mnopqr",  (const char*)table[3+n].second);
        CHECK_EQUAL("stuvxyz", (const char*)table[4+n].second);

        CHECK_EQUAL(true, table[0+n].third);
        CHECK_EQUAL(true, table[1+n].third);
        CHECK_EQUAL(true, table[2+n].third);
        CHECK_EQUAL(true, table[3+n].third);
        CHECK_EQUAL(true, table[4+n].third);

        CHECK_EQUAL(Mon, table[0+n].fourth);
        CHECK_EQUAL(Tue, table[1+n].fourth);
        CHECK_EQUAL(Wed, table[2+n].fourth);
        CHECK_EQUAL(Thu, table[3+n].fourth);
        CHECK_EQUAL(Fri, table[4+n].fourth);
    }


}


TEST(TableAutoEnumerationFindFindAll)
{
    TestTableAE table;

    for (size_t i = 0; i < 5; ++i) {
        table.add(1, "abd",     true, Mon);
        table.add(2, "eftg",    true, Tue);
        table.add(5, "hijkl",   true, Wed);
        table.add(8, "mnopqr",  true, Thu);
        table.add(9, "stuvxyz", true, Fri);
    }

    table.optimize();

    size_t t = table.cols().second.find_first("eftg");
    CHECK_EQUAL(1, t);

    TestTableAE::View tv = table.cols().second.find_all("eftg");
    CHECK_EQUAL(5, tv.size());
    CHECK_EQUAL("eftg", static_cast<const char*>(tv[0].second));
    CHECK_EQUAL("eftg", static_cast<const char*>(tv[1].second));
    CHECK_EQUAL("eftg", static_cast<const char*>(tv[2].second));
    CHECK_EQUAL("eftg", static_cast<const char*>(tv[3].second));
    CHECK_EQUAL("eftg", static_cast<const char*>(tv[4].second));
}

#include <tightdb/alloc_slab.hpp>
TEST(Table_SlabAlloc)
{
    SlabAlloc alloc;
    TestTable table(alloc);

    table.add(0, 10, true, Wed);
    const TestTable::Cursor r = table.back(); // last item

    CHECK_EQUAL(   0, r.first);
    CHECK_EQUAL(  10, r.second);
    CHECK_EQUAL(true, r.third);
    CHECK_EQUAL( Wed, r.fourth);

    // Add some more rows
    table.add(1, 10, true, Wed);
    table.add(2, 20, true, Wed);
    table.add(3, 10, true, Wed);
    table.add(4, 20, true, Wed);
    table.add(5, 10, true, Wed);

    // Delete some rows
    table.remove(2);
    table.remove(4);

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG
}

#include <tightdb/group.hpp>
TEST(Table_Spec)
{
    Group group;
    TableRef table = group.get_table("test");

    // Create specification with sub-table
    Spec& s = table->get_spec();
    s.add_column(COLUMN_TYPE_INT,    "first");
    s.add_column(COLUMN_TYPE_STRING, "second");
    Spec sub = s.add_subtable_column("third");
        sub.add_column(COLUMN_TYPE_INT,    "sub_first");
        sub.add_column(COLUMN_TYPE_STRING, "sub_second");
    table->update_from_spec();

    CHECK_EQUAL(3, table->get_column_count());

    // Add a row
    table->insert_int(0, 0, 4);
    table->insert_string(1, 0, "Hello");
    table->insert_subtable(2, 0);
    table->insert_done();

    CHECK_EQUAL(0, table->get_subtable_size(2, 0));

    // Get the sub-table
    {
        TableRef subtable = table->get_subtable(2, 0);
        CHECK(subtable->is_empty());

        subtable->insert_int(0, 0, 42);
        subtable->insert_string(1, 0, "test");
        subtable->insert_done();

        CHECK_EQUAL(42,     subtable->get_int(0, 0));
        CHECK_EQUAL("test", subtable->get_string(1, 0));
    }

    CHECK_EQUAL(1, table->get_subtable_size(2, 0));

    // Get the sub-table again and see if the values
    // still match.
    {
        TableRef subtable = table->get_subtable(2, 0);

        CHECK_EQUAL(1,      subtable->size());
        CHECK_EQUAL(42,     subtable->get_int(0, 0));
        CHECK_EQUAL("test", subtable->get_string(1, 0));
    }

    // Write the group to disk
    group.write("subtables.tightdb");

    // Read back tables
    {
        Group fromDisk("subtables.tightdb", GROUP_READONLY);
        TableRef fromDiskTable = fromDisk.get_table("test");

        TableRef subtable2 = fromDiskTable->get_subtable(2, 0);

        CHECK_EQUAL(1,      subtable2->size());
        CHECK_EQUAL(42,     subtable2->get_int(0, 0));
        CHECK_EQUAL("test", subtable2->get_string(1, 0));
    }
}

TEST(Table_Mixed)
{
    Table table;
    table.add_column(COLUMN_TYPE_INT, "first");
    table.add_column(COLUMN_TYPE_MIXED, "second");

    CHECK_EQUAL(COLUMN_TYPE_INT, table.get_column_type(0));
    CHECK_EQUAL(COLUMN_TYPE_MIXED, table.get_column_type(1));
    CHECK_EQUAL("first", table.get_column_name(0));
    CHECK_EQUAL("second", table.get_column_name(1));

    const size_t ndx = table.add_empty_row();
    table.set_int(0, ndx, 0);
    table.set_mixed(1, ndx, true);

    CHECK_EQUAL(0, table.get_int(0, 0));
    CHECK_EQUAL(COLUMN_TYPE_BOOL, table.get_mixed(1, 0).get_type());
    CHECK_EQUAL(true, table.get_mixed(1, 0).get_bool());

    table.insert_int(0, 1, 43);
    table.insert_mixed(1, 1, (int64_t)12);
    table.insert_done();

    CHECK_EQUAL(0,  table.get_int(0, ndx));
    CHECK_EQUAL(43, table.get_int(0, 1));
    CHECK_EQUAL(COLUMN_TYPE_BOOL, table.get_mixed(1, 0).get_type());
    CHECK_EQUAL(COLUMN_TYPE_INT,  table.get_mixed(1, 1).get_type());
    CHECK_EQUAL(true, table.get_mixed(1, 0).get_bool());
    CHECK_EQUAL(12,   table.get_mixed(1, 1).get_int());

    table.insert_int(0, 2, 100);
    table.insert_mixed(1, 2, "test");
    table.insert_done();

    CHECK_EQUAL(0,  table.get_int(0, 0));
    CHECK_EQUAL(43, table.get_int(0, 1));
    CHECK_EQUAL(COLUMN_TYPE_BOOL,   table.get_mixed(1, 0).get_type());
    CHECK_EQUAL(COLUMN_TYPE_INT,    table.get_mixed(1, 1).get_type());
    CHECK_EQUAL(COLUMN_TYPE_STRING, table.get_mixed(1, 2).get_type());
    CHECK_EQUAL(true,   table.get_mixed(1, 0).get_bool());
    CHECK_EQUAL(12,     table.get_mixed(1, 1).get_int());
    CHECK_EQUAL("test", table.get_mixed(1, 2).get_string());

    table.insert_int(0, 3, 0);
    table.insert_mixed(1, 3, Date(324234));
    table.insert_done();

    CHECK_EQUAL(0,  table.get_int(0, 0));
    CHECK_EQUAL(43, table.get_int(0, 1));
    CHECK_EQUAL(0,  table.get_int(0, 3));
    CHECK_EQUAL(COLUMN_TYPE_BOOL,   table.get_mixed(1, 0).get_type());
    CHECK_EQUAL(COLUMN_TYPE_INT,    table.get_mixed(1, 1).get_type());
    CHECK_EQUAL(COLUMN_TYPE_STRING, table.get_mixed(1, 2).get_type());
    CHECK_EQUAL(COLUMN_TYPE_DATE,   table.get_mixed(1, 3).get_type());
    CHECK_EQUAL(true,   table.get_mixed(1, 0).get_bool());
    CHECK_EQUAL(12,     table.get_mixed(1, 1).get_int());
    CHECK_EQUAL("test", table.get_mixed(1, 2).get_string());
    CHECK_EQUAL(324234, table.get_mixed(1, 3).get_date());

    table.insert_int(0, 4, 43);
    table.insert_mixed(1, 4, Mixed("binary", 7));
    table.insert_done();

    CHECK_EQUAL(0,  table.get_int(0, 0));
    CHECK_EQUAL(43, table.get_int(0, 1));
    CHECK_EQUAL(0,  table.get_int(0, 3));
    CHECK_EQUAL(43, table.get_int(0, 4));
    CHECK_EQUAL(COLUMN_TYPE_BOOL,   table.get_mixed(1, 0).get_type());
    CHECK_EQUAL(COLUMN_TYPE_INT,    table.get_mixed(1, 1).get_type());
    CHECK_EQUAL(COLUMN_TYPE_STRING, table.get_mixed(1, 2).get_type());
    CHECK_EQUAL(COLUMN_TYPE_DATE,   table.get_mixed(1, 3).get_type());
    CHECK_EQUAL(COLUMN_TYPE_BINARY, table.get_mixed(1, 4).get_type());
    CHECK_EQUAL(true,   table.get_mixed(1, 0).get_bool());
    CHECK_EQUAL(12,     table.get_mixed(1, 1).get_int());
    CHECK_EQUAL("test", table.get_mixed(1, 2).get_string());
    CHECK_EQUAL(324234, table.get_mixed(1, 3).get_date());
    CHECK_EQUAL("binary", (const char*)table.get_mixed(1, 4).get_binary().pointer);
    CHECK_EQUAL(7,      table.get_mixed(1, 4).get_binary().len);

    table.insert_int(0, 5, 0);
    table.insert_mixed(1, 5, Mixed(COLUMN_TYPE_TABLE));
    table.insert_done();

    CHECK_EQUAL(0,  table.get_int(0, 0));
    CHECK_EQUAL(43, table.get_int(0, 1));
    CHECK_EQUAL(0,  table.get_int(0, 3));
    CHECK_EQUAL(43, table.get_int(0, 4));
    CHECK_EQUAL(0,  table.get_int(0, 5));
    CHECK_EQUAL(COLUMN_TYPE_BOOL,   table.get_mixed(1, 0).get_type());
    CHECK_EQUAL(COLUMN_TYPE_INT,    table.get_mixed(1, 1).get_type());
    CHECK_EQUAL(COLUMN_TYPE_STRING, table.get_mixed(1, 2).get_type());
    CHECK_EQUAL(COLUMN_TYPE_DATE,   table.get_mixed(1, 3).get_type());
    CHECK_EQUAL(COLUMN_TYPE_BINARY, table.get_mixed(1, 4).get_type());
    CHECK_EQUAL(COLUMN_TYPE_TABLE,  table.get_mixed(1, 5).get_type());
    CHECK_EQUAL(true,   table.get_mixed(1, 0).get_bool());
    CHECK_EQUAL(12,     table.get_mixed(1, 1).get_int());
    CHECK_EQUAL("test", table.get_mixed(1, 2).get_string());
    CHECK_EQUAL(324234, table.get_mixed(1, 3).get_date());
    CHECK_EQUAL("binary", (const char*)table.get_mixed(1, 4).get_binary().pointer);
    CHECK_EQUAL(7,      table.get_mixed(1, 4).get_binary().len);

    // Get table from mixed column and add schema and some values
    TableRef subtable = table.get_subtable(1, 5);
    subtable->add_column(COLUMN_TYPE_STRING, "name");
    subtable->add_column(COLUMN_TYPE_INT,    "age");

    subtable->insert_string(0, 0, "John");
    subtable->insert_int(1, 0, 40);
    subtable->insert_done();

    // Get same table again and verify values
    TableRef subtable2 = table.get_subtable(1, 5);
    CHECK_EQUAL(1, subtable2->size());
    CHECK_EQUAL("John", subtable2->get_string(0, 0));
    CHECK_EQUAL(40, subtable2->get_int(1, 0));

#ifdef _DEBUG
    table.Verify();
#endif //_DEBUG
}

TIGHTDB_TABLE_1(TestTableMX,
                first, Mixed)


TEST(Table_Mixed2)
{
    TestTableMX table;

    table.add(int64_t(1));
    table.add(true);
    table.add(Date(1234));
    table.add("test");

    CHECK_EQUAL(COLUMN_TYPE_INT,    table[0].first.get_type());
    CHECK_EQUAL(COLUMN_TYPE_BOOL,   table[1].first.get_type());
    CHECK_EQUAL(COLUMN_TYPE_DATE,   table[2].first.get_type());
    CHECK_EQUAL(COLUMN_TYPE_STRING, table[3].first.get_type());

    CHECK_EQUAL(1,            table[0].first.get_int());
    CHECK_EQUAL(true,         table[1].first.get_bool());
    CHECK_EQUAL(time_t(1234), table[2].first.get_date());
    CHECK_EQUAL("test",       table[3].first.get_string());
}


TEST(Table_SubtableSizeAndClear)
{
    Table table;
    Spec& spec = table.get_spec();
    {
        Spec subspec = spec.add_subtable_column("subtab");
        subspec.add_column(COLUMN_TYPE_INT, "int");
    }
    spec.add_column(COLUMN_TYPE_MIXED, "mixed");
    table.update_from_spec();

    table.insert_subtable(0, 0);
    table.insert_mixed(1, 0, false);
    table.insert_done();

    table.insert_subtable(0, 1);
    table.insert_mixed(1, 1, Mixed(COLUMN_TYPE_TABLE));
    table.insert_done();

    CHECK_EQUAL(table.get_subtable_size(0,0), 0); // Subtable column
    CHECK_EQUAL(table.get_subtable_size(1,0), 0); // Mixed column, bool value
    CHECK_EQUAL(table.get_subtable_size(1,1), 0); // Mixed column, table value

    CHECK(table.get_subtable(0, 0));  // Subtable column
    CHECK(!table.get_subtable(1, 0)); // Mixed column, bool value, must return NULL
    CHECK(table.get_subtable(1, 1));  // Mixed column, table value

    table.set_mixed(1, 0, Mixed(COLUMN_TYPE_TABLE));
    table.set_mixed(1, 1, false);
    CHECK(table.get_subtable(1, 0));
    CHECK(!table.get_subtable(1, 1));

    TableRef subtab1 = table.get_subtable(0, 0);
    TableRef subtab2 = table.get_subtable(1, 0);
    {
        Spec& subspec = subtab2->get_spec();
        subspec.add_column(COLUMN_TYPE_INT, "int");
        subtab2->update_from_spec();
    }

    CHECK_EQUAL(table.get_subtable_size(1, 0), 0);
    CHECK(table.get_subtable(1, 0));

    subtab1->insert_int(0, 0, 0);
    subtab1->insert_done();

    subtab2->insert_int(0, 0, 0);
    subtab2->insert_done();

    CHECK_EQUAL(table.get_subtable_size(0,0), 1);
    CHECK_EQUAL(table.get_subtable_size(1,0), 1);

    table.clear_subtable(0, 0);
    table.clear_subtable(1, 0);

    CHECK_EQUAL(table.get_subtable_size(0,0), 0);
    CHECK_EQUAL(table.get_subtable_size(1,0), 0);

    CHECK(table.get_subtable(1, 0));
}


namespace
{
    TIGHTDB_TABLE_2(MyTable1,
                    val, Int,
                    val2, Int)

    TIGHTDB_TABLE_2(MyTable2,
                    val, Int,
                    subtab, Subtable<MyTable1>)

    TIGHTDB_TABLE_1(MyTable3,
                    subtab, Subtable<MyTable2>)
}


TEST(Table_SetMethod)
{
    MyTable1 t;
    t.add(8, 9);
    CHECK_EQUAL(t[0].val,  8);
    CHECK_EQUAL(t[0].val2, 9);
    t.set(0, 2, 4);
    CHECK_EQUAL(t[0].val,  2);
    CHECK_EQUAL(t[0].val2, 4);
}


TEST(Table_HighLevelSubtables)
{
    MyTable3 t;
    {
        MyTable3::Ref r1 = t.get_table_ref();
        MyTable3::ConstRef r2 = t.get_table_ref();
        MyTable3::ConstRef r3 = r2->get_table_ref();
        r3 = t.get_table_ref(); // Also test assigment that converts to const
        static_cast<void>(r1);
        static_cast<void>(r3);
    }

    t.add();
    const MyTable3& ct = t;
    {
        MyTable2::Ref       s1 = t[0].subtab;
        MyTable2::ConstRef  s2 = t[0].subtab;
        MyTable2::Ref       s3 = t[0].subtab->get_table_ref();
        MyTable2::ConstRef  s4 = t[0].subtab->get_table_ref();
        MyTable2::Ref       s5 = t.cols().subtab[0];
        MyTable2::ConstRef  s6 = t.cols().subtab[0];
        MyTable2::Ref       s7 = t.cols().subtab[0]->get_table_ref();
        MyTable2::ConstRef  s8 = t.cols().subtab[0]->get_table_ref();
        MyTable2::ConstRef cs1 = ct[0].subtab;
        MyTable2::ConstRef cs2 = ct[0].subtab->get_table_ref();
        MyTable2::ConstRef cs3 = ct.cols().subtab[0];
        MyTable2::ConstRef cs4 = ct.cols().subtab[0]->get_table_ref();
        s1 = t[0].subtab;
        s2 = t[0].subtab; // Also test assigment that converts to const
        static_cast<void>(s1);
        static_cast<void>(s2);
        static_cast<void>(s3);
        static_cast<void>(s4);
        static_cast<void>(s5);
        static_cast<void>(s6);
        static_cast<void>(s7);
        static_cast<void>(s8);
        static_cast<void>(cs1);
        static_cast<void>(cs2);
        static_cast<void>(cs3);
        static_cast<void>(cs4);
    }

    t[0].subtab->add();
    {
        MyTable1::Ref       s1 = t[0].subtab[0].subtab;
        MyTable1::ConstRef  s2 = t[0].subtab[0].subtab;
        MyTable1::Ref       s3 = t[0].subtab[0].subtab->get_table_ref();
        MyTable1::ConstRef  s4 = t[0].subtab[0].subtab->get_table_ref();
        MyTable1::Ref       s5 = t.cols().subtab[0]->cols().subtab[0];
        MyTable1::ConstRef  s6 = t.cols().subtab[0]->cols().subtab[0];
        MyTable1::Ref       s7 = t.cols().subtab[0]->cols().subtab[0]->get_table_ref();
        MyTable1::ConstRef  s8 = t.cols().subtab[0]->cols().subtab[0]->get_table_ref();
        MyTable1::ConstRef cs1 = ct[0].subtab[0].subtab;
        MyTable1::ConstRef cs2 = ct[0].subtab[0].subtab->get_table_ref();
        MyTable1::ConstRef cs3 = ct.cols().subtab[0]->cols().subtab[0];
        MyTable1::ConstRef cs4 = ct.cols().subtab[0]->cols().subtab[0]->get_table_ref();
        s1 = t[0].subtab[0].subtab;
        s2 = t[0].subtab[0].subtab; // Also test assigment that converts to const
        static_cast<void>(s1);
        static_cast<void>(s2);
        static_cast<void>(s3);
        static_cast<void>(s4);
        static_cast<void>(s5);
        static_cast<void>(s6);
        static_cast<void>(s7);
        static_cast<void>(s8);
        static_cast<void>(cs1);
        static_cast<void>(cs2);
        static_cast<void>(cs3);
        static_cast<void>(cs4);
    }

    t[0].subtab[0].val = 1;
    CHECK_EQUAL(t[0].subtab[0].val,                 1);
    CHECK_EQUAL(t.cols().subtab[0]->cols().val[0],  1);
    CHECK_EQUAL(t[0].subtab->cols().val[0],         1);
    CHECK_EQUAL(t.cols().subtab[0][0].val,          1);

    t.cols().subtab[0]->cols().val[0] = 2;
    CHECK_EQUAL(t[0].subtab[0].val,                 2);
    CHECK_EQUAL(t.cols().subtab[0]->cols().val[0],  2);
    CHECK_EQUAL(t[0].subtab->cols().val[0],         2);
    CHECK_EQUAL(t.cols().subtab[0][0].val,          2);

    t[0].subtab->cols().val[0] = 3;
    CHECK_EQUAL(t[0].subtab[0].val,                 3);
    CHECK_EQUAL(t.cols().subtab[0]->cols().val[0],  3);
    CHECK_EQUAL(t[0].subtab->cols().val[0],         3);
    CHECK_EQUAL(t.cols().subtab[0][0].val,          3);

    t.cols().subtab[0][0].val = 4;
    CHECK_EQUAL(t[0].subtab[0].val,                 4);
    CHECK_EQUAL(t.cols().subtab[0]->cols().val[0],  4);
    CHECK_EQUAL(t[0].subtab->cols().val[0],         4);
    CHECK_EQUAL(t.cols().subtab[0][0].val,          4);
    CHECK_EQUAL(ct[0].subtab[0].val,                4);
    CHECK_EQUAL(ct.cols().subtab[0]->cols().val[0], 4);
    CHECK_EQUAL(ct[0].subtab->cols().val[0],        4);
    CHECK_EQUAL(ct.cols().subtab[0][0].val,         4);

    t[0].subtab[0].subtab->add();
    t[0].subtab[0].subtab[0].val = 5;
    CHECK_EQUAL(t[0].subtab[0].subtab[0].val,                         5);
    CHECK_EQUAL(t.cols().subtab[0]->cols().subtab[0]->cols().val[0],  5);
    CHECK_EQUAL(ct[0].subtab[0].subtab[0].val,                        5);
    CHECK_EQUAL(ct.cols().subtab[0]->cols().subtab[0]->cols().val[0], 5);

    t.cols().subtab[0]->cols().subtab[0]->cols().val[0] = 6;
    CHECK_EQUAL(t[0].subtab[0].subtab[0].val,                         6);
    CHECK_EQUAL(t.cols().subtab[0]->cols().subtab[0]->cols().val[0],  6);
    CHECK_EQUAL(ct[0].subtab[0].subtab[0].val,                        6);
    CHECK_EQUAL(ct.cols().subtab[0]->cols().subtab[0]->cols().val[0], 6);

/*
  Idea for compile time failure tests:

    const MyTable2 t;
#if    TEST_INDEX == 0
    t[0].val = 7;
#elsif TEST_INDEX == 1
    t.cols().val[0] = 7;
#elsif TEST_INDEX == 2
    t[0].subtab[0].val = 7;
#elsif TEST_INDEX == 3
    t[0].subtab->cols().val[0] = 7;
#endif
*/
}


namespace
{
    TIGHTDB_TABLE_2(TableDateAndBinary,
                    date, Date,
                    bin, Binary)
}

TEST(Table_DateAndBinary)
{
    TableDateAndBinary t;

    const size_t size = 10;
    char data[size];
    for (size_t i=0; i<size; ++i) data[i] = (char)i;
    t.add(8, BinaryData(data, size));
    CHECK_EQUAL(t[0].date, 8);
    CHECK_EQUAL(t[0].bin.get_len(), size);
    CHECK(std::equal(t[0].bin.get_pointer(), t[0].bin.get_pointer()+size, data));
}

// Test for a specific bug found: Calling clear on a group with a table with a subtable
TEST(Table_Test_Clear_With_Subtable_AND_Group)
{
    Group group;
    TableRef table = group.get_table("test");

    // Create specification with sub-table
    Spec& s = table->get_spec();
    s.add_column(COLUMN_TYPE_STRING, "name");
    Spec sub = s.add_subtable_column("sub");
        sub.add_column(COLUMN_TYPE_INT, "num");
    table->update_from_spec();

    CHECK_EQUAL(2, table->get_column_count());

    // Add a row
    table->insert_string(0, 0, "Foo");
    table->insert_subtable(1, 0);
    table->insert_done();

    CHECK_EQUAL(0, table->get_subtable_size(1, 0));

    // Get the sub-table
    {
        TableRef subtable = table->get_subtable(1, 0);
        CHECK(subtable->is_empty());

        subtable->insert_int(0, 0, 123);
        subtable->insert_done();

        CHECK_EQUAL(123, subtable->get_int(0, 0));
    }

    CHECK_EQUAL(1, table->get_subtable_size(1, 0));

    table->clear();
}


/*
TEST(Table_SubtableWithParentChange)
{
FIXME: Two problems:
- When new rows are inserted in parent table before a subtable, then the 'index in parent' property of the top level array of the subtable is not adjusted as it should be. Also, the indices, that are used as keys in the subtable instance cache of a subtable column, are not updated as they should be.
- When a subtable ref exists and the row that contains the subtable is removed - ouch!

    MyTable3 table;
    table.add();
    table.add();
    MyTable2::Ref subtab = table[1].subtab;
    subtab->add(7, 0);
    cout << "N:1:" << subtab->size() << endl;
    cout << "IIP:1:" << subtab->_get_index_in_parent() << endl;
//    CHECK_EQUAL(subtab, MyTable2::Ref(table[1].subtab));
//    CHECK_EQUAL(table[1].subtab[0].val, 7);
//    CHECK_EQUAL(subtab[0].val, 7);
#ifdef _DEBUG
    table.Verify();
    subtab->Verify();
#endif // _DEBUG
    table.insert(0, 0);
    cout << "N:2:" << subtab->size() << endl;
    cout << "IIP:2:" << subtab->_get_index_in_parent() << endl;
//    CHECK_EQUAL(subtab, MyTable2::Ref(table[2].subtab));
//    CHECK_EQUAL(table[2].subtab[0].val, 7);
//    CHECK_EQUAL(subtab[0].val, 7);
#ifdef _DEBUG
    table.Verify();
    subtab->Verify();
#endif // _DEBUG
    table.remove(1);
    cout << "N:3:" << subtab->size() << endl;
    cout << "IIP:3:" << subtab->_get_index_in_parent() << endl;
//    CHECK_EQUAL(table[1].subtab[0].val, 7);
//    CHECK_EQUAL(subtab[0].val, 7);
#ifdef _DEBUG
//    table.Verify();
//    subtab->Verify();
#endif // _DEBUG
    table.remove(1);
    cout << "N:4:" << subtab->size() << endl;
    cout << "IIP:4:" << subtab->_get_index_in_parent() << endl;
//    CHECK_EQUAL(subtab[0].val, 7);
#ifdef _DEBUG
//    table.Verify();
//    subtab->Verify();
#endif // _DEBUG
}
*/
