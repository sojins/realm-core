/*************************************************************************
 *
 * Copyright 2016 Realm Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **************************************************************************/

#include "testsettings.hpp"
#ifdef TEST_JSON

#include <algorithm>
#include <limits>
#include <string>
#include <fstream>
#include <ostream>

#include <realm.hpp>
#include <external/json/json.hpp>

#include "util/misc.hpp"

#include "test.hpp"

using namespace realm;
using namespace realm::util;
using namespace realm::test_util;


// Test independence and thread-safety
// -----------------------------------
//
// All tests must be thread safe and independent of each other. This
// is required because it allows for both shuffling of the execution
// order and for parallelized testing.
//
// In particular, avoid using std::rand() since it is not guaranteed
// to be thread safe. Instead use the API offered in
// `test/util/random.hpp`.
//
// All files created in tests must use the TEST_PATH macro (or one of
// its friends) to obtain a suitable file system path. See
// `test/util/test_path.hpp`.
//
//
// Debugging and the ONLY() macro
// ------------------------------
//
// A simple way of disabling all tests except one called `Foo`, is to
// replace TEST(Foo) with ONLY(Foo) and then recompile and rerun the
// test suite. Note that you can also use filtering by setting the
// environment varible `UNITTEST_FILTER`. See `README.md` for more on
// this.
//
// Another way to debug a particular test, is to copy that test into
// `experiments/testcase.cpp` and then run `sh build.sh
// check-testcase` (or one of its friends) from the command line.

