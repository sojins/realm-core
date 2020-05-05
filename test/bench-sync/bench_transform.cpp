#include "../util/benchmark_results.hpp"
#include "../util/dump_changesets.hpp"
#include "../util/timer.hpp"
#include "../util/test_path.hpp"
#include "../test_all.hpp"

#if REALM_ENABLE_ENCRYPTION
#include "../util/crypt_key.hpp"
#endif // REALM_ENABLE_ENCRYPTION

#include <realm/sync/history.hpp>
#include <realm/sync/object.hpp>

#include "../peer.hpp"

using namespace realm;
using namespace realm::sync;
using namespace realm::test_util;
using namespace realm::test_util::unit_test;

static constexpr auto s_bench_test_dump_dir = "BENCHTEST_DUMP_TRANSFORM";

namespace bench {

// Two peers have 1000 transactions each with a handful of instructions in
// each (25% transactions contain MoveLastOver). One peer receives and merges
// all transactions from the other (but does not apply them to their
// database).
template <size_t num_transactions>
void transform_transactions(TestContext& test_context, BenchmarkResults& results)
{
    std::string ident = test_context.test_details.test_name;
    std::string ident_preface = test_context.test_details.test_name + "_Setup";
    const size_t num_iterations = 3;

    for (size_t i = 0; i < num_iterations; ++i) {
        // We dump the changesets generated by the performance tests when a directory is specified.
        // This generates a performance testing corpus for the Golang implementation.
        auto changeset_dump_dir_gen = get_changeset_dump_dir_generator(test_context, s_bench_test_dump_dir);

        auto server = Peer::create_server(test_context, changeset_dump_dir_gen.get());
        auto origin = Peer::create_client(test_context, 2, changeset_dump_dir_gen.get());
        auto client = Peer::create_client(test_context, 3, changeset_dump_dir_gen.get());

        // Produce some mostly realistic transactions on both sides.
        auto make_transactions = [](Peer& peer) {
            ColKey col_ndx;
            {
                peer.start_transaction();
                TableRef t = sync::create_table(*peer.group, "class_t");
                col_ndx = t->add_column(type_Int, "i");
                peer.commit();
            }

            for (size_t j = 0; j < num_transactions-1; ++j) {
                peer.start_transaction();
                TableRef t = peer.table("class_t");
                sync::create_object(*peer.group, *t).set(col_ndx, 123);

                // Let 25% of commits contain a MoveLastOver
                if (j % 4 == 0) {
                    t->remove_object(t->begin());
                }
                peer.commit();
            }
        };

        //Timer t_preface{Timer::type_RealTime};
        make_transactions(*origin);
        make_transactions(*client);
        //results.submit(ident_preface.c_str(), t_preface.get_elapsed_time());

        // Upload everything to the server (fast, no conflicts)
        size_t outstanding = server->count_outstanding_changesets_from(*origin);
        for (size_t j = 0; j < outstanding; ++j) {
            server->integrate_next_changeset_from(*origin);
        }

        outstanding = client->count_outstanding_changesets_from(*server);
        REALM_ASSERT(outstanding != 0);
        Timer t{Timer::type_RealTime};
        // FIXME: client to server is artificially slow, because we do not yet
        // have a batched UPLOAD ability.
        //
        // for (size_t j = 0; j < outstanding; ++j) {
        //     server->integrate_next_changeset_from(*client);
        // }
        client->integrate_next_changesets_from(*server, outstanding);
        results.submit(ident.c_str(), t.get_elapsed_time());

    }

    //results.finish(ident_preface, ident_preface);
    results.finish(ident, ident);
}

// Two peers have 1 transaction each with 1000 instructions (8.3% of
// instructions are MoveLastOver). One peer receives and merges the large
// transaction from the other (but does not apply it to their database).
template <size_t num_iterations>
void transform_instructions(TestContext& test_context, BenchmarkResults& results)
{
    std::string ident = test_context.test_details.test_name;
    std::string ident_preface = test_context.test_details.test_name + "_Setup";

    for (size_t i = 0; i < 3; ++i) {
        // We dump the changesets generated by the performance tests when a directory is specified.
        // This generates a performance testing corpus for the Golang implementation.
        auto changeset_dump_dir_gen = get_changeset_dump_dir_generator(test_context, s_bench_test_dump_dir);

        auto server = Peer::create_server(test_context, changeset_dump_dir_gen.get());
        auto client = Peer::create_client(test_context, 2, changeset_dump_dir_gen.get());

        // Produce some mostly realistic transactions on both sides.
        auto make_instructions = [](Peer& peer) {
            peer.start_transaction();
            TableRef t = sync::create_table(*peer.group, "class_t");
            ColKey col_ndx = t->add_column(type_Int, "i");

            for (size_t j = 0; j < num_iterations; ++j) {
                Obj obj = sync::create_object(*peer.group, *t);
                obj.set(col_ndx, 123);

                // Let 25% of commits contain a MoveLastOver
                if (j % 4 == 0) {
                    t->begin()->remove();
                }
            }

            return peer.commit();
        };

        //Timer t_preface{Timer::type_RealTime};
        make_instructions(*server);
        make_instructions(*client);
        //results.submit(ident_preface.c_str(), t_preface.get_elapsed_time());

        size_t outstanding = server->count_outstanding_changesets_from(*client);
        REALM_ASSERT(outstanding != 0);
        Timer t{Timer::type_RealTime};
        for (size_t j = 0; j < outstanding; ++j) {
            server->integrate_next_changeset_from(*client);
        }
        results.submit(ident.c_str(), t.get_elapsed_time());

    }

    //results.finish(ident_preface, ident_preface);
    results.finish(ident, ident);
}

template <size_t num_iterations>
void connected_objects(TestContext& test_context, BenchmarkResults& results)
{
    std::string ident = test_context.test_details.test_name;
    std::string ident_preface = test_context.test_details.test_name + "_Setup";

    for (size_t i = 0; i < 3; ++i) {
        // We dump the changesets generated by the performance tests when a directory is specified.
        // This generates a performance testing corpus for the Golang implementation.
        auto changeset_dump_dir_gen = get_changeset_dump_dir_generator(test_context, s_bench_test_dump_dir);

        auto server = Peer::create_server(test_context, changeset_dump_dir_gen.get());
        auto client = Peer::create_client(test_context, 2, changeset_dump_dir_gen.get());

        auto make_instructions = [](Peer& peer) {
            peer.start_transaction();
            TableRef t = sync::create_table_with_primary_key(*peer.group, "class_t", type_String, "pk");
            auto col_key = t->add_column_link(type_Link, "l", *t);

            // Everything links to this object!
            auto first_key = sync::create_object_with_primary_key(*peer.group, *t, "Hello").get_key();

            for (size_t j = 0; j < num_iterations; ++j) {
                std::stringstream ss;
                ss << j;
                std::string pk = ss.str();
                sync::create_object_with_primary_key(*peer.group, *t, pk).set(col_key, first_key);
            }

            return peer.commit();
        };

        //Timer t_preface{Time::type_RealTime};
        make_instructions(*server);
        make_instructions(*client);
        //results.submit(ident_preface.c_str(), t_preface.get_elapsed_time());

        size_t outstanding = server->count_outstanding_changesets_from(*client);
        REALM_ASSERT(outstanding != 0);
        Timer t{Timer::type_RealTime};
        for (size_t j = 0; j < outstanding; ++j) {
            server->integrate_next_changeset_from(*client);
        }
        results.submit(ident.c_str(), t.get_elapsed_time());
    }

    results.finish(ident, ident);
}

} // namespace bench

