#include <gtest/gtest.h>
#include <string>
#include <chrono>

#include "tests/xelection/xmocked_vnode_service.h"

#include "xbasic/xasio_io_context_wrapper.h"
#include "xbasic/xtimer_driver.h"
#include "xblockstore/xblockstore_face.h"
#include "xchain_timer/xchain_timer.h"
#include "xdata/xblock_statistics_data.h"
#include "xloader/xconfig_onchain_loader.h"
#include "xstake/xstake_algorithm.h"
#include "xvm/xsystem_contracts/xslash/xtable_statistic_info_collection_contract_new.h"

using namespace top;
using namespace top::tests::election;
using namespace top::data;

// const uint16_t AUDITOR_ACCOUNT_ADDR_NUM = 64;
// const uint16_t VALIDATOR_ACCOUNT_ADDR_NUM = 128;
const uint16_t AUDITOR_ACCOUNT_ADDR_NUM = 256;
const uint16_t VALIDATOR_ACCOUNT_ADDR_NUM = 512;

class test_runtime_statistic_data: public testing::Test {
public:
    test_runtime_statistic_data(): node_serv{common::xaccount_address_t{"mocked_nodesvr"}, "null"}{};

    void SetUp(){
        create_account_addrs(AUDITOR_ACCOUNT_ADDR_NUM, VALIDATOR_ACCOUNT_ADDR_NUM);

        uint64_t elect_blk_height = 1;
        nodeservice_add_group(elect_blk_height, create_group_xip2(elect_blk_height, uint8_t(1), AUDITOR_ACCOUNT_ADDR_NUM), auditor_account_addrs);
        nodeservice_add_group(elect_blk_height, create_group_xip2(elect_blk_height, uint8_t(64), VALIDATOR_ACCOUNT_ADDR_NUM), validator_account_addrs);
    }
    void TearDown(){}



    void create_account_addrs(uint32_t auditor_account_num, uint32_t validator_account_num);
    common::xip2_t create_group_xip2(uint64_t elect_blk_height, uint8_t group_id, uint16_t group_size);
    void nodeservice_add_group(uint64_t elect_blk_height, common::xip2_t const& group_xip, std::vector<common::xaccount_address_t> const& nodes);
    void set_according_block_statistic_data(uint64_t elect_blk_height, std::vector<common::xip2_t> const& group_xip);


public:
    std::vector<common::xaccount_address_t> auditor_account_addrs;
    std::vector<common::xaccount_address_t> validator_account_addrs;
    xstatistics_data_t data;
    xmocked_vnodesvr_t node_serv;
};


void test_runtime_statistic_data::create_account_addrs(uint32_t auditor_account_num, uint32_t validator_account_num) {
    auditor_account_addrs.resize(auditor_account_num);
    validator_account_addrs.resize(validator_account_num);

    for (uint32_t i = 0; i < auditor_account_num; ++i) {
        auditor_account_addrs[i] = common::xaccount_address_t{std::string{"auditor_account__"} + std::to_string(i)};
        // top::utl::xecprikey_t prikey;
        // auditor_account_addrs[i] = common::xaccount_address_t{prikey.to_account_address('0', 0)};

    }

    for (uint32_t i = 0; i < validator_account_num; ++i) {
        validator_account_addrs[i] = common::xaccount_address_t{std::string{"validator_account__"} + std::to_string(i)};
        // top::utl::xecprikey_t prikey;
        // validator_account_addrs[i] = common::xaccount_address_t{prikey.to_account_address('0', 0)};

    }


}

common::xip2_t test_runtime_statistic_data::create_group_xip2(uint64_t elect_blk_height, uint8_t group_id, uint16_t group_size) {
     return common::xip2_t{
        common::xnetwork_id_t{0},
        common::xconsensus_zone_id,
        common::xdefault_cluster_id,
        common::xgroup_id_t{group_id},
        group_size,
        elect_blk_height
     };
 }

void test_runtime_statistic_data::nodeservice_add_group(uint64_t elect_blk_height, common::xip2_t const& group_xip, std::vector<common::xaccount_address_t> const& nodes) {
    node_serv.add_group(
        group_xip.network_id(),
        group_xip.zone_id(),
        group_xip.cluster_id(),
        group_xip.group_id(),
        (uint16_t)group_xip.size(),
        elect_blk_height
    );


    xmocked_vnode_group_t* node_group = dynamic_cast<xmocked_vnode_group_t*>(node_serv.get_group(group_xip).get());
    node_group->reset_nodes();
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        node_group->add_node(nodes[i]);
    }


    assert(node_group->get_nodes().size() == nodes.size());
    // auto group_nodes = node_serv.get_group(group_xip)->get_nodes();
    // std::cout << "node size: " << group_nodes.size() << "\n";
    // for (std::size_t i = 0; i < group_nodes.size(); ++i) {
    //     std::cout << group_nodes[i]->get_account() << "\n";
    // }

}

void test_runtime_statistic_data::set_according_block_statistic_data(uint64_t elect_blk_height, std::vector<common::xip2_t> const& group_xips) {
    xelection_related_statistics_data_t elect_data;
    for (std::size_t i = 0; i < group_xips.size(); ++i) {
        auto vnode_group = node_serv.get_group(group_xips[i]);
        assert(vnode_group != nullptr);

        xgroup_related_statistics_data_t group_data;
        // set vote data
        for (std::size_t i = 0; i < vnode_group->get_size(); ++i) {
            xaccount_related_statistics_data_t account_data;
            account_data.vote_data.block_count = i + 1;
            account_data.vote_data.vote_count = i;

            group_data.account_statistics_data.push_back(account_data);
        }

        common::xgroup_address_t  group_addr{group_xips[i].xip()};
        elect_data.group_statistics_data[group_addr] = group_data;
        // std::cout << "[test_runtime_statistic_data::set_according_block_statistic_data] " << common::xgroup_address_t{group_xips[i].xip()}.to_string() << "\n";
        // std::cout << "[test_runtime_statistic_data::set_according_block_statistic_data] " << std::hex <<(int)common::xgroup_address_t{group_xips[i].xip()}.type() << "\n";
    }

    data.detail[elect_blk_height] = elect_data;

}

TEST_F(test_runtime_statistic_data, fulltableblock_statistic_accounts) {
    uint64_t elect_blk_height = 1;
    auto group_1_xip2 = create_group_xip2(elect_blk_height, 1, auditor_account_addrs.size());
    auto group_64_xip2 = create_group_xip2(elect_blk_height, 64, validator_account_addrs.size());
    set_according_block_statistic_data(1, std::vector<common::xip2_t>{group_1_xip2, group_64_xip2});

    auto auditor_group_nodes = node_serv.get_group(group_1_xip2)->get_nodes();
    auto validator_group_nodes = node_serv.get_group(group_64_xip2)->get_nodes();


    auto statistic_accounts = fulltableblock_statistic_accounts(data, node_serv);

}