namespace {

const bool generate_all = false;

// After modifying json methods in core, set above generate_all = true to
// make the unit tests output their results to files. Then inspect the
// files manually to see if the json is correct.
//
// Finally set generate_all = false and commit them to github which will
// make all successive runs compare their produced json with these files
//
// All produced json is automatically checked for syntax regardless of
// the setting of generate_all. This is done using the 'nlohmann::json' parser.

void setup_multi_table(Table& table, size_t rows)
{
    // Create table with all column types
    table.add_column(type_Int, "int");                                     //  0
    table.add_column(type_Bool, "bool");                                   //  1
    table.add_column(type_Timestamp, "date");                              //  2
    table.add_column(type_Float, "float");                                 //  3
    table.add_column(type_Double, "double");                               //  4
    table.add_column(type_String, "string");                               //  5
    table.add_column(type_String, "string_long");                          //  6
    ColKey col_string_big = table.add_column(type_String, "string_big_blobs"); //  7
    ColKey col_string_enum = table.add_column(type_String, "string_enum");     //  8 - becomes StringEnumColumn
    ColKey col_binary = table.add_column(type_Binary, "binary");               //  9
    ColKey col_oid = table.add_column(type_ObjectId, "oid");                   //  10
    ColKey col_decimal = table.add_column(type_Decimal, "decimal");            //  11
    ColKey col_int_list = table.add_column_list(type_Int, "integers");         //  12
    ColKey col_string_list = table.add_column_list(type_String, "strings");    //  13
    ColKey col_dict = table.add_column_dictionary(type_Int, "dictionary");     //  14
    ColKey col_set = table.add_column_set(type_Int, "set");                    //  15
    ColKey col_uuid = table.add_column(type_UUID, "uuid");                     //  16


    std::vector<std::string> strings;
    for (size_t i = 0; i < rows; ++i) {
        std::stringstream out;
        out << "string" << i;
        strings.push_back(out.str());
    }
    for (size_t i = 0; i < rows; ++i) {
        int64_t sign = (i % 2 == 0) ? 1 : -1;
        std::string long_str = strings[i] + " very long string.........";
        auto obj = table.create_object().set_all(int64_t(i * sign), (i % 2 ? true : false), Timestamp{12345, 0},
                                                 123.456f * sign, 9876.54321 * sign, strings[i], long_str);
        switch (i % 2) {
            case 0: {
                std::string s = strings[i];
                s += " very long string.........";
                for (int j = 0; j != 4; ++j)
                    s += " big blobs big blobs big blobs"; // +30
                obj.set(col_string_big, s);
                break;
            }
            case 1:
                obj.set(col_string_big, "");
                break;
        }
        switch (i % 3) {
            case 0:
                obj.set(col_string_enum, "enum1");
                break;
            case 1:
                obj.set(col_string_enum, "enum2");
                break;
            case 2:
                obj.set(col_string_enum, "enum3");
                break;
        }
        obj.set(col_binary, BinaryData("binary", 7));
        obj.set(col_oid, ObjectId());
        obj.set(col_decimal, Decimal128("1.2345"));
        obj.set(col_uuid, UUID());
        auto int_list = obj.get_list<Int>(col_int_list);
        auto str_list = obj.get_list<String>(col_string_list);
        for (size_t n = 0; n < i % 5; n++) {
            int64_t val = -123 + i * n * 1234 * sign;
            std::string str = "sub_" + util::to_string(val);
            int_list.add(val);
            str_list.add(str);
        }

        auto dict = obj.get_dictionary(col_dict);
        dict.insert("a", 2);

        auto set = obj.get_set<Int>(col_set);
        set.insert(123);
    }

    // We also want a StringEnumColumn
    table.enumerate_string_column(col_string_enum);
}

bool json_test(std::string json, std::string expected_file, bool generate)
{
    // std::cout << json << std::endl;
    std::string file_name = get_test_resource_path();
    file_name += expected_file + ".json";

    auto j = nlohmann::json::parse(json);

    if (generate) {
        // Generate the testdata to compare. After doing this,
        // verify that the output is correct with a json validator:
        // http://jsonformatter.curiousconcept.com/
        std::ofstream test_file(file_name.c_str(), std::ios::out | std::ios::binary);
        test_file << j;
        std::cerr << "\n----------------------------------------\n";
        std::cerr << "Generated " << expected_file << ":\n";
        std::cerr << json << "\n----------------------------------------\n";
        return true;
    }
    else {
        nlohmann::json expected;
        std::ifstream test_file(file_name.c_str(), std::ios::in | std::ios::binary);

        // fixme, find a way to use CHECK from a function
        if (!test_file.good())
            return false;
        if (test_file.fail())
            return false;
        test_file >> expected;
        if (j != expected) {
            std::cout << "Current: " << json << std::endl;
            std::cout << std::endl;
            std::cout << "Excpected " << expected << std::endl;
            std::cout << std::endl;
            std::string file_name = get_test_resource_path();
            std::string path = file_name + "bad_" + expected_file + ".json";
            std::string pathOld = "bad_" + file_name;
            File out(path, File::mode_Write);
            out.write(json);
            std::cerr << "\n error result in '" << std::string(path) << "'\n";
            return false;
        }
        return true;
    }
}


std::map<std::string, std::string> no_renames;

TEST(Json_NoLinks)
{
    Table table;
    setup_multi_table(table, 15);

    std::stringstream ss;
    table.to_json(ss, 0, no_renames);
    CHECK(json_test(ss.str(), "expect_json", generate_all));
    return;
}

TEST(Xjson_NoLinks)
{
    Table table;
    setup_multi_table(table, 15);

    std::stringstream ss;
    table.to_json(ss, 0, no_renames, output_mode_xjson);

    CHECK(json_test(ss.str(), "expect_xjson", generate_all));
    return;
}

TEST(Xjson_Plus_NoLinks)
{
    Table table;
    setup_multi_table(table, 15);

    std::stringstream ss;
    table.to_json(ss, 0, no_renames, output_mode_xjson_plus);

    CHECK(json_test(ss.str(), "expect_xjson_plus", generate_all));
    return;
}

/*
For tables with links, the link_depth argument in to_json() means following:

link_depth = -1:
    Follow links to infinite depth, but only follow each link exactly once. Not suitable if cycles exist because they
    make it complex to find out what link is being followed for a table that has multiple outgoing links

link_depth >= 0:
    Follow all possible permitations of link paths that are at most link_depth links deep. A link can be taken any
    number if times.

A link which isn't followed (bottom of link_depth has been met, or link has already been followed with
    link_depth = -1) is printed as a simple sequence of integers of row indexes in the link column.
*/
TEST(Json_LinkList1)
{
    // Basic non-cyclic LinkList test that also tests column and table renaming
    Group group;

    TableRef table1 = group.add_table("table1");
    TableRef table2 = group.add_table("table2");
    TableRef table3 = group.add_table("table3");

    // add some more columns to table1 and table2
    table1->add_column(type_Int, "col1");
    table1->add_column(type_String, "str1");

    table2->add_column(type_Int, "col1");
    table2->add_column(type_String, "str2");

    table3->add_column(type_Int, "col1");
    table3->add_column(type_String, "str2");

    // add some rows
    auto obj0 = table1->create_object().set_all(100, "foo");
    auto obj1 = table1->create_object().set_all(200, "!");
    table1->create_object().set_all(300, "bar");

    table2->create_object().set_all(400, "hello");
    auto k21 = table2->create_object().set_all(500, "world").get_key();
    auto k22 = table2->create_object().set_all(600, "!").get_key();

    auto k30 = table3->create_object().set_all(700, "baz").get_key();
    table3->create_object().set_all(800, "test");
    auto k32 = table3->create_object().set_all(900, "hi").get_key();

    ColKey col_link2 = table1->add_column_list(*table2, "linkA");
    ColKey col_link3 = table1->add_column_list(*table3, "linkB");

    // set some links
    auto ll0 = obj0.get_linklist(col_link2); // Links to table 2
    ll0.add(k21);

    auto ll1 = obj1.get_linklist(col_link2); // Links to table 2
    ll1.add(k21);
    ll1.add(k22);

    auto ll2 = obj0.get_linklist(col_link3); // Links to table 3
    ll2.add(k30);
    ll2.add(k32);

    std::stringstream ss;

    // Now try different link_depth arguments
    table1->to_json(ss, 0, no_renames);
    CHECK(json_test(ss.str(), "expected_json_linklist1_1", generate_all));

    ss.str("");
    table1->to_json(ss, -1, no_renames);
    CHECK(json_test(ss.str(), "expected_json_linklist1_2", generate_all));

    ss.str("");
    table1->to_json(ss, 0, no_renames);
    CHECK(json_test(ss.str(), "expected_json_linklist1_3", generate_all));

    ss.str("");
    table1->to_json(ss, 1, no_renames);
    CHECK(json_test(ss.str(), "expected_json_linklist1_4", generate_all));

    ss.str("");
    table1->to_json(ss, 2, no_renames);
    CHECK(json_test(ss.str(), "expected_json_linklist1_5", generate_all));

    // Column and table renaming
    std::map<std::string, std::string> m;
    m["str1"] = "STR1";
    m["linkA"] = "LINKA";
    m["table1"] = "TABLE1";
    ss.str("");
    table1->to_json(ss, 2, m);
    CHECK(json_test(ss.str(), "expected_json_linklist1_6", generate_all));
}

TEST(Json_LinkListCycle)
{
    // Cycle in LinkList
    Group group;

    TableRef table1 = group.add_table("table1");
    TableRef table2 = group.add_table("table2");

    table1->add_column(type_String, "str1");
    table2->add_column(type_String, "str2");

    // add some rows
    auto t10 = table1->create_object().set_all("hello");
    auto t11 = table1->create_object().set_all("world");

    table2->create_object().set_all("bar");
    auto t20 = table2->create_object().set_all("foo");

    auto col_link1 = table1->add_column_list(*table2, "linkA");
    auto col_link2 = table2->add_column_list(*table1, "linkB");

    // set some links
    auto links1 = t10.get_linklist(col_link1);
    links1.add(t20.get_key());

    auto links2 = t20.get_linklist(col_link2);
    links2.add(t10.get_key());
    links2.add(t11.get_key());

    // create json

    std::stringstream ss;

    // Now try different link_depth arguments
    table1->to_json(ss, 0, no_renames);
    CHECK(json_test(ss.str(), "expected_json_linklist_cycle1", generate_all));

    ss.str("");
    table1->to_json(ss, -1, no_renames);
    CHECK(json_test(ss.str(), "expected_json_linklist_cycle2", generate_all));

    ss.str("");
    table1->to_json(ss, 1, no_renames);
    CHECK(json_test(ss.str(), "expected_json_linklist_cycle4", generate_all));

    ss.str("");
    table1->to_json(ss, 2, no_renames);
    CHECK(json_test(ss.str(), "expected_json_linklist_cycle5", generate_all));

    ss.str("");
    table1->to_json(ss, 3, no_renames);
    CHECK(json_test(ss.str(), "expected_json_linklist_cycle6", generate_all));
}

TEST(Json_LinkListLong)
{
    Group group;

    TableRef foos = group.add_table("foo");
    TableRef bars = group.add_table("bar");

    auto col_str = foos->add_column(type_String, "str1");
    auto col_sub = foos->add_column(*foos, "sub");
    auto col_link = bars->add_column(*foos, "link");

    // add some rows
    ObjKey prev;
    for (int i = 0; i < 10; i++) {
        std::string str = "String_" + util::to_string(i);
        prev = foos->create_object().set(col_str, str).set(col_sub, prev).get_key();
    }
    bars->create_object().set(col_link, prev);

    // create json

    std::stringstream ss;

    // Now try different link_depth arguments
    bars->to_json(ss, 0, no_renames);
    CHECK(json_test(ss.str(), "expected_json_linklist_long1", generate_all));

    ss.str("");
    bars->to_json(ss, -1, no_renames);
    CHECK(json_test(ss.str(), "expected_json_linklist_long2", generate_all));

    ss.str("");
    bars->to_json(ss, 5, no_renames);
    CHECK(json_test(ss.str(), "expected_json_linklist_long3", generate_all));
}

TEST(Json_LinkCycles)
{
    // Cycle in Link
    Group group;

    TableRef table1 = group.add_table("table1");
    TableRef table2 = group.add_table("table2");

    table1->add_column(type_String, "str1");
    table2->add_column(type_String, "str2");

    // add some rows
    auto t10 = table1->create_object().set_all("hello");
    table1->create_object().set_all("world");

    auto t20 = table2->create_object().set_all("foo");

    ColKey col_link1 = table1->add_column(*table2, "linkA");
    ColKey col_link2 = table2->add_column(*table1, "linkB");

    // set some links
    table1->begin()->set(col_link1, t20.get_key());
    table2->begin()->set(col_link2, t10.get_key());

    std::stringstream ss;

    // Now try different link_depth arguments
    table1->to_json(ss, 0, no_renames);
    CHECK(json_test(ss.str(), "expected_json_link_cycles1", generate_all));

    ss.str("");
    table1->to_json(ss, -1, no_renames);
    CHECK(json_test(ss.str(), "expected_json_link_cycles2", generate_all));

    ss.str("");
    table1->to_json(ss, 0, no_renames);
    CHECK(json_test(ss.str(), "expected_json_link_cycles3", generate_all));

    ss.str("");
    table1->to_json(ss, 1, no_renames);
    CHECK(json_test(ss.str(), "expected_json_link_cycles4", generate_all));

    ss.str("");
    table1->to_json(ss, 2, no_renames);
    CHECK(json_test(ss.str(), "expected_json_link_cycles5", generate_all));

    // Redo but from a TableView instead of the Table.
    auto tv = table1->where().find_all();
    // Now try different link_depth arguments
    ss.str("");
    tv.to_json(ss);
    CHECK(json_test(ss.str(), "expected_json_link_cycles1", generate_all));

    ss.str("");
    tv.to_json(ss, -1);
    CHECK(json_test(ss.str(), "expected_json_link_cycles2", generate_all));

    ss.str("");
    tv.to_json(ss, 0);
    CHECK(json_test(ss.str(), "expected_json_link_cycles3", generate_all));

    ss.str("");
    tv.to_json(ss, 1);
    CHECK(json_test(ss.str(), "expected_json_link_cycles4", generate_all));

    ss.str("");
    tv.to_json(ss, 2);
    CHECK(json_test(ss.str(), "expected_json_link_cycles5", generate_all));
}

TEST(Xjson_LinkList1)
{
    // Basic non-cyclic LinkList test that also tests column and table renaming
    Group group;

    TableRef table1 = group.add_table_with_primary_key("table1", type_String, "primaryKey");
    TableRef table2 = group.add_table_with_primary_key("table2", type_String, "primaryKey");

    // add some more columns to table1 and table2
    ColKey table1Coll = table1->add_column(type_Int, "int1");
    ColKey table2Coll = table2->add_column(type_Int, "int2");

    // add some rows
    auto obj0 = table1->create_object_with_primary_key("t1o1").set(table1Coll, 100);
    table1->create_object_with_primary_key("t1o3").set(table1Coll, 300);
    auto obj1 = table1->create_object_with_primary_key("t1o2").set(table1Coll, 200);


    auto k20 = table2->create_object_with_primary_key("t2o1").set(table2Coll, 400).get_key();
    auto k21 = table2->create_object_with_primary_key("t2o2").set(table2Coll, 500).get_key();
    auto k22 = table2->create_object_with_primary_key("t2o3").set(table2Coll, 600).get_key();

    ColKey col_link2 = table1->add_column_list(*table2, "linkA");
    ColKey col_link3 = table1->add_column(*table2, "linkB");

    // set some links
    obj0.set(col_link3, k20);
    auto ll0 = obj0.get_linklist(col_link2); // Links to table 2
    ll0.add(k21);

    auto ll1 = obj1.get_linklist(col_link2); // Links to table 2
    ll1.add(k21);
    ll1.add(k22);

    std::stringstream ss;

    // Now try different link_depth arguments
    table1->to_json(ss, 0, no_renames, output_mode_xjson);
    CHECK(json_test(ss.str(), "expected_xjson_linklist1", generate_all));

    ss.str("");
    table1->to_json(ss, 0, no_renames, output_mode_xjson_plus);
    CHECK(json_test(ss.str(), "expected_xjson_plus_linklist1", generate_all));

    // Column and table renaming
    std::map<std::string, std::string> m;
    m["str1"] = "STR1";
    m["linkA"] = "LINKA";
    m["table1"] = "TABLE1";
    ss.str("");
    table1->to_json(ss, 2, m, output_mode_xjson);
    CHECK(json_test(ss.str(), "expected_xjson_linklist2", generate_all));

    ss.str("");
    table1->to_json(ss, 2, m, output_mode_xjson_plus);
    CHECK(json_test(ss.str(), "expected_xjson_plus_linklist2", generate_all));
}

TEST(Xjson_NestedJsonTest)
{
    Group group;

    TableRef table1 = group.add_table_with_primary_key("table1", type_String, "primaryKey");
    TableRef table2 = group.add_table_with_primary_key("table2", type_String, "primaryKey");
    TableRef table3 = group.add_table_with_primary_key("table3", type_String, "primaryKey");
    TableRef table4 = group.add_table_with_primary_key("table4", type_String, "primaryKey");
    TableRef table5 = group.add_table_with_primary_key("table5", type_String, "primaryKey");

    table1->add_column(type_Int, "list_list_int", false, {{CollectionType::List, CollectionType::List}});
    table2->add_column(type_Int, "list_list_int2", false, {{CollectionType::List, CollectionType::List}});
    table3->add_column(type_Int, "dict_list_int", false, {{CollectionType::Dictionary, CollectionType::List}});
    table4->add_column(type_Int, "list_dict_int", false, {{CollectionType::List, CollectionType::Dictionary}});
    table5->add_column(type_Int, "dict_dict_int", false, {{CollectionType::Dictionary, CollectionType::Dictionary}});

    // add some rows to test basic nested collections
    auto obj1 = table1->create_object_with_primary_key("t1o1");
    auto obj2 = table2->create_object_with_primary_key("t2o1");
    auto obj3 = table3->create_object_with_primary_key("t3o1");
    auto obj4 = table4->create_object_with_primary_key("t4o1");
    auto obj5 = table5->create_object_with_primary_key("t5o1");
    //[[1,2],[3,4],[5,6],[7,8]]
    auto list1 = obj1.get_list_ptr<Int>({"list_list_int", 0});
    list1->add(1);
    list1->add(2);
    auto list2 = obj1.get_list_ptr<Int>({"list_list_int", 1});
    list2->add(3);
    list2->add(4);
    auto list3 = obj1.get_list_ptr<Int>({"list_list_int", 2});
    list3->add(5);
    list3->add(6);
    auto list4 = obj1.get_list_ptr<Int>({"list_list_int", 3});
    list4->add(7);
    list4->add(8);

    //[[1],[2]]
    list1 = obj2.get_list_ptr<Int>({"list_list_int2", 0});
    list1->add(1);
    list2 = obj2.get_list_ptr<Int>({"list_list_int2", 1});
    list2->add(2);

    //{"Foo":[1,2,3], "Foo1":[4,5], "Foo2":[6,7,8]}
    list1 = obj3.get_list_ptr<Int>({"dict_list_int", "Foo"});
    list1->add(1);
    list1->add(2);
    list1->add(3);
    list1 = obj3.get_list_ptr<Int>({"dict_list_int", "Foo1"});
    list1->add(4);
    list1->add(5);
    list1 = obj3.get_list_ptr<Int>({"dict_list_int", "Foo2"});
    list1->add(6);
    list1->add(7);
    list1->add(8);

    //[{"Key1":10,"Key2":10,"Key3":10}, {"Key1":20,"Key2":20,"Key3":20}]
    auto dict4_1 = obj4.get_dictionary_ptr({"list_dict_int", 0});
    auto dict4_2 = obj4.get_dictionary_ptr({"list_dict_int", 1});
    dict4_1->insert("Key1", 10);
    dict4_1->insert("Key2", 10);
    dict4_1->insert("Key3", 10);
    dict4_2->insert("Key1", 20);
    dict4_2->insert("Key2", 20);
    dict4_2->insert("Key3", 20);

    //{"Foo":{"Key1":10,"Key2":10,"Key3":10}, "Foo1":{"Key1":20,"Key2":20,"Key3":20}}
    auto dict5_1 = obj5.get_dictionary_ptr({"dict_dict_int", "Foo"});
    auto dict5_2 = obj5.get_dictionary_ptr({"dict_dict_int", "Foo1"});
    dict5_1->insert("Key1", 10);
    dict5_1->insert("Key2", 10);
    dict5_1->insert("Key3", 10);
    dict5_2->insert("Key1", 20);
    dict5_2->insert("Key2", 20);
    dict5_2->insert("Key3", 20);

    std::stringstream ss;

    table1->to_json(ss, 0, no_renames, output_mode_xjson);
    CHECK(json_test(ss.str(), "expected_xjson_nested_linklist1", generate_all));

    ss.str("");
    table2->to_json(ss, 0, no_renames, output_mode_xjson);
    CHECK(json_test(ss.str(), "expected_xjson_nested_linklist2", generate_all));

    ss.str("");
    table3->to_json(ss, 0, no_renames, output_mode_json);
    CHECK(json_test(ss.str(), "expected_xjson_nested_dictionary1", generate_all));

    ss.str("");
    table4->to_json(ss, 0, no_renames, output_mode_json);
    CHECK(json_test(ss.str(), "expected_xjson_nested_dictionary2", generate_all));

    ss.str("");
    table5->to_json(ss, 0, no_renames, output_mode_json);
    CHECK(json_test(ss.str(), "expected_xjson_nested_dictionary3", generate_all));

    // test links

    // List<Dictionary<Link>>
    table1->add_column(*table2, "obj_list_dict", {CollectionType::List, CollectionType::Dictionary});
    table1->create_object_with_primary_key("to1_list_dict_link");
    obj1.get_dictionary_ptr({"obj_list_dict", 0})->insert("Link_Key", obj2.get_key());

    // List<List<Link>>
    table1->add_column(*table2, "obj_list_list", {CollectionType::List, CollectionType::List});
    table1->create_object_with_primary_key("to1_list_list_link");
    auto link_list = obj1.get_collection_ptr(Path{"obj_list_list", 0});
    dynamic_cast<LnkLst*>(link_list.get())->add(obj3.get_key());

    // Dictionary<List<Link>>
    table1->add_column(*table2, "obj_dict_list", {CollectionType::Dictionary, CollectionType::List});
    table1->create_object_with_primary_key("to1_dict_list_link");
    link_list = obj1.get_collection_ptr(Path{"obj_dict_list", "Link_Key"});
    dynamic_cast<LnkLst*>(link_list.get())->add(obj4.get_key());

    // Dictionary<Dictionary<Link>>
    table1->add_column(*table2, "obj_dict_dict", {CollectionType::Dictionary, CollectionType::Dictionary});
    table1->create_object_with_primary_key("to1_dict_dict_link");
    obj1.get_dictionary_ptr({"obj_dict_dict", "Link_Key"})->insert("Link_Key_Nested", obj5.get_key());

    ss.str("");
    table1->to_json(ss, 0, no_renames, output_mode_xjson_plus);
    CHECK(json_test(ss.str(), "expected_xjson_nested_links", generate_all));
}

TEST(Xjson_LinkSet1)
{
    // Basic non-cyclic LinkList test that also tests column and table renaming
    Group group;

    TableRef table1 = group.add_table_with_primary_key("table1", type_String, "primaryKey");
    TableRef table2 = group.add_table_with_primary_key("table2", type_String, "primaryKey");

    // add some more columns to table1 and table2
    ColKey table1Coll = table1->add_column(type_Int, "int1");
    ColKey table2Coll = table2->add_column(type_Int, "int2");

    // add some rows
    auto obj0 = table1->create_object_with_primary_key("t1o1").set(table1Coll, 100);
    table1->create_object_with_primary_key("t1o3").set(table1Coll, 300);
    auto obj1 = table1->create_object_with_primary_key("t1o2").set(table1Coll, 200);


    table2->create_object_with_primary_key("t2o1").set(table2Coll, 400);
    auto k21 = table2->create_object_with_primary_key("t2o2").set(table2Coll, 500).get_key();
    auto k22 = table2->create_object_with_primary_key("t2o3").set(table2Coll, 600).get_key();

    ColKey col_link2 = table1->add_column_set(*table2, "linkA");

    // set some links
    auto ll0 = obj0.get_linkset(col_link2); // Links to table 2
    ll0.insert(k21);

    auto ll1 = obj1.get_linkset(col_link2); // Links to table 2
    ll1.insert(k21);
    ll1.insert(k22);

    std::stringstream ss;

    // Now try different link_depth arguments
    table1->to_json(ss, 0, no_renames, output_mode_xjson);
    CHECK(json_test(ss.str(), "expected_xjson_linkset1", generate_all));

    ss.str("");
    table1->to_json(ss, 0, no_renames, output_mode_xjson_plus);
    CHECK(json_test(ss.str(), "expected_xjson_plus_linkset1", generate_all));

    // Column and table renaming
    std::map<std::string, std::string> m;
    m["str1"] = "STR1";
    m["linkA"] = "LINKA";
    m["table1"] = "TABLE1";
    ss.str("");
    table1->to_json(ss, 2, m, output_mode_xjson);
    CHECK(json_test(ss.str(), "expected_xjson_linkset2", generate_all));

    ss.str("");
    table1->to_json(ss, 2, m, output_mode_xjson_plus);
    CHECK(json_test(ss.str(), "expected_xjson_plus_linkset2", generate_all));
}

TEST(Xjson_LinkDictionary1)
{
    // Basic non-cyclic LinkList test that also tests column and table renaming
    Group group;

    TableRef table1 = group.add_table_with_primary_key("table1", type_String, "primaryKey");
    TableRef table2 = group.add_table_with_primary_key("table2", type_String, "primaryKey");

    // add some more columns to table1 and table2
    ColKey table1Coll = table1->add_column(type_Int, "int1");
    ColKey table2Coll = table2->add_column(type_Int, "int2");

    // add some rows
    auto obj0 = table1->create_object_with_primary_key("t1o1").set(table1Coll, 100);
    table1->create_object_with_primary_key("t1o3").set(table1Coll, 300);
    auto obj1 = table1->create_object_with_primary_key("t1o2").set(table1Coll, 200);


    table2->create_object_with_primary_key("t2o1").set(table2Coll, 400);
    auto k21 = table2->create_object_with_primary_key("t2o2").set(table2Coll, 500).get_key();
    auto k22 = table2->create_object_with_primary_key("t2o3").set(table2Coll, 600).get_key();
    auto k_unres = table2->get_objkey_from_primary_key("t2o4");

    ColKey col_link2 = table1->add_column_dictionary(*table2, "linkA");

    // set some links
    auto ll0 = obj0.get_dictionary(col_link2); // Links to table 2
    ll0.insert("key1", k21);

    auto ll1 = obj1.get_dictionary(col_link2); // Links to table 2
    ll1.insert("key2", k21);
    ll1.insert("key3", k22);
    ll1.insert("key4", k_unres);
    ll1.insert("key5", Mixed{});

    std::stringstream ss;

    // Now try different link_depth arguments
    table1->to_json(ss, 0, no_renames);
    CHECK(json_test(ss.str(), "expected_json_linkdict1", generate_all));

    ss.str("");
    table1->to_json(ss, 0, no_renames, output_mode_xjson);
    CHECK(json_test(ss.str(), "expected_xjson_linkdict1", generate_all));

    ss.str("");
    table1->to_json(ss, 0, no_renames, output_mode_xjson_plus);
    CHECK(json_test(ss.str(), "expected_xjson_plus_linkdict1", generate_all));

    // Column and table renaming
    std::map<std::string, std::string> m;
    m["str1"] = "STR1";
    m["linkA"] = "LINKA";
    m["table1"] = "TABLE1";
    ss.str("");
    table1->to_json(ss, 2, m);
    CHECK(json_test(ss.str(), "expected_json_linkdict2", generate_all));

    ss.str("");
    table1->to_json(ss, 2, m, output_mode_xjson);
    CHECK(json_test(ss.str(), "expected_xjson_linkdict2", generate_all));

    ss.str("");
    table1->to_json(ss, 2, m, output_mode_xjson_plus);
    CHECK(json_test(ss.str(), "expected_xjson_plus_linkdict2", generate_all));
}

TEST(Xjson_DictionaryEmbeddedObject1)
{
    // Basic EmbeddedDictionary test
    Group group;

    TableRef table1 = group.add_table_with_primary_key("table1", type_String, "primaryKey");
    TableRef table2 = group.add_table("table2", Table::Type::Embedded);

    // add some columns to table1 and table2
    ColKey table1Coll = table1->add_column(type_Int, "int1");
    ColKey col_obj = table1->add_column(*table2, "embedded");
    ColKey col_dict = table1->add_column_dictionary(*table2, "linkA");
    table2->add_column(type_Int, "int2");

    // add some rows
    auto obj0 = table1->create_object_with_primary_key("t1o1").set(table1Coll, 100);
    auto obj2 = table1->create_object_with_primary_key("t1o3").set(table1Coll, 300);
    auto obj1 = table1->create_object_with_primary_key("t1o2").set(table1Coll, 200);

    auto dict1 = obj0.get_dictionary(col_dict);
    dict1.create_and_insert_linked_object("key1").set("int2", 111);

    auto dict2 = obj1.get_dictionary(col_dict);
    dict2.create_and_insert_linked_object("key2").set("int2", 222);
    dict2.create_and_insert_linked_object("key3").set("int2", 333);

    obj2.create_and_set_linked_object(col_obj).set("int2", 123);

    std::stringstream ss;
    table1->to_json(ss, 0, no_renames, output_mode_json);
    CHECK(json_test(ss.str(), "expected_json_embeddeddict1", generate_all));

    ss.str("");
    table1->to_json(ss, 0, no_renames, output_mode_xjson);
    CHECK(json_test(ss.str(), "expected_xjson_embeddeddict1", generate_all));

    ss.str("");
    table1->to_json(ss, 0, no_renames, output_mode_xjson_plus);
    CHECK(json_test(ss.str(), "expected_xjson_plus_embeddeddict1", generate_all));
}

TEST(Xjson_Mixed)
{
    Group group;

    TableRef foos = group.add_table_with_primary_key("Foo", type_Int, "_id");
    TableRef bars = group.add_table_with_primary_key("Bar", type_Int, "_id");

    // add some more columns to table1 and table2
    ColKey col_any = foos->add_column(type_Mixed, "any");
    ColKey col_any_list = foos->add_column_list(type_Mixed, "any_list");
    ColKey col_any_dict = foos->add_column_dictionary(type_Mixed, "any_dict");
    ColKey col_int = bars->add_column(type_Int, "int");

    // add some rows
    for (int64_t i = 0; i < 10; i++) {
        bars->create_object_with_primary_key(i).set(col_int, 100 * i);
    }
    auto obj = foos->create_object_with_primary_key(999);

    auto it = bars->begin();
    obj.set(col_any, Mixed(it->get_link()));

    auto list = obj.get_list<Mixed>(col_any_list);
    list.add(Mixed((++it)->get_link()));
    list.add(Mixed((++it)->get_link()));

    auto dict = obj.get_dictionary(col_any_dict);
    dict.insert("key1", Mixed((++it)->get_link()));
    dict.insert("key2", Mixed((++it)->get_link()));

    std::stringstream ss;

    foos->to_json(ss, 0, no_renames);
    CHECK(json_test(ss.str(), "expected_json_mixed1", generate_all));

    ss.str("");
    foos->to_json(ss, realm::npos, no_renames);
    CHECK(json_test(ss.str(), "expected_json_mixed2", generate_all));

    ss.str("");
    foos->to_json(ss, 0, no_renames, output_mode_xjson);
    CHECK(json_test(ss.str(), "expected_xjson_mixed1", generate_all));

    ss.str("");
    foos->to_json(ss, 0, no_renames, output_mode_xjson_plus);
    CHECK(json_test(ss.str(), "expected_xjson_plus_mixed1", generate_all));
}

TEST(Xjson_LinkCycles)
{
    // Cycle in Link
    Group group;

    TableRef table1 = group.add_table_with_primary_key("table1", type_String, "primaryKey");
    TableRef table2 = group.add_table_with_primary_key("table2", type_String, "primaryKey");

    ColKey table1Coll = table1->add_column(type_String, "str1");
    ColKey table2Coll = table2->add_column(type_String, "str2");

    // add some rows
    auto t10 = table1->create_object_with_primary_key("t1o1").set(table1Coll, "hello");
    table1->create_object_with_primary_key("t1o2").set(table1Coll, "world");

    auto t20 = table2->create_object_with_primary_key("t2o1").set(table2Coll, "foo");

    ColKey col_link1 = table1->add_column(*table2, "linkA");
    ColKey col_link2 = table2->add_column(*table1, "linkB");

    // set some links
    table1->begin()->set(col_link1, t20.get_key());
    table2->begin()->set(col_link2, t10.get_key());

    std::stringstream ss;

    // Now try different link_depth arguments
    table1->to_json(ss, 0, no_renames, output_mode_xjson);
    CHECK(json_test(ss.str(), "expected_xjson_link", generate_all));

    ss.str("");
    table1->to_json(ss, 0, no_renames, output_mode_xjson_plus);
    CHECK(json_test(ss.str(), "expected_xjson_plus_link", generate_all));
}

TEST(Json_Nulls)
{
    Group group;

    TableRef table1 = group.add_table("table1");

    constexpr bool is_nullable = true;
    ColKey str_col_ndx = table1->add_column(type_String, "str_col", is_nullable);
    ColKey bool_col_ndx = table1->add_column(type_Bool, "bool_col", is_nullable);
    ColKey int_col_ndx = table1->add_column(type_Int, "int_col", is_nullable);
    ColKey timestamp_col_ndx = table1->add_column(type_Timestamp, "timestamp_col", is_nullable);

    // add one row, populated manually
    auto obj = table1->create_object();
    obj.set(str_col_ndx, "Hello");
    obj.set(bool_col_ndx, false);
    obj.set(int_col_ndx, 1);
    obj.set(timestamp_col_ndx, Timestamp{1, 1});
    // add one row with default null values
    table1->create_object();

    std::stringstream ss;
    table1->to_json(ss, 0, no_renames);
    CHECK(json_test(ss.str(), "expected_json_nulls", generate_all));
}

TEST(Json_Schema)
{
    Group group;

    TableRef persons = group.add_table("person");
    TableRef dogs = group.add_table("dog", Table::Type::Embedded);

    constexpr bool is_nullable = true;
    persons->add_column(type_String, "name");
    persons->add_column(type_Bool, "isMarried");
    persons->add_column(type_Int, "age", is_nullable);
    persons->add_column_list(type_Timestamp, "dates");
    persons->add_column_list(*dogs, "pet");
    persons->add_column_dictionary(type_Mixed, "dictionary_pet");
    persons->add_column(type_Int, "nested_list", false, {CollectionType::List, CollectionType::List});
    persons->add_column(type_Int, "nested_dict", false, {CollectionType::List, CollectionType::Dictionary});
    dogs->add_column(type_String, "name");

    std::stringstream ss;
    group.schema_to_json(ss);
    const std::string json = ss.str();
    std::string expected =
        "[\n"
        "{\"name\":\"person\",\"tableType\":\"TopLevel\",\"properties\":["
        "{\"name\":\"name\",\"type\":\"string\"},"
        "{\"name\":\"isMarried\",\"type\":\"bool\"},"
        "{\"name\":\"age\",\"type\":\"int\",\"isOptional\":true},"
        "{\"name\":\"dates\",\"type\":\"timestamp\",\"isArray\":true,\"isNested\":false},"
        "{\"name\":\"pet\",\"type\":\"object\",\"objectType\":\"dog\",\"isArray\":true,\"isNested\":false},"
        "{\"name\":\"dictionary_pet\",\"type\":\"mixed\",\"isMap\":true,\"isNested\":false,\"keyType\":\"string\","
        "\"isOptional\":true},"
        "{\"name\":\"nested_list\",\"type\":\"int\",\"isArray\":true,\"isNested\":true},"
        "{\"name\":\"nested_dict\",\"type\":\"int\",\"isMap\":true,\"isNested\":true,\"keyType\":\"string\"}"
        "]},\n"
        "{\"name\":\"dog\",\"tableType\":\"Embedded\",\"properties\":[{\"name\":\"name\",\"type\":\"string\"}]}\n"
        "]\n";
    CHECK_EQUAL(expected, json);
}

} // anonymous namespace

#endif // TEST_TABLE