const int max_lead_text_width = 40;

TEST(BenchMerge1000x1000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_1000x1000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<1000>(test_context, results);
}

TEST(BenchMerge2000x2000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_2000x2000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<2000>(test_context, results);
}

TEST(BenchMerge3000x3000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_3000x3000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<3000>(test_context, results);
}

TEST(BenchMerge4000x4000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_4000x4000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<4000>(test_context, results);
}

TEST(BenchMerge5000x5000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_5000x5000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<5000>(test_context, results);
}

TEST(BenchMerge10000x10000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_10000x10000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<10000>(test_context, results);
}

TEST(BenchMerge11000x11000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_11000x11000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<11000>(test_context, results);
}

TEST(BenchMerge12000x12000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_12000x12000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<12000>(test_context, results);
}

TEST(BenchMerge13000x13000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_13000x13000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<13000>(test_context, results);
}

TEST(BenchMerge14000x14000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_14000x14000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<14000>(test_context, results);
}

TEST(BenchMerge15000x15000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_15000x15000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<15000>(test_context, results);
}

TEST(BenchMerge16000x16000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_16000x16000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<16000>(test_context, results);
}

TEST(BenchMerge17000x17000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_17000x17000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<17000>(test_context, results);
}

TEST(BenchMerge18000x18000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_18000x18000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<18000>(test_context, results);
}

TEST(BenchMerge19000x19000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_19000x19000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<19000>(test_context, results);
}

TEST(BenchMerge20000x20000Instructions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "instructions_20000x20000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_instructions<20000>(test_context, results);
}


TEST(BenchMerge100x100Transactions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "transactions_100x100";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_transactions<100>(test_context, results);
}

TEST(BenchMerge500x500Transactions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "transactions_500x500";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_transactions<500>(test_context, results);
}

TEST(BenchMerge1000x1000Transactions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "transactions_1000x1000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_transactions<1000>(test_context, results);
}

TEST(BenchMerge2000x2000Transactions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "transactions_2000x2000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_transactions<2000>(test_context, results);
}

TEST(BenchMerge3000x3000Transactions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "transactions_3000x3000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_transactions<3000>(test_context, results);
}

TEST(BenchMerge4000x4000Transactions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "transactions_4000x4000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_transactions<4000>(test_context, results);
}

TEST(BenchMerge5000x5000Transactions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "transactions_5000x5000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_transactions<5000>(test_context, results);
}

TEST(BenchMerge6000x6000Transactions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "transactions_6000x6000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_transactions<6000>(test_context, results);
}

TEST(BenchMerge7000x7000Transactions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "transactions_7000x7000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_transactions<7000>(test_context, results);
}

TEST(BenchMerge8000x8000Transactions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "transactions_8000x8000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_transactions<8000>(test_context, results);
}

TEST(BenchMerge9000x9000Transactions)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "transactions_9000x9000";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::transform_transactions<9000>(test_context, results);
}

TEST(BenchMergeManyConnectedObjects)
{
    std::string results_file_stem = test_util::get_test_path_prefix() + "connected_objects";
    BenchmarkResults results(max_lead_text_width, results_file_stem.c_str());

    bench::connected_objects<8000>(test_context, results);
}

#if !REALM_IOS
int main(int argc, char** argv)
{
    return test_all(argc, argv, nullptr);
}
#endif // REALM_IOS